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
#include "fl-energy.h"



namespace ns3 {

    class Address;
    class Socket;
    class Packet;

    /**
   * \ingroup fl-server
   * \brief Server application for federated learning
   */
    class Server : public Application {

    public:
        /**
         * \brief Data structure to track statistics for each connected client
         */
        class ClientSessionData {
        public:
            ClientSessionData() : m_bytesReceived(0), m_bytesSent(0), m_bytesModelToSend(0), m_bytesModelToReceive(0) {

            }

            ns3::Time m_timeBeginReceivingModelFromClient;    //!<Set time when connected
            ns3::Time m_timeEndReceivingModelFromClient;      //!<Set time when last message received by server
            ns3::Time m_timeBeginSendingModelFromClient;      //!<Set time when connected
            ns3::Time m_timeEndSendingModelFromClient;        //!<Set time when last message is sent to client
            uint32_t m_bytesReceived;                         //!<Total number of bytes received
            uint32_t m_bytesSent;                             //!<Total number of bytes sent
            uint32_t m_bytesModelToSend;                      //!<Remaining number of bytes to send
            uint32_t m_bytesModelToReceive;                   //!<Remaining number of bytes to receive
            ns3::Address m_address;                           //!<Address of the connected client

        };


        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId(void);

        Server();
        virtual ~Server();

        /**
         * \brief Gets a list of all connected sockets and their client session
         * \return List of pointers to accepted sockets
         */
        std::map <Ptr<Socket>, std::shared_ptr<ClientSessionData>> GetAcceptedSockets(void) const;

        /**
         * \brief Sets the session manager and flsim provider
         * \param pSessionManager Session Manager for this experiment
         * \param fl_sim_provider Flsim provider for this experiment
         */
         //TODO: move to cc file
        void SetClientSessionManager(ClientSessionManager *pSessionManager, FLSimProvider *fl_sim_provider, FILE *fp, int round) {
            m_clientSessionManager = pSessionManager;
            m_fLSimProvider = fl_sim_provider;
            m_fp=fp;
            m_round=round;
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
         * \brief Sends a packet from server to client, continues
         *        to send packets until there are no remaining bytes to be sent.
         * \param socket Connected client socket in which to send bytes.
         */
        void SendModel(Ptr <Socket> socket);

        /**
         * \brief Begins the process of sending the model to the client
         */
        void StartSendingModel(Ptr <Socket> socket);

        /**
         * \brief
         * \param s
         * \param from
         * \return
         */
         //TODO: Make a null callback and delete this
        bool ConnectionRequestCallback(Ptr <Socket> s, const Address &from);

        /**
         * \brief Callback for when new connection is created
         * \param socket Newly connected socket
         * \param from Address of newly connected socket
         */
        void NewConnectionCreatedCallback(Ptr <Socket> socket, const Address &from);

        /**
         * \brief Callback used to schedule a send when TxAvailable becomes
         *        positive
         * \param sock       Socket that is ready to send
         * \param available  TxAvailable
         */
         //TODO:rename to handlereadytosend like in client file
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


        Ptr <Socket> m_socket;                                                    //!< Listening socket
        std::map <Ptr<Socket>, std::shared_ptr<ClientSessionData>> m_socketList;  //!< the accepted sockets
        ClientSessionManager *m_clientSessionManager;                             //!< Container that holds all client sessions
        Address m_local;                                                          //!< Local address to bind to
        uint64_t m_totalRx;                                                       //!< Total bytes received
        TypeId m_tid;                                                             //!< Protocol TypeId
        uint32_t m_packetSize;                                                    //!< Max packet size for server to client communication
        ns3::EventId m_sendEvent; //!< Send event handle used to cancel a pending event
        uint32_t m_bytesModel;    //!< Size of model that will be sent between the client and server
        ns3::DataRate m_dataRate; //!< Rate which data is transmitted from server to client
        bool m_bAsync;            //!< Flag that is used to configure server as sync or async
        FLSimProvider *m_fLSimProvider; //!< Communications interface with python simulator
        ns3::Time m_timeOffset;   //!< For async, offset between rounds
        FILE *m_fp;               //!< File pointer for logging.
        int m_round;              //!< Round

    };

} // namespace ns3

#endif

