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

#ifndef FL_SIM_INTERFACE_H
#define FL_SIM_INTERFACE_H

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"

#include "fl-client-session.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <map>
#include <memory>

namespace ns3 {

    /**
    * \ingroup fl-sim-interface
    * \brief Interface to flsim
    */
    class FLSimProvider {
    public:

        /**
         * \breif Message used to communicate the results of a sync experiment.
         */
        struct Message {
            uint64_t id;
            double roundTime;
            double throughput;
        };

        /**
         * \brief AsyncMessage used to communicate the result of a clients round in a async experiment.
         */
        struct AsyncMessage {
            uint64_t id;
            double startTime;
            double endTime;
            double throughput;
        };


        /**
         * \brief Command message used to communicate intent between
         * flsim and fl-experiment
         */
        struct COMMAND {
            enum class Type : uint32_t {
                RESPONSE       = 0,
                RUN_SIMULATION = 1,
                EXIT           = 2,
                ENDSIM         = 3,
            };

            Type command;
            uint32_t nItems;
        };

        /**
         * \brief Constructor; listening port for the flsim
         *
         * \param port
         */
        FLSimProvider(uint16_t port) : m_port(port) {}

        /**
         * \brief Wait for flsim to connect.
         */
        void waitForConnection();

        /**
         * \brief Receive next experiment to run
         *
         * \param clientSessionMap Map of client id to ClientSession
         * \return
         */
        COMMAND::Type recv(std::map<int, std::shared_ptr<ClientSession>> &clientSessionMap);

        /**
         * \brief Send the round times
         *
         * \param roundTimeMap
         */
        void send(std::map<int, Message> &roundTimeMap);

        /**
         * \brief Send an AsyncMessage
         * Used at the end of each client round
         * \param pMessage Pointer to an AsyncMessage to send
         */
        void send(AsyncMessage *pMessage);

        /**
         * \brief Send end message
         */
        void end();

        /**
         * Close socket if open
         */
        void Close();

    private:

        uint16_t m_port;              //!< Listening port number
        int m_server_fd;              //!< Listening socket file descriptor
        int m_new_socket;             //!< Session socket file descriptor
        struct sockaddr_in m_address; //!< Address data structure used to configure TCP socket.
    };
}
#endif
