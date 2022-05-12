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
#include "fl-gateway.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "fl-sim-interface.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("Gateway");

    NS_OBJECT_ENSURE_REGISTERED (Gateway);

    TypeId
    Gateway::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Gateway")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<Gateway>()

                .AddAttribute("BytesSentToServer", "The bytes send to server.",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Gateway::m_serverBytesSent),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BytesReceivedFromServer", "The bytes received to server.",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Gateway::m_serverBytesModelReceived),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BeginDownlink",
                              "Time begin receiving model from server",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&Gateway::m_serverTimeBeginReceivingModelFromServer),
                              MakeTimeChecker())

                .AddAttribute("EndDownlink",
                              "Time finish receiving model from server",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&Gateway::m_serverTimeEndReceivingModelFromServer),
                              MakeTimeChecker())

                .AddAttribute("DataRate", "The data rate in on state.",
                              DataRateValue(DataRate("1b/s")),
                              MakeDataRateAccessor(&Gateway::m_dataRate),
                              MakeDataRateChecker())


                .AddAttribute("Local",
                              "The Address on which to Bind the rx socket.",
                              AddressValue(),
                              MakeAddressAccessor(&Gateway::m_local),
                              MakeAddressChecker())
                .AddAttribute("Protocol",
                              "The type id of the protocol to use for the rx socket.",
                              TypeIdValue(UdpSocketFactory::GetTypeId()),
                              MakeTypeIdAccessor(&Gateway::m_tid),
                              MakeTypeIdChecker())

                .AddAttribute("MaxPacketSize",
                              "MaxPacketSize to send to client",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Gateway::m_packetSize),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("Async",
                              "Run as async client",
                              TypeId::ATTR_SGC,
                              BooleanValue(false),
                              MakeBooleanAccessor(&Gateway::m_bAsync),
                              MakeBooleanChecker())
                .AddAttribute("BytesModel",
                              "Number of bytes in model",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&Gateway::m_bytesModel),
                              MakeUintegerChecker<uint32_t>())
                .AddAttribute("TimeOffset",
                              "Time offset to add to simulation time reported",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&Gateway::m_timeOffset),
                              MakeTimeChecker());


        return tid;
    }

    Gateway::Gateway() : m_packetSize(0), m_sendEvent(), m_bytesModel(0), m_bAsync(false), m_fLSimProvider(nullptr) {
        m_socket = 0;
    }

    Gateway::~Gateway() {
        NS_LOG_FUNCTION(this);
    }

    std::map <Ptr<Socket>, std::shared_ptr<Gateway::ClientSessionData>>
    Gateway::GetAcceptedSockets(void) const {
        NS_LOG_FUNCTION(this);
        return m_socketList;
    }

    void Gateway::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        m_socketList.clear();

        // chain up
        Application::DoDispose();
    }

// Application Methods
    void Gateway::StartApplication()    // Called at time specified by Start
    {

        //_______________________Initialize server functionality_________________________
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

        m_socket->SetRecvCallback(MakeCallback(&Gateway::ReceivedDataCallback, this));
        m_socket->SetAcceptCallback(
                MakeCallback(&Gateway::ConnectionRequestCallback, this),
                MakeCallback(&Gateway::NewConnectionCreatedCallback, this));
        m_socket->SetCloseCallbacks(
                MakeCallback(&Gateway::HandlePeerClose, this),
                MakeCallback(&Gateway::HandlePeerError, this));

        //___________________Initialize client functionality___________________________
        NS_LOG_UNCOND("XXXXXXXX 1");
        NS_LOG_UNCOND(this);
        NS_LOG_UNCOND(m_serverSocket);
        m_serverSocket->SetCloseCallbacks(
                MakeCallback(&Gateway::NormalClose, this),
                MakeCallback(&Gateway::ErrorClose, this));
        m_serverSocket->SetConnectCallback(
                MakeCallback(&Gateway::ConnectionSucceeded, this),
                MakeCallback(&Gateway::ConnectionFailed, this));



        m_serverSocket->Bind();
        m_serverSocket->Connect(m_serverPeer);

        m_serverTimeBeginReceivingModelFromServer = Simulator::Now();

        m_serverSocket->SetRecvCallback(MakeCallback(&Gateway::HandleReadFromServer, this));
    }



    void Gateway::StopApplication()     // Called at time specified by Stop
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

    void Gateway::ReceivedDataCallback(Ptr <Socket> socket) {
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

                m_clientSessionManager->IncrementCycleCountFromServer(socket);

                if (m_clientSessionManager->HasAllClientsFinishedFirstCycle())
                {
                    StartWritingToServer();
                }

            }

            socket->GetSockName(localAddress);

        }
    }




    void Gateway::HandlePeerClose(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
    }

    void Gateway::HandlePeerError(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
    }

    bool Gateway::ConnectionRequestCallback(Ptr <Socket> socket, const Address &address) {
        NS_LOG_FUNCTION(this << socket << address);
        return true;
    }


    void Gateway::NewConnectionCreatedCallback(Ptr <Socket> socket, const Address &from) {
        NS_LOG_FUNCTION(this << socket << from);
        auto clientSession = std::make_shared<ClientSessionData>();

        auto nsess = m_socketList.insert(std::make_pair(socket, clientSession));
        nsess.first->second->m_address = from;
        socket->SetRecvCallback(MakeCallback(&Gateway::ReceivedDataCallback, this));
       // StartSendingModel(socket);

        NS_LOG_UNCOND("Accept:" << m_clientSessionManager->ResolveToIdFromServer(socket));

      //  Ptr <Packet> packet;
      //  while ((packet = socket->Recv())) {
      //      if (packet->GetSize() == 0) {
      //          break; // EOF
      //      }
      //  }
    }

    void Gateway::GatewayHandleSend(Ptr <Socket> socket, uint32_t available) {
        m_socket->SetSendCallback(MakeNullCallback < void, Ptr < Socket > , uint32_t > ());

        if (m_sendEvent.IsExpired()) {
            SendModel(socket);
        }
    }

    void Gateway::StartSendingModel(Ptr <Socket> socket) {
        auto itr = m_socketList.find(socket);
        itr->second->m_bytesModelToReceive = m_bytesModel;
        itr->second->m_bytesModelToSend = m_bytesModel;
        if (m_clientSessionManager->GetRound(socket) == 0)
            itr->second->m_timeBeginSendingModelFromClient;
        else
            itr->second->m_timeBeginSendingModelFromClient = Simulator::Now();
        SendModel(socket);
    }

    void Gateway::SendModel(Ptr <Socket> socket) {

        //Check if send buffer has available space
        //If not, wait for data ready callback
        auto available = socket->GetTxAvailable();
        if (available == 0) {
            socket->SetSendCallback(MakeCallback(&Gateway::GatewayHandleSend, this));
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
                                              &Gateway::SendModel, this, socket);
        }
        else
          {
            itr->second->m_timeEndSendingModelFromClient=Simulator::Now();
          }
    }











void
Gateway::Setup(Ptr <Socket> socket, Address address, uint32_t packetSize, uint32_t nBytesModel,
                         DataRate dataRate) {
    m_serverSocket = socket;
    m_serverPeer = address;
    m_serverPacketSize = packetSize;
    m_serverBytesModel = nBytesModel;
    m_serverDataRate = dataRate;

    NS_LOG_UNCOND("XXXXXXXX 0");
    NS_LOG_UNCOND(this);
    NS_LOG_UNCOND(m_serverSocket);
}

    void Gateway::NormalClose(Ptr <Socket> socket) {
        NS_LOG_UNCOND(" Close ...");
    }

    void Gateway::ErrorClose(Ptr <Socket> socket) {
        NS_LOG_UNCOND("Error Close ...");
    }

    void Gateway::ConnectionSucceeded(Ptr <Socket> socket) {

        NS_LOG_UNCOND("Gatway Connected to Server");
        socket->SetRecvCallback(MakeCallback(&Gateway::HandleReadFromServer, this));

        m_serverBytesModelToReceive = m_serverBytesModel;
        m_serverBytesModelToSend = 0;

        NS_LOG_UNCOND("Gateway " << (socket->GetNode()->GetId() + 1) << " " << m_serverBytesModelToReceive);
    }

    void Gateway::HandleReadFromServer(Ptr <Socket> socket) {

        Ptr <Packet> packet;

        while ((packet = socket->Recv())) {

            if (packet->GetSize() == 0) { //EOF
                break;
            }

            if (m_serverBytesModelReceived % m_bytesModel == 0 && m_serverBytesModelReceived != 0) {
                m_serverTimeBeginReceivingModelFromServer = Simulator::Now();
            }

            m_serverBytesModelReceived += packet->GetSize(); //Increment total bytes received
            m_serverBytesModelToReceive -= packet->GetSize(); //Decrement bytes expected this round

            if (m_serverBytesModelToReceive == 0)  //All bytes received, start transmitting
            {
                m_serverTimeEndReceivingModelFromServer = Simulator::Now();

                NS_LOG_UNCOND("Gateway " << (socket->GetNode()->GetId() + 1) << " " << "recv full model");


                for(auto itr=m_socketList.begin();itr!=m_socketList.end();itr++)
                {
                    NS_LOG_UNCOND("Sending to ?");
                    StartSendingModel(itr->first);
                }

                //Simulator::Schedule(Seconds(0), &Gateway::StartWritingToClients, this);

            }

        }
    }

    void Gateway::ConnectionFailed(Ptr <Socket> socket) {
        NS_LOG_UNCOND("Not Connected ..." << socket->GetNode()->GetId());
    }

    void Gateway::StartWritingToServer()
    {
        NS_LOG_UNCOND("StartWrotingToServer");
        m_serverBytesModelToSend = m_serverBytesModel;

        SendToServer(m_serverSocket);
    }

    void Gateway::SendToServer(Ptr <Socket> socket) {
        //If nothing to  send, return
        if (m_serverBytesModelToSend == 0) {
            return;
        }

        auto available = m_serverSocket->GetTxAvailable();

        if (available) {

            auto bytes = std::min(std::min(m_serverBytesModelToSend, available), m_serverPacketSize);

            auto bytesSent = socket->Send(Create<Packet>(bytes));

            if (bytesSent == -1) {
                return;
            }


            m_serverBytesSent += bytesSent;
            m_serverBytesModelToSend -= bytesSent;


            if (m_serverBytesModelToSend) {

                Time nextTime(Seconds((bytesSent * 8) /
                                      static_cast<double>(m_dataRate.GetBitRate()))); // Time till next packet

                m_sendEvent = Simulator::Schedule(nextTime,
                                                  &Gateway::SendToServer, this, socket);
            } else {
                m_serverBytesModelToReceive = m_serverBytesModel;
            }
        } else {
            socket->SetSendCallback(MakeCallback(&Gateway::HandleReadyToSendToServer, this));
        }

    }

    void Gateway::HandleReadyToSendToServer(Ptr <Socket> sock, uint32_t available) {
        m_serverSocket->SetSendCallback(MakeNullCallback < void, Ptr < Socket > , uint32_t > ());
        SendToServer(sock);
    }


} // Namespace ns3
