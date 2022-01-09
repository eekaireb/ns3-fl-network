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

namespace ns3 {

    class Experiment {
    public:

        Experiment(int numClients, std::string &networkType, int maxPacketSize, double txGain, double modelSize,
                   std::string &dataRate, bool bAsync, FLSimProvider *pflSymProvider);

        std::map<int, FLSimProvider::Message>
        WeakNetwork(std::map<int, std::shared_ptr<ClientSession> > &packetsReceived, ns3::Time &timeOffset);

    private:
        void SetPosition(Ptr <Node> node, double radius, double theta);

        Vector GetPosition(Ptr <Node> node);

        NetDeviceContainer Wifi(ns3::NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients);

        NetDeviceContainer Ethernet(NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients);

        int m_numClients;
        std::string m_networkType;
        int m_maxPacketSize;
        double m_txGain;
        double m_modelSize;
        std::string m_dataRate;
        bool m_bAsync;

        FLSimProvider *m_flSymProvider;

    };
}

#endif //IOT_EXPERIMENT_H
