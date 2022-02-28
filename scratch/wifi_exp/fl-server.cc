/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Emily Ekaireb
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Emily Ekaireb <eekaireb@ucsd.edu>
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "fl-server.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "fl-sim-interface.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("Server");

    NS_OBJECT_ENSURE_REGISTERED (Server);

    TypeId
    Server::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Server")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<Server>()

                .AddAttribute("DataRate", "The data rate in on state.",
                              DataRateValue(DataRate("1b/s")),
                              MakeDataRateAccessor(&Server::m_dataRate),
                              MakeDataRateChecker())

                .AddAttribute("Local",
                              "The Address on which to Bind the rx socket.",
                              AddressValue(),
                              MakeAddressAccessor(&Server::m_local),
                              MakeAddressChecker())
                .AddAttribute("Protocol",
                              "The type id of the protocol to use for the rx socket.",
                              TypeIdValue(UdpSocketFactory::GetTypeId()),
                              MakeTypeIdAccessor(&Server::m_tid),
                              MakeTypeIdChecker())

                .AddAttribute("MaxPacketSize",
                              "MaxPacketSize to send to client",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Server::m_packetSize),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("Async",
                              "Run as async client",
                              TypeId::ATTR_SGC,
                              BooleanValue(false),
                              MakeBooleanAccessor(&Server::m_bAsync),
                              MakeBooleanChecker())
                .AddAttribute("BytesModel",
                              "Number of bytes in model",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Server::m_bytesModel),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("TimeOffset",
                              "Time offset to add to simulation time reported",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&Server::m_timeOffset),
                              MakeTimeChecker());


        return tid;
    }

    Server::Server() : m_packetSize(0), m_sendEvent(), m_bytesModel(0), m_bAsync(false), m_fLSimProvider(nullptr) {
        m_socket = 0;
    }

    Server::~Server() {
        NS_LOG_FUNCTION(this);
    }

    std::map <Ptr<Socket>, std::shared_ptr<Server::ClientSessionData>>
    Server::GetAcceptedSockets(void) const {
        NS_LOG_FUNCTION(this);
        return m_socketList;
    }

    void Server::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        m_socketList.clear();

        // chain up
        Application::DoDispose();
    }

// Application Methods
    void Server::StartApplication()    // Called at time specified by Start
    {
        NS_LOG_FUNCTION(this);
        // Create the socket if not already
        if (!m_socket) {
            m_socket = Socket::CreateSocket(GetNode(), m_tid);
            if (m_socket->Bind(m_local) == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (m_socket->Listen() == -1) {
                NS_FATAL_ERROR("Failed to listen socket");
            }
        }

        m_socket->SetRecvCallback(MakeCallback(&Server::ReceivedDataCallback, this));
        m_socket->SetAcceptCallback(
                MakeCallback(&Server::ConnectionRequestCallback, this),
                MakeCallback(&Server::NewConnectionCreatedCallback, this));
        m_socket->SetCloseCallbacks(
                MakeCallback(&Server::HandlePeerClose, this),
                MakeCallback(&Server::HandlePeerError, this));

    }

    void Server::StopApplication()     // Called at time specified by Stop
    {
        NS_LOG_FUNCTION(this);
        NS_LOG_UNCOND("Stopping Application");

        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }

        //Close all connections
        for (auto const &itr: m_socketList) {
            itr.first->Close();
            itr.first->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }

        if (m_socket) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }

    }

    void Server::ReceivedDataCallback(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        Address localAddress;
        while ((packet = socket->RecvFrom(from))) {
            if (packet->GetSize() == 0) { //EOF
                break;
            }

            auto itr = m_socketList.find(socket);

            if (itr == m_socketList.end()) {
                return;
            }

            if (itr->second->m_bytesReceived % m_bytesModel == 0) {
                itr->second->m_timeBeginReceivingModelFromClient = Simulator::Now();
            }

            itr->second->m_bytesReceived += packet->GetSize();
            itr->second->m_bytesModelToReceive -= packet->GetSize();


            if (itr->second->m_bytesModelToReceive == 0) {
                itr->second->m_timeEndReceivingModelFromClient = Simulator::Now();

                auto endUplink =
                    itr->second->m_timeEndReceivingModelFromClient.GetSeconds() + m_timeOffset.GetSeconds();
                auto beginUplink =
                    itr->second->m_timeBeginReceivingModelFromClient.GetSeconds() + m_timeOffset.GetSeconds();
                auto beginDownlink =
                    itr->second->m_timeBeginSendingModelFromClient.GetSeconds() + m_timeOffset.GetSeconds();
                auto endDownlink=
                    itr->second->m_timeEndSendingModelFromClient.GetSeconds() +  m_timeOffset.GetSeconds();

                auto energy = FLEnergy();
                energy.SetDeviceType("400");
                energy.SetLearningModel("CIFAR-10");
                energy.SetEpochs(5.0);
                double compEnergy = energy.CalcComputationalEnergy(beginUplink-endDownlink);
                double tranEnergy = energy.CalcTransmissionEnergy(endUplink-beginUplink);
                NS_LOG_UNCOND(energy.GetA());
                fprintf(m_fp, "%i,%u,%f,%f,%f,%f,%f,%f\n",
                         m_round,
                         m_clientSessionManager->ResolveToIdFromServer(socket),
                         beginUplink, endUplink,
                         beginDownlink, endDownlink,
                         compEnergy, tranEnergy
                );
                fflush(m_fp);

                if (m_bAsync) {

                    if (m_fLSimProvider) {
                        FLSimProvider::AsyncMessage message;

                        message.id = m_clientSessionManager->ResolveToIdFromServer(socket);
                        message.endTime = endUplink;
                        message.startTime = beginDownlink;
                        message.throughput = itr->second->m_bytesReceived * 8.0 / 1000.0 /
                                             ((endUplink - beginUplink));

                        m_fLSimProvider->send(&message);

                        ns3::Address addr;
                        socket->GetPeerName(addr);
                        auto temp = ns3::InetSocketAddress::ConvertFrom(addr).GetIpv4();

                        NS_LOG_UNCOND(
                                "[SERVER]  " << temp << " -> 10.1.1.1" << std::endl <<
                                             "  Round " << m_clientSessionManager->GetRound(socket) << std::endl <<
                                             "  Begin downlink=" << beginDownlink << std::endl <<
                                             "  Begin uplink=" << beginUplink << std::endl <<
                                             "  End uplink=" << endUplink << std::endl <<
                                             "  Round time=" << (endUplink - beginDownlink) << std::endl <<
                                             "  UplinkDifference=" << (endUplink - beginUplink) << std::endl);
                    }

                    m_clientSessionManager->IncrementCycleCountFromServer(socket);

                    if (m_clientSessionManager->HasAllClientsFinishedFirstCycle() ||
                        (m_clientSessionManager->GetRound(socket) == 3)) {
                        m_clientSessionManager->Close();
                        if (m_fLSimProvider) {
                            m_fLSimProvider->end();
                        }

                        m_timeOffset = Simulator::Now() + Time(m_timeOffset.GetSeconds());
                        NS_LOG_UNCOND("STOPPING_SIMULATION" << std::endl << std::endl);
                        Simulator::Stop();
                    } else {
                        StartSendingModel(socket);
                    }
                }
            }

            socket->GetSockName(localAddress);

        }
    }

    void Server::HandlePeerClose(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
    }

    void Server::HandlePeerError(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
    }

    bool Server::ConnectionRequestCallback(Ptr <Socket> socket, const Address &address) {
        NS_LOG_FUNCTION(this << socket << address);
        return true;
    }


    void Server::NewConnectionCreatedCallback(Ptr <Socket> socket, const Address &from) {
        NS_LOG_FUNCTION(this << socket << from);
        auto clientSession = std::make_shared<ClientSessionData>();

        auto nsess = m_socketList.insert(std::make_pair(socket, clientSession));
        nsess.first->second->m_address = from;
        socket->SetRecvCallback(MakeCallback(&Server::ReceivedDataCallback, this));
        StartSendingModel(socket);

        NS_LOG_UNCOND("Accept:" << m_clientSessionManager->ResolveToIdFromServer(socket));

        Ptr <Packet> packet;
        while ((packet = socket->Recv())) {
            if (packet->GetSize() == 0) {
                break; // EOF
            }
        }
    }

    void Server::ServerHandleSend(Ptr <Socket> socket, uint32_t available) {
        m_socket->SetSendCallback(MakeNullCallback < void, Ptr < Socket > , uint32_t > ());

        if (m_sendEvent.IsExpired()) {
            SendModel(socket);
        }
    }

    void Server::StartSendingModel(Ptr <Socket> socket) {
        auto itr = m_socketList.find(socket);
        itr->second->m_bytesModelToReceive = m_bytesModel;
        itr->second->m_bytesModelToSend = m_bytesModel;
        if (m_clientSessionManager->GetRound(socket) == 0)
            itr->second->m_timeBeginSendingModelFromClient;
        else
            itr->second->m_timeBeginSendingModelFromClient = Simulator::Now();
        SendModel(socket);
    }

    void Server::SendModel(Ptr <Socket> socket) {

        //Check if send buffer has available space
        //If not, wait for data ready callback
        auto available = socket->GetTxAvailable();
        if (available == 0) {
            socket->SetSendCallback(MakeCallback(&Server::ServerHandleSend, this));
            return;
        }

        auto itr = m_socketList.find(socket);

        if (itr == m_socketList.end() || itr->second->m_bytesModelToSend == 0) {
            return;
        }

        auto bytes = std::min(std::min(
                itr->second->m_bytesModelToSend, available), m_packetSize);

        auto bytesSent = socket->Send(Create<Packet>(bytes));

        if (bytesSent == -1) {
            return;
        }

        itr->second->m_bytesSent += bytesSent;
        itr->second->m_bytesModelToSend -= bytesSent;

        if (itr->second->m_bytesModelToSend) {
            Time nextTime(Seconds((bytes * 8) /
                                  static_cast<double>(m_dataRate.GetBitRate())));

            m_sendEvent = Simulator::Schedule(nextTime,
                                              &Server::SendModel, this, socket);
        }
        else
          {
            itr->second->m_timeEndSendingModelFromClient=Simulator::Now();
          }
    }

} // Namespace ns3
