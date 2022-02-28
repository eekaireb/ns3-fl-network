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

#include "fl-client-application.h"

#include "ns3/internet-module.h"
#include "fl-energy.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("ClientApplication");
    NS_OBJECT_ENSURE_REGISTERED (ClientApplication);




    ClientApplication::ClientApplication()
            : m_socket(0),
              m_peer(),
              m_packetSize(0),

              m_bytesModel(0),
              m_dataRate(0),


              m_bytesModelReceived(0),
              m_bytesModelToReceive(0),
              m_timeBeginReceivingModelFromServer(),
              m_timeEndReceivingModelFromServer(),



              m_bytesModelToSend(0),
              m_bytesSent(0),

              m_sendEvent(),
              m_model() {

    }

    TypeId
    ClientApplication::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::ClientApplication")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<ClientApplication>()
                .AddAttribute("DataRate", "The data rate in on state.",
                              DataRateValue(DataRate("500kb/s")),
                              MakeDataRateAccessor(&ClientApplication::m_dataRate),
                              MakeDataRateChecker())

                .AddAttribute("MaxPacketSize",
                              "MaxPacketSize to send to client",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&ClientApplication::m_packetSize),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BytesModel",
                              "Number of bytes in model",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&ClientApplication::m_bytesModel),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BytesSent",
                              "Number of bytes sent from client",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&ClientApplication::m_bytesSent),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BytesReceived",
                              "Number of bytes sent from server",
                              TypeId::ATTR_SGC,
                              UintegerValue(),
                              MakeUintegerAccessor(&ClientApplication::m_bytesModelReceived),
                              MakeUintegerChecker<uint32_t>())

                .AddAttribute("BeginDownlink",
                              "Time begin receiving model from server",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&ClientApplication::m_timeBeginReceivingModelFromServer),
                              MakeTimeChecker())

                .AddAttribute("EndDownlink",
                              "Time finish receiving model from server",
                              TypeId::ATTR_SGC,
                              TimeValue(),
                              MakeTimeAccessor(&ClientApplication::m_timeEndReceivingModelFromServer),
                              MakeTimeChecker());
        return tid;
    }

    void ClientApplication::NormalClose(Ptr <Socket> socket) {
        NS_LOG_UNCOND(" Close ...");
    }

    void ClientApplication::ErrorClose(Ptr <Socket> socket) {
        NS_LOG_UNCOND("Error Close ...");
    }

    void ClientApplication::HandleReadyToSend(Ptr <Socket> sock, uint32_t available) {
        m_socket->SetSendCallback(MakeNullCallback < void, Ptr < Socket > , uint32_t > ());
        Send(sock);
    }

    void ClientApplication::Send(Ptr <Socket> socket) {
        //If nothing to  send, return
        if (m_bytesModelToSend == 0) {
            return;
        }

        auto available = m_socket->GetTxAvailable();

        if (available) {

            auto bytes = std::min(std::min(m_bytesModelToSend, available), m_packetSize);

            auto bytesSent = socket->Send(Create<Packet>(bytes));

            if (bytesSent == -1) {
                return;
            }


            m_bytesSent += bytesSent;
            m_bytesModelToSend -= bytesSent;


            if (m_bytesModelToSend) {

                Time nextTime(Seconds((bytesSent * 8) /
                                      static_cast<double>(m_dataRate.GetBitRate()))); // Time till next packet

                m_sendEvent = Simulator::Schedule(nextTime,
                                                  &ClientApplication::Send, this, socket);
            } else {
                m_bytesModelToReceive = m_bytesModel;
            }
        } else {
            socket->SetSendCallback(MakeCallback(&ClientApplication::HandleReadyToSend, this));
        }

    }

    void ClientApplication::ConnectionSucceeded(Ptr <Socket> socket) {

        NS_LOG_UNCOND("Client Connected");
        socket->SetRecvCallback(MakeCallback(&ClientApplication::HandleRead, this));

        m_bytesModelToReceive = m_bytesModel;
        m_bytesModelToSend = 0;

        NS_LOG_UNCOND("Client " << (socket->GetNode()->GetId() + 1) << " " << m_bytesModelToReceive);
    }

    void ClientApplication::HandleRead(Ptr <Socket> socket) {

        Ptr <Packet> packet;

        while ((packet = socket->Recv())) {

            if (packet->GetSize() == 0) { //EOF
                break;
            }

            if (m_bytesModelReceived % m_bytesModel == 0 && m_bytesModelReceived != 0) {
                m_timeBeginReceivingModelFromServer = Simulator::Now();
            }

            m_bytesModelReceived += packet->GetSize(); //Increment total bytes received
            m_bytesModelToReceive -= packet->GetSize(); //Decrement bytes expected this round

            if (m_bytesModelToReceive == 0)  //All bytes received, start transmitting
            {
                m_timeEndReceivingModelFromServer = Simulator::Now();

                NS_LOG_UNCOND("Client " << (socket->GetNode()->GetId() + 1) << " " << "recv full model");



                //Todo[] Add a meaningful delay
                auto energy = FLEnergy();
                energy.SetDeviceType("400");
                energy.SetLearningModel("CIFAR-10");
                energy.SetEpochs(5.0);
                Simulator::Schedule(Seconds(energy.CalcComputationTime()), &ClientApplication::StartWriting, this);

            }

        }
    }

    void ClientApplication::StartWriting() {

        m_bytesModelToSend = m_bytesModel;

        Send(m_socket);
    }

    void ClientApplication::ConnectionFailed(Ptr <Socket> socket) {
        NS_LOG_UNCOND("Not Connected ..." << socket->GetNode()->GetId());
    }

    ClientApplication::~ClientApplication() {
    }

    void
    ClientApplication::Setup(Ptr <Socket> socket, Address address, uint32_t packetSize, uint32_t nBytesModel,
                             DataRate dataRate) {
        m_socket = socket;
        m_peer = address;
        m_packetSize = packetSize;
        m_bytesModel = nBytesModel;
        m_dataRate = dataRate;
    }

    void
    ClientApplication::StartApplication(void) {


        m_model.SetDeviceType("RaspberryPi");
        m_model.SetDataSize(DoubleValue(m_bytesModel));
        m_model.SetPacketSize(DoubleValue(m_packetSize));

        m_model.SetApplication("kNN", DoubleValue(m_packetSize));


        m_socket->SetCloseCallbacks(
                MakeCallback(&ClientApplication::NormalClose, this),
                MakeCallback(&ClientApplication::ErrorClose, this));
        m_socket->SetConnectCallback(
                MakeCallback(&ClientApplication::ConnectionSucceeded, this),
                MakeCallback(&ClientApplication::ConnectionFailed, this));



        m_socket->Bind();
        m_socket->Connect(m_peer);

        m_timeBeginReceivingModelFromServer = Simulator::Now();

        m_socket->SetRecvCallback(MakeCallback(&ClientApplication::HandleRead, this));
    }

    void
    ClientApplication::StopApplication(void) {


        if (m_sendEvent.IsRunning()) {
            Simulator::Cancel(m_sendEvent);
        }

        if (m_socket) {
            //m_socket->Close ();
        }
    }

}
