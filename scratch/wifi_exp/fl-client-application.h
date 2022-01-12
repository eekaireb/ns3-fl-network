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

#include "ns3/applications-module.h"
#include "ns3/type-id.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/performance-simple-model.h"

namespace ns3
{

  /**
 * \ingroup fl-client
 * \brief CLient application for federated learning
 */
  class ClientApplication : public Application {
   public:

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    ClientApplication ();
    virtual ~ClientApplication ();

    /**
    * \brief Setup associates an existing socket with desired parameters.
    * \param socket       Socket to associate with client
    * \param address      Address for server for client to connect to
    * \param packetSize   Max packet size for communication from client to server
    * \param nBytesModel  Size of model that will be sent between the client and server
    * \param dataRate     Rate which data is transmitted from client to server
    */
    void Setup (Ptr <Socket> socket, Address address, uint32_t packetSize, uint32_t nBytesModel,
                DataRate dataRate);

   private:
    // inherited from Application base class.
    virtual void StartApplication (void);  //Called when application starts
    virtual void StopApplication (void);   //Called at the end of simulation

    /**
     * \brief Begins the process of sending the model to the server
     */
    void StartWriting ();

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
    void HandleRead (Ptr <Socket> socket);

    /**
     * \brief Sends a packet from client to server (remote host), continues
     *        to send packets until there are no remaining bytes to be sent.
     * \param socket Connected socket in which to send bytes.
     */
    void Send (Ptr <Socket> socket);

    /**
     * \brief Callback used to schedule a send when TxAvailable becomes
     *        positive
     * \param sock       Socket that is ready to send
     * \param available  TxAvailable
     */
    void HandleReadyToSend (Ptr <Socket> sock, uint32_t available);

    //Set by Setup
    Ptr <Socket> m_socket;                    //!< Socket to associate with client
    Address m_peer;                           //!< Server to connect to
    uint32_t m_packetSize;                    //!< Max packet size for communication from client to server
    uint32_t m_bytesModel;                    //!< Size of model that will be sent between the client and server
    DataRate m_dataRate;                      //!< Rate which data is transmitted from client to server

    uint32_t m_bytesModelReceived;            //!< Number of bytes of model received
    uint32_t m_bytesModelToReceive;           //!< Number of bytes of model left to receive
    Time m_timeBeginReceivingModelFromServer; //!< Set time when receiving model from server
    Time m_timeEndReceivingModelFromServer;   //!< Set time when last message received by server

    uint32_t m_bytesModelToSend;              //!< Number of bytes left to send to server
    uint32_t m_bytesSent;                     //!< Number of bytes sent to server

    EventId m_sendEvent;                      //!< Send event handle used to cancel a pending event
    PerformanceSimpleModel m_model;           //!< Performance model used to calculate computational delay.
  };
}