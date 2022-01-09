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
#ifndef SERVER_H
#define SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/data-rate.h"
#include <unordered_map>
#include <map>
#include <memory>
#include "fl-sim-interface.h"



namespace ns3 {

    class Address;

    class Socket;

    class Packet;

    class Server : public Application {

    public:

        class ClientSessionData {
        public:
            ClientSessionData() : m_bytesReceived(0), m_bytesSent(0), m_bytesModelToSend(0), m_bytesModelToReceive(0) {

            }

            ns3::Time m_timeBeginReceivingModelFromClient;  //Set time when connected
            ns3::Time m_timeEndReceivingModelFromClient;  //Set time when last message received by server

            ns3::Time m_timeBeginSendingModelFromClient;  //Set time when connected
            ns3::Time m_timeEndSendingModelFromClient;  //Set time when last message received by server

            uint32_t m_bytesReceived;
            uint32_t m_bytesSent;
            uint32_t m_bytesModelToSend;
            uint32_t m_bytesModelToReceive;
            ns3::Address m_address;

        };


        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId(void);

        Server();

        virtual ~Server();

        /**
         * \return list of pointers to accepted sockets
         */
        std::map <Ptr<Socket>, std::shared_ptr<ClientSessionData>> GetAcceptedSockets(void) const;

        void SetClientSessionManager(ClientSessionManager *pSessionManager, FLSimProvider *fl_sim_provider) {
            m_clientSessionManager = pSessionManager;
            m_fLSimProvider = fl_sim_provider;
        }

    protected:
        virtual void DoDispose(void);


    private:
        // inherited from Application base class.
        virtual void StartApplication(void);    // Called at time specified by Start
        virtual void StopApplication(void);     // Called at time specified by Stop

        /**
         * \brief Handle a packet received by the application
         * \param socket the receiving socket
         */
        void ReceivedDataCallback(Ptr <Socket> socket);

        /**
         * \brief Handle an incoming connection
         * \param socket the incoming connection socket
         * \param from the address the connection is from
         */

        void SendModel(Ptr <Socket> socket);

        void StartSendingModel(Ptr <Socket> socket);


        bool ConnectionRequestCallback(Ptr <Socket> s, const Address &from);

        void NewConnectionCreatedCallback(Ptr <Socket> socket, const Address &from);

        void ServerHandleSend(Ptr <Socket> sock, uint32_t available);

        /**
         * \brief Handle an connection close
         * \param socket the connected socket
         */
        void HandlePeerClose(Ptr <Socket> socket);

        /**
         * \brief Handle an connection error
         * \param socket the connected socket
         */
        void HandlePeerError(Ptr <Socket> socket);

        /**
         * \brief Packet received: assemble byte stream to extract SeqTsSizeHeader
         * \param p received packet
         * \param from from address
         * \param localAddress local address
         *
         * The method assembles a received byte stream and extracts SeqTsSizeHeader
         * instances from the stream to export in a trace source.
         */
        void PacketReceived(const Ptr <Packet> &p, const Address &from, const Address &localAddress);


        Ptr <Socket> m_socket;       //!< Listening socket
        std::map <Ptr<Socket>, std::shared_ptr<ClientSessionData>> m_socketList; //!< the accepted sockets

        ClientSessionManager *m_clientSessionManager;

        Address m_local;        //!< Local address to bind to
        uint64_t m_totalRx;      //!< Total bytes received
        TypeId m_tid;          //!< Protocol TypeId
        uint32_t m_packetSize;

        ns3::EventId m_sendEvent;
        uint32_t m_bytesModel;
        ns3::DataRate m_dataRate;
        bool m_bAsync;

        FLSimProvider *m_fLSimProvider;
        ns3::Time m_timeOffset;

    };

} // namespace ns3

#endif /* PACKET_SINK_H */

