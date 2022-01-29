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

#ifndef IOT_EXPERIMENT_H
#define IOT_EXPERIMENT_H

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "fl-sim-interface.h"
#include "fl-client-session.h"

#include <memory>
#include <string>

#include <cstdio>

namespace ns3 {
    /**
  * \ingroup fl-experiment
  * \brief Sets up and runs fl experiments
  */
    class Experiment {
    public:
        /**
        * \brief Constructs Experiment
        * \param numClients      Number of clients in experiment
        * \param networkType     Network type (wifi or ethernet)
        * \param maxPacketSize   Max packet size for network
        * \param txGain          TX gain for wifi network
        * \param modelSize       Model size
        * \param dataRate        Datarate for server
        * \param bAsync          If running async experiment, true
        * \param pflSymProvider  pointer to an fl-sim-interface (used to communicate with flsim)
        */
        Experiment(int numClients, std::string &networkType, int maxPacketSize, double txGain, double modelSize,
                   std::string &dataRate, bool bAsync, FLSimProvider *pflSymProvider, FILE *fp, int round);

        /**
        * \brief Runs network experiment
        * \param packetsReceived   map of <client, client sessions>
        * \param timeOffset        Async, make timeline between rounds continious
        * \return                  map of <client id, message>, messages to send back to flsim for each client
        */
        //TODO: Change to run and change packet recieved
        std::map<int, FLSimProvider::Message>
        WeakNetwork(std::map<int, std::shared_ptr<ClientSession> > &packetsReceived, ns3::Time &timeOffset);

    private:
        /**
        * \brief Set position of node in network
        * \param node        Node to set position of
        * \param radius      Radius location of node
        * \param theta       Angular location of node
        */
        void SetPosition(Ptr <Node> node, double radius, double theta);

        /**
        * \brief Gets position of node
        * \param node   Node to get position of
        * \return       Vector of node position
        */
        Vector GetPosition(Ptr <Node> node);

        /**
        * \brief Sets up wifi network
        */
        NetDeviceContainer Wifi(ns3::NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients);

        /**
        * \brief Sets up ethernet network
        */
        NetDeviceContainer Ethernet(NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients);

        int m_numClients;                 //!< Number of clients in experiment
        std::string m_networkType;        //!< Network type
        int m_maxPacketSize;              //!< Max packet size
        double m_txGain;                  //!< TX gain (for wifi network)
        double m_modelSize;               //!< Size of model
        std::string m_dataRate;           //!< Datarate for server
        bool m_bAsync;                    //!< Indicator bool for whether experiement is async
        FLSimProvider *m_flSymProvider;   //!< pointer to an fl-sim-interface (used to communicate with flsim)
        FILE *m_fp;                       //!< pointer to logfile
        int m_round;                      //!< experiment round
    };
}

#endif //IOT_EXPERIMENT_H
