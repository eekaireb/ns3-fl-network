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
    class FLSimProvider {
    public:
        struct Message {
            uint64_t id;
            double roundTime;
            double throughput;
        };

        struct AsyncMessage {
            uint64_t id;
            double startTime;
            double endTime;
            double throughput;
        };


        struct COMMAND {
            enum class Type : uint32_t {
                RESPONSE = 0,
                RUN_SIMULATION = 1,
                EXIT = 2,
                INIT = 3,
                ENDSIM = 4,
            };

            Type command;
            uint32_t nItems;
        };

        FLSimProvider(uint16_t port) : m_port(port) {}

        void waitForConnection();

        COMMAND::Type recv(std::map<int, std::shared_ptr<ClientSession>> &in);

        void send(std::map<int, Message> &roundTime);

        void send(AsyncMessage *pMessage);

        void end();

        void Close();

    private:

        uint16_t m_port;
        int m_server_fd;
        int m_new_socket;
        struct sockaddr_in m_address;
    };
}
#endif
