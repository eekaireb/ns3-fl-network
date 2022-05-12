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
#ifndef GATEWAY_H
#define GATEWAY_H

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
   * \brief Gateway application for federated learning
   */
    class Gateway : public Application {

    public:
        /**
         * \brief Data structure to track statistics for each connected client
         */
        class ClientSessionData {
        public:
            ClientSessionData() : m_bytesReceived(0), m_bytesSent(0), m_bytesModelToSend(0), m_bytesModelToReceive(0) {

            }

            //Client to gateway
            ns3::Time m_timeBeginReceivingModelFromClient;    //!<Set time when connected
            ns3::Time m_timeEndReceivingModelFromClient;      //!<Set time when last message received by server
            ns3::Time m_timeBeginSendingModelFromClient;      //!<Set time when connected
            ns3::Time m_timeEndSendingModelFromClient;        //!<Set time when last message is sent to client
            uint32_t m_bytesReceived;                         //!<Total number of bytes received
            uint32_t m_bytesSent;                             //!<Total number of bytes sent
            uint32_t m_bytesModelToSend;                      //!<Remaining number of bytes to send
            uint32_t m_bytesModelToReceive;                   //!<Remaining number of bytes to receive
            ns3::Address m_address;                           //!<Address of the connected client


            //Client to gateway
            ns3::Time m_timeBeginReceivingModelFromServer;    //!<Set time when connected
            ns3::Time m_timeEndReceivingModelFromServer;      //!<Set time when last message received by server
            ns3::Time m_timeBeginSendingModelFromServer;      //!<Set time when connected
            ns3::Time m_timeEndSendingModelFromServer;        //!<Set time when last message is sent to client
            uint32_t m_bytesReceivedFromServer;                         //!<Total number of bytes received
            uint32_t m_bytesSentToServer;                             //!<Total number of bytes sent
            uint32_t m_bytesModelToSendToServer;                      //!<Remaining number of bytes to send
            uint32_t m_bytesModelToReceiveFromServer;                   //!<Remaining number of bytes to receive
            //ns3::Address m_address;                           //!<Address of the connected client



        };


        /**
         * \brief Get the type ID.
         * \return the object TypeId
         */
        static TypeId GetTypeId(void);

        Gateway();
        virtual ~Gateway();

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
        void GatewayHandleSend(Ptr <Socket> sock, uint32_t available);

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


        //////////////////
        //GATEWAY-TO-SERVER
        /////////////////
    public:
        /**
        * \brief Setup associates an existing socket with desired parameters.
        * \param socket       Socket to associate with gateway
        * \param address      Address for server for gateway to connect to
        * \param packetSize   Max packet size for communication from client to server
        * \param nBytesModel  Size of model that will be sent between the client and server
        * \param dataRate     Rate which data is transmitted from client to server
        */
        void Setup (Ptr <Socket> socket, Address address, uint32_t packetSize, uint32_t nBytesModel,
                    DataRate dataRate);

        /**
         * \brief Callback function for when a connection to remote host succeeds,
         * used to trigger receiving for model from server
         * \param socket The connected socket
         */
        void ConnectionSucceeded (Ptr <Socket> socket);

        /**
         * \brief Callback function to log that socket closed normally
         * \param socket The closed socket
         */
        void NormalClose (Ptr <Socket> socket);

        /**
         * \brief Callback function to log that socket closed with an error
         * \param socket The closed socket
         */
        void ErrorClose (Ptr <Socket> socket);

        /**
         * \brief Callback function for when connection to remote host failed
         * \param socket socket that failed to connect to remote host
         */
        void ConnectionFailed (Ptr <Socket> socket);

        /**
         * \brief Upon a received packet, update the bytes left to receive
         * If there are no more bytes to receive, trigger the sending process.
         * \param socket The socket which received a packet
         */
        void HandleReadFromServer (Ptr <Socket> socket);

        /**
         * \brief Begins the process of sending the model to the server
         */
        void StartWritingToClients ();

        void SendToServer(Ptr <Socket> socket);
        void HandleReadyToSendToServer(Ptr <Socket> sock, uint32_t available);

        void StartWritingToServer();

    private:

        Ptr <Socket> m_serverSocket;
        Address m_serverPeer;
        uint32_t m_serverPacketSize;                    //!< Max packet size for communication from client to server
        uint32_t m_serverBytesModel;                    //!< Size of model that will be sent between the client and server
        DataRate m_serverDataRate;                      //!< Rate which data is transmitted from client to server

        uint32_t m_serverBytesModelReceived;            //!< Number of bytes of model received
        uint32_t m_serverBytesModelToReceive;           //!< Number of bytes of model left to receive
        Time m_serverTimeBeginReceivingModelFromServer; //!< Set time when receiving model from server
        Time m_serverTimeEndReceivingModelFromServer;   //!< Set time when last message received by server

        uint32_t m_serverBytesModelToSend;              //!< Number of bytes left to send to server
        uint32_t m_serverBytesSent;                     //!< Number of bytes sent to server

        EventId m_serverSendEvent;                      //!< Send event handle used to cancel a pending event

    };

} // namespace ns3

#endif

