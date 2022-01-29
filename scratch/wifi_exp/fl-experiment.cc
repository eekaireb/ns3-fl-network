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

#include "fl-experiment.h"
#include "fl-client-application.h"
#include "fl-server.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "fl-server-helper.h"
#include "ns3/reliability-helper.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/reliability-module.h"
#include "ns3/yans-error-rate-model.h"

namespace ns3 {

    Experiment::Experiment(int numClients, std::string &networkType, int maxPacketSize, double txGain, double modelSize,
                           std::string &dataRate, bool bAsync, FLSimProvider *fl_sim_provider, FILE *fp, int round) :
            m_numClients(numClients),
            m_networkType(networkType),
            m_maxPacketSize(maxPacketSize),
            m_txGain(txGain),
            m_modelSize(modelSize),
            m_dataRate(dataRate),
            m_bAsync(bAsync),
            m_flSymProvider(fl_sim_provider),
            m_fp(fp),
            m_round(round) {
    }

    void
    Experiment::SetPosition(Ptr <Node> node, double radius, double theta) {
        double x = radius * sin(theta * 2 * M_PI);
        double y = radius * cos(theta * 2 * M_PI);
        double z = 0;
        Ptr <MobilityModel> mobility = node->GetObject<MobilityModel>();
        mobility->SetPosition(Vector(x, y, z));
    }

    Vector
    Experiment::GetPosition(Ptr <Node> node) {
        Ptr <MobilityModel> mobility = node->GetObject<MobilityModel>();
        return mobility->GetPosition();
    }

    NetDeviceContainer Experiment::Ethernet(NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients) {
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        NetDeviceContainer csmaDevices;
        csmaDevices = csma.Install(c);

        return csmaDevices;
    }


    NetDeviceContainer Experiment::Wifi(NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &clients) {

        WifiHelper wifi;
        WifiMacHelper wifiMac;
        YansWifiPhyHelper wifiPhy;


        YansWifiChannelHelper wifiChannel = YansWifiChannelHelper();

        //wifiPhy.Set("TxGain", DoubleValue(m_txGain));//-23.5) );
        

        wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

        //90 Weak Network
        //70 Medium
        //30 Stong
        //double trigger = 30.0;

        Ptr<UniformRandomVariable> expVar = CreateObjectWithAttributes<UniformRandomVariable> (
            "Min", DoubleValue (m_txGain),
            "Max", DoubleValue (m_txGain+30)
            );

        wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel","Variable",PointerValue(expVar));

        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
        //wifiChannel.SetPropagationDelay("ns3::RandomPropagationDelayModel", "Variable", StringValue ("ns3::UniformRandomVariable[Min=0|Max=2]"));

        //std::string phyMode("HtMcs0");
        std::string phyMode("DsssRate11Mbps");

        // Fix non-unicast data rate to be the same as that of unicast
        Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                           StringValue(phyMode));

        //wifi.SetStandard(WIFI_STANDARD_80211n_5GHZ);
        wifi.SetStandard(WIFI_STANDARD_80211b);
        // This is one parameter that matters when using FixedRssLossModel
        // set it to zero; otherwise, gain will be added
        wifiPhy.Set("RxGain", DoubleValue(0));

        // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
     //   wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);


        wifiPhy.SetChannel(wifiChannel.Create());

        

        // Add a mac and disable rate control
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue(phyMode),
                                     "ControlMode", StringValue(phyMode));

        // Set it to adhoc mode
        wifiMac.SetType("ns3::AdhocWifiMac");


        NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(c);

        int numClients = clients.size();
        for (int j = 1; j <= numClients; j++) {
            if (clients[j - 1]->GetInRound()) {

                Experiment::SetPosition(c.Get(j), clients[j - 1]->GetRadius(), clients[j - 1]->GetTheta());
            }
        }

        return devices;

    }

    const char *wifi_strings[] =
            {
                    "75kbps",
                    "125kbps",
                    "150kbps",
                    "160kbps",
                    "175kbps",
                    "200kbps",
            };

    const char *ethernet_strings[] =
            {
                    "80kbps",
                    "160kbps",
                    "320kbps",
                    "640kbps",
                    "1024kbps",
                    "2048kbps",
            };

    std::map<int, FLSimProvider::Message>
    Experiment::WeakNetwork(std::map<int, std::shared_ptr<ClientSession> > &clients, ns3::Time &timeOffset) {

        int server = 0;
        int numClients = clients.size();

        NodeContainer c;
        c.Create(numClients + 1);

        NetDeviceContainer devices;
        const char **strings = ethernet_strings;
        if (m_networkType.compare("wifi") == 0) {
            devices = Wifi(c, clients);
            strings = ethernet_strings;
        } else //assume ethernet if not specified
        {
            devices = Ethernet(c, clients);
        }


        InternetStackHelper internet;
        internet.Install(c);
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);



        //Setup Server
        ServerHelper server_helper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
        server_helper.SetAttribute("MaxPacketSize", UintegerValue(m_maxPacketSize));
        server_helper.SetAttribute("BytesModel", UintegerValue(m_modelSize));
        server_helper.SetAttribute("DataRate", StringValue(m_dataRate));
        server_helper.SetAttribute("Async", BooleanValue(m_bAsync));
        server_helper.SetAttribute("TimeOffset", TimeValue(timeOffset));
        ApplicationContainer sinkApps = server_helper.Install(c.Get(server));


        sinkApps.Start(Seconds(0.));


        Address sinkAddress(InetSocketAddress(interfaces.GetAddress(server, 0), 80));
        std::map<Ipv4Address, int> m_addrMap;
        //initialize clients
        for (int j = 1; j <= numClients; j++) {
            if (clients[j - 1]->GetInRound()) {

                // Experiment::SetPosition (c.Get (j), clients[j - 1]->radius, clients[j - 1]->theta);
                Ptr <Socket> source = Socket::CreateSocket(c.Get(j), TcpSocketFactory::GetTypeId());

                m_addrMap[c.Get((j))->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()] = j - 1;

                source->SetAttribute("ConnCount", UintegerValue(1000));
                source->SetAttribute("DataRetries", UintegerValue(100));


                Ptr <ClientApplication> app = CreateObject<ClientApplication>();

                app->Setup(source, sinkAddress, m_maxPacketSize, m_modelSize, std::string(strings[j % 6]));
                c.Get(j)->AddApplication(app);
                app->SetStartTime(Seconds(1.));
                app->SetStopTime(Seconds(1000000.0));

                clients[j - 1]->SetClient(source);
                clients[j - 1]->SetCycle(0);

            }
        }

        ClientSessionManager client_session_manager(clients);
        sinkApps.Get(0)->GetObject<ns3::Server>()->SetClientSessionManager(
            &client_session_manager,
            m_flSymProvider,
            m_fp,
            m_round
            );


        Simulator::Stop(Seconds(1000000.0));
        Simulator::Run();

        TimeValue endTime;
        sinkApps.Get(0)->GetObject<ns3::Server>()->GetAttribute("TimeOffset", endTime);
        timeOffset = endTime.Get();

        std::map<int, FLSimProvider::Message> roundStats;
        if (m_bAsync == false) {
            auto s1 = sinkApps.Get(0)->GetObject<ns3::Server>();

            auto sk = s1->GetAcceptedSockets();
            std::map <Ipv4Address, FLSimProvider::Message> stats;
            std::map<Ipv4Address, double> uplinkRound;
            for (auto itr = sk.begin(); itr != sk.end(); itr++) {
                auto beginUplink = itr->second->m_timeBeginReceivingModelFromClient;
                auto endUplink = itr->second->m_timeEndReceivingModelFromClient;
                auto clientAddress = InetSocketAddress::ConvertFrom(itr->second->m_address).GetIpv4();

                NS_LOG_UNCOND(
                        "[SERVER]  " << clientAddress << " -> 10.1.1.1" << std::endl <<
                                     "  Sent=     " << itr->second->m_bytesSent << " bytes" << std::endl <<
                                     "  Recv=     " << itr->second->m_bytesReceived << " bytes" << std::endl <<
                                     "  Begin uplink=" << beginUplink.As(Time::S) << std::endl <<
                                     "  End uplink=" << endUplink.As(Time::S) << std::endl <<
                                     "  Difference=" << (endUplink - beginUplink).As(Time::S));
                stats[clientAddress].roundTime = endUplink.GetDouble();

                stats[clientAddress].throughput = itr->second->m_bytesReceived * 8.0 / 1000.0 /
                                                  ((endUplink.GetDouble() - beginUplink.GetDouble()) / 1000000000.0);

            }

            for (int j = 1; j <= numClients; j++) {
                if (clients[j - 1]->GetInRound()) {
                    auto app = clients[j - 1]->GetClient()->GetNode()->GetApplication(0);
                    UintegerValue sent;
                    UintegerValue rec;
                    TimeValue begin;
                    TimeValue end;
                    Ipv4Address clientAddress;
                    app->GetAttribute("BytesSent", sent);
                    app->GetAttribute("BytesReceived", rec);
                    app->GetAttribute("BeginDownlink", begin);
                    app->GetAttribute("EndDownlink", end);

                    clientAddress = InetSocketAddress::ConvertFrom(
                            InetSocketAddress(interfaces.GetAddress(j, 0), 80)).GetIpv4();
                    NS_LOG_UNCOND(
                            "[CLIENT]  " << "10.1.1.1 -> " << clientAddress << std::endl <<
                                         "  Sent=" << sent.Get() << " bytes" << std::endl <<
                                         "  Recv=" << rec.Get() << " bytes" << std::endl <<
                                         "  Begin downlink=" << begin.Get().As(Time::S) << std::endl <<
                                         "  End downlink=" << end.Get().As(Time::S)
                    );
                    stats[clientAddress].roundTime =
                            (stats[clientAddress].roundTime - begin.Get().GetDouble()) / 1000000000.0;

                }
            }


            for (auto itr: stats) {

                int id = m_addrMap[itr.first];

                NS_LOG_UNCOND("ID " << id << "  ,ADDRESS: " << itr.first << " ,Round 1 Latency=" << itr.second.roundTime
                                    << "s ,Round 1 Throughput= "
                                    << itr.second.throughput << "kbps");

                roundStats[id].throughput = itr.second.throughput;
                roundStats[id].roundTime = itr.second.roundTime;
            }
        }
        Simulator::Destroy();
        return roundStats;
    }

}

