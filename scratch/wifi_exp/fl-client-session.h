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

#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <cstdint>
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
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include <cstring>
#include<memory>
#include <map>

namespace ns3 {
    class Socket;


    class ClientSession {
    public:
        ClientSession(int clientID_, double radius_, double theta_) ;

        Ptr<ns3::Socket> GetClient();
        void SetClient(Ptr<ns3::Socket> client);
        bool GetInRound();
        void SetInRound(bool inRound);
        int GetCycle();
        void SetCycle(int cycle);
        void IncrementCycle();
        double GetRadius();
        double GetTheta();
        int GetClientId();


    private:
        ns3::Ptr<ns3::Socket> m_client;
        double m_radius;
        double m_theta;
        int m_clientID;
        int m_cycle;
        bool m_inRound;
        bool m_dropOut;
    };

    class ClientSessionManager {
    public:
        ClientSessionManager(std::map<int, std::shared_ptr<ClientSession> > &inn);

        int ResolveToId(ns3::Ipv4Address &address);

        void IncrementCycleCountFromServer(ns3::Ptr<ns3::Socket> socket);

        bool HasAllClientsFinishedFirstCycle();


        int GetRound(ns3::Ptr<ns3::Socket> socket);




        int ResolveToIdFromServer(ns3::Ptr<ns3::Socket> socket);

        void Close();


        ns3::Ipv4Address ResolveToAddress(int id);
    private:
        std::map<ns3::Ipv4Address, int> m_clientSessionByAddress;
        std::map<int, std::shared_ptr<ClientSession> > &m_clientSessionById;
        int m_nInRound;
        int m_nInRoundFirstCycleDone;

    };

}
#endif
