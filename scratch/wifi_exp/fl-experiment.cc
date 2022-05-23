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
#include "fl-gateway.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "fl-server-helper.h"
#include "fl-gateway-helper.h"
#include "ns3/reliability-helper.h"
#include "ns3/energy-module.h"
#include "ns3/internet-module.h"
#include "ns3/reliability-module.h"
#include "ns3/yans-error-rate-model.h"
#include "ns3/point-to-point-helper.h"



#include "ns3/enum.h"
#include "ns3/hybrid-buildings-propagation-loss-model.h"
#include "ns3/propagation-environment.h"
#include <ns3/buildings-helper.h>
#include <sstream>

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
    Experiment::SetPosition(Ptr <Node> node, double x, double y, double z) {
        Ptr <MobilityModel> mobility = node->GetObject<MobilityModel>();
        //if(x==0)x=1.0;
        //if(y==0)y=1.0;
        if(z==0)z=10.0;

        mobility->SetPosition(Vector(x, y, z));
    }

    Vector
    Experiment::GetPosition(Ptr <Node> node) {
        Ptr <MobilityModel> mobility = node->GetObject<MobilityModel>();
        return mobility->GetPosition();
    }

    NetDeviceContainer Experiment::Ethernet(NodeContainer &c) {
        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        NetDeviceContainer csmaDevices;
        csmaDevices = csma.Install(c);

        return csmaDevices;
    }


    NetDeviceContainer Experiment::Wifi(NodeContainer &c) 
    {

        WifiHelper wifi;
        WifiMacHelper wifiMac;
        YansWifiPhyHelper wifiPhy;


        YansWifiChannelHelper wifiChannel = YansWifiChannelHelper();

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(c);

        ns3::Ptr<ns3::MobilityBuildingInfo> buildingInfoSensors = ns3::CreateObject<ns3::MobilityBuildingInfo>();
        ns3::BuildingsHelper::Install( c );


        
        wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

        wifiChannel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel",  "Environment",
                                       EnumValue(UrbanEnvironment), "CitySize", EnumValue(LargeCity));


        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
 	
	    wifiPhy.SetChannel(wifiChannel.Create());


        std::string phyMode("HtMcs0");


        // Fix non-unicast data rate to be the same as that of unicast
        Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                           StringValue(phyMode));

        wifi.SetStandard(WIFI_STANDARD_80211n_5GHZ);

        // This is one parameter that matters when using FixedRssLossModel
        // set it to zero; otherwise, gain will be added
        wifiPhy.Set("RxGain", DoubleValue(0));




        // Add a mac and disable rate control
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue(phyMode),
                                     "ControlMode", StringValue(phyMode));

        // Set it to adhoc mode
        wifiMac.SetType("ns3::AdhocWifiMac");


        NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);


        return devices;

    }


    NetDeviceContainer Experiment::Wifi(NodeContainer &c, std::map<int, std::shared_ptr<ClientSession> > &gateways,
                                        std::map<int, std::shared_ptr<ClientSession> > &clients) {

        WifiHelper wifi;
        WifiMacHelper wifiMac;
        YansWifiPhyHelper wifiPhy;


        YansWifiChannelHelper wifiChannel = YansWifiChannelHelper();

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(c);

        ns3::Ptr<ns3::MobilityBuildingInfo> buildingInfoSensors = ns3::CreateObject<ns3::MobilityBuildingInfo>();
        ns3::BuildingsHelper::Install( c );

        //wifiPhy.Set("TxGain", DoubleValue(m_txGain));//-23.5) );
        

        wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

        wifiChannel.AddPropagationLoss("ns3::HybridBuildingsPropagationLossModel",  "Environment",
                                       EnumValue(UrbanEnvironment), "CitySize", EnumValue(LargeCity));


        //90 Weak Network
        //70 Medium
        //30 Stong
        //double trigger = 30.0;
#if 0
        Ptr<UniformRandomVariable> expVar = CreateObjectWithAttributes<UniformRandomVariable> (
            "Min", DoubleValue (m_txGain),
            "Max", DoubleValue (m_txGain+30)
            );

        wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel","Variable",PointerValue(expVar));
#endif
        wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
 	
	wifiPhy.SetChannel(wifiChannel.Create());

        //wifiChannel.SetPropagationDelay("ns3::RandomPropagationDelayModel", "Variable", StringValue ("ns3::UniformRandomVariable[Min=0|Max=2]"));

        std::string phyMode("HtMcs0");
        //std::string phyMode("DsssRate11Mbps");

        // Fix non-unicast data rate to be the same as that of unicast
        Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",
                           StringValue(phyMode));

        wifi.SetStandard(WIFI_STANDARD_80211n_5GHZ);
        //wifi.SetStandard(WIFI_STANDARD_80211b);
        // This is one parameter that matters when using FixedRssLossModel
        // set it to zero; otherwise, gain will be added
        wifiPhy.Set("RxGain", DoubleValue(0));

        // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
        //wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);



        // Add a mac and disable rate control
        wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                     "DataMode", StringValue(phyMode),
                                     "ControlMode", StringValue(phyMode));

        // Set it to adhoc mode
        wifiMac.SetType("ns3::AdhocWifiMac");


        NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);



        //0.0,0.0,163.15896056
        //-712.7249999999847,280.9854000000179,139.91086307
        //-385.35599999993053,-128.99309999997897,49.58342741

        Experiment::SetPosition(c.Get(0),-385.35599999993053,-128.99309999997897,49.58342741);
        int index = 1;

        int numGateways = gateways.size();
        for (int j = 0; j < numGateways; j++) {
            if (gateways[j]->GetInRound()) {

                Experiment::SetPosition(c.Get(index), gateways[j]->GetX(), gateways[j]->GetY(), gateways[j]->GetZ() );
            }
            index++;
        }

        int numClients = clients.size();
        for (int j = 0; j < numClients; j++) {
            if (clients[j]->GetInRound()) {

                Experiment::SetPosition(c.Get(index), clients[j]->GetX(), clients[j]->GetY(), clients[j]->GetZ()  );
            }
            index++;
        }

       ///int ccc=c.GetN();
        for(int i=0;i<2;i++)
        {
            Vector v=GetPosition(c.Get(i));
        
            
            NS_LOG_UNCOND(v.x << "," << v.y << "," << v.z);
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
    Experiment::WeakNetwork(std::map<int, std::shared_ptr<ClientSession> > &gateways,
                            std::map<int, std::shared_ptr<ClientSession> > &clients, ns3::Time &timeOffset,
                            std::vector<NodeConfiguration> &clientToGateway) {



        int server  = 0;


        NodeContainer server_c;
        server_c.Create(1);

        NodeContainer gateway_c;
        gateway_c.Create(gateways.size());

        NodeContainer clients_c;
        clients_c.Create(clients.size());

        NodeContainer c(server_c, gateway_c, clients_c);

        //Build Gateway Clients
        std::map<int, std::map<int, std::shared_ptr<ClientSession>>> gatewayClients;

        for(auto o:gateways)
        {
            gatewayClients[o.first]=std::map<int, std::shared_ptr<ClientSession>>();
            o.second->SetInRound(false);
        }

        for(auto o:clientToGateway)
        {
            auto itr = gatewayClients.find(o.gateway_node);
            if(itr != gatewayClients.end())
            {
                gateways[o.gateway_node]->SetInRound(true); //Enable gateway
                itr->second[o.client_node]=clients[o.client_node];
            }
        }


        //Build Client Session Managers
        std::vector<std::shared_ptr<ClientSessionManager>> gatewayClientSessions;

        for(auto o:gatewayClients)
        {
            gatewayClientSessions.push_back(std::make_shared<ClientSessionManager>(o.second));
        }




//////////////////////////////////////////////////

        InternetStackHelper internet;
        internet.Install(c);


        NodeContainer eth(server_c,gateway_c);
        NetDeviceContainer ethDevices = Ethernet(eth);



        //For each gateway; assign wifi network
        std::vector<NodeContainer> gatewayClientNodeContiners;

        int pindex = 0;
        for(auto itr=gateway_c.Begin(); itr!=gateway_c.End(); itr++, pindex ++)
        {
 

            auto temp = NodeContainer(*itr);

            for(auto o: clientToGateway)
            {
                if(o.gateway_node == pindex)
                {
                    temp.Add(clients_c.Get(o.client_node));
                }
            }
            
            gatewayClientNodeContiners.push_back(temp);            
        }





        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");

        //Assign IP to each ethDevice
        auto ethIpv4InterfaceContainer = ipv4.Assign (ethDevices);


        //Assign IP to each wireleses device
        int cnt=3;
        std::vector<NetDeviceContainer> gatewayClientNetDeviceContiners;
        std::vector<Ipv4InterfaceContainer> gatewayClientIpv4InterfaceContainers;
        for(auto o:gatewayClientNodeContiners)
        {
            auto w=Wifi(o);
            std::string str("10." + std::to_string(cnt++) + ".1.0");
            
            ipv4.SetBase(str.c_str(), "255.255.255.0");

            gatewayClientIpv4InterfaceContainers.push_back(ipv4.Assign(w));
            gatewayClientNetDeviceContiners.push_back(w);
        }

#if 1
        int index = 0;
        for(auto itr=gateway_c.Begin(); itr!=gateway_c.End(); itr++,index++)
        {
            Experiment::SetPosition(*itr, gateways[index]->GetX(), gateways[index]->GetY(), gateways[index]->GetZ() );
        }

        index = 0;
        for(auto itr=clients_c.Begin(); itr!=clients_c.End(); itr++,index++)
        {
            Experiment::SetPosition(*itr, clients[index]->GetX(), clients[index]->GetY(), clients[index]->GetZ() );
        
        }
#endif


////////////////////////////////////////////////////


        











        //Ipv4AddressHelper ipv4_2;
        //ipv4_2.SetBase("10.2.1.0", "255.255.255.0");
        //Ipv4InterfaceContainer interfaces2 = ipv4_2.Assign(wifiDevices);
        const char **strings = ethernet_strings;

   #if 0
        NetDeviceContainer devices;
        const char **strings = ethernet_strings;
        if (m_networkType.compare("wifi") == 0) {
            devices = Wifi(c,gateways,clients);
            strings = ethernet_strings;
        } else //assume ethernet if not specified
        {
            devices = Ethernet(c);
        }





        InternetStackHelper internet;
        internet.Install(c);
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
#endif


        //Setup Server
        ServerHelper server_helper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
        server_helper.SetAttribute("MaxPacketSize", UintegerValue(m_maxPacketSize));
        server_helper.SetAttribute("BytesModel", UintegerValue(m_modelSize));
        server_helper.SetAttribute("DataRate", StringValue(m_dataRate));
        server_helper.SetAttribute("Async", BooleanValue(m_bAsync));
        server_helper.SetAttribute("TimeOffset", TimeValue(timeOffset));
        ApplicationContainer serverApps = server_helper.Install(c.Get(server));
       // Address serverAddress(InetSocketAddress(c.Get(server)->GetObject<Ipv4>()->GetAddress(server, 0).GetLocal(), 80));

        Address serverAddress(InetSocketAddress(ethIpv4InterfaceContainer.GetAddress(server, 0), 80));

        //clients_c.Get((client_id))->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()
     
        ClientSessionManager gw_session_manager(gateways);
        serverApps.Get(0)->GetObject<ns3::Server>()->SetClientSessionManager(
                &gw_session_manager,
                m_flSymProvider,
                m_fp,
                m_round
        );


        serverApps.Start(Seconds(0.));


        //initialize gateways
        GatewayHelper gateway_helper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 80));
        ApplicationContainer gatewayApps = gateway_helper.Install(gateway_c);


        for(auto o:gateways)
        {
            if(o.second->GetInRound() == false)
            {
                continue;
            }
           //Create a new socket and set attributes
           Ptr <Socket> gw_source = Socket::CreateSocket(gateway_c.Get(o.first), TcpSocketFactory::GetTypeId());
           gw_source->SetAttribute("ConnCount", UintegerValue(1000));
           gw_source->SetAttribute("DataRetries", UintegerValue(100));


           auto gh=gatewayApps.Get(o.first)->GetObject<Gateway>();
           gh->Setup(gw_source, serverAddress, m_maxPacketSize, m_modelSize, std::string(strings[0])); 
           gh->SetAttribute("MaxPacketSize", UintegerValue(m_maxPacketSize));
           gh->SetAttribute("BytesModel", UintegerValue(m_modelSize));
           gh->SetAttribute("DataRate", StringValue(m_dataRate));
           gh->SetAttribute("Async", BooleanValue(m_bAsync));
           gh->SetAttribute("TimeOffset", TimeValue(timeOffset));

           gh->SetClientSessionManager(
            gatewayClientSessions[o.first].get(),
            m_flSymProvider,
            m_fp,
            m_round
            );


           gateways[o.first]->SetClient(gw_source);
           gateways[o.first]->SetCycle(0);
        }
        gatewayApps.Start(Seconds(0.));
     




        std::map<Ipv4Address, int> m_clientIdAddrMap;
        for(auto o:gatewayClients)
        {

            int gateway_id = o.first;
            auto gateway_address = 
                InetSocketAddress(gatewayApps.Get(gateway_id)->GetNode()->GetObject<Ipv4>()->GetAddress(2, 0).GetLocal(),80);


            NS_LOG_UNCOND("GW:" <<gatewayApps.Get(gateway_id)->GetNode()->GetObject<Ipv4>()->GetAddress(2, 0).GetLocal());
            //NS_LOG_UNCOND()



            for(auto j:o.second)
            {

                int client_id=j.first;

                //If not in round; skip
                if(!j.second->GetInRound())
                {
                    continue;
                }

                NS_LOG_UNCOND("   CL:" << clients_c.Get((client_id))->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());

                m_clientIdAddrMap[clients_c.Get((client_id))->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()] = client_id;


              //  NS_LOG_UNCOND(gatewayApps.Get(gateway_id)->GetNode()->GetObject<Ipv4>()->GetAddress(2, 0).GetLocal());
              //  NS_LOG_UNCOND(clients_c.Get(client_id)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
              //  exit(0);


                Ptr <Socket> source = Socket::CreateSocket(clients_c.Get(client_id), TcpSocketFactory::GetTypeId());
                source->SetAttribute("ConnCount", UintegerValue(1000));
                source->SetAttribute("DataRetries", UintegerValue(100));


                Ptr <ClientApplication> app = CreateObject<ClientApplication>();

                app->Setup(source, gateway_address, m_maxPacketSize, m_modelSize, std::string(strings[client_id % 6]));
                clients_c.Get(client_id)->AddApplication(app);
                app->SetStartTime(Seconds(1.));
                app->SetStopTime(Seconds(1000000.0));

                clients[client_id]->SetClient(source);
                clients[client_id]->SetCycle(0);



            }

        }

        NS_LOG_UNCOND("<>");


        for(auto o:gatewayClientSessions)
        {
            o->Init();
        }






   
        gw_session_manager.Init();


        Simulator::Stop(Seconds(1000000.0));
        Simulator::Run();

        TimeValue endTime;
        serverApps.Get(0)->GetObject<ns3::Server>()->GetAttribute("TimeOffset", endTime);
        timeOffset = endTime.Get();

        std::map<int, FLSimProvider::Message> roundStats;
        if (m_bAsync == false) {
            auto s1 = serverApps.Get(0)->GetObject<ns3::Server>();

            auto sk = s1->GetAcceptedSockets();
            std::map <Ipv4Address, FLSimProvider::Message> stats;
            std::map<Ipv4Address, double> uplinkRound;



            for (auto itr = sk.begin(); itr != sk.end(); itr++) {
                auto beginUplink = itr->second->m_timeBeginReceivingModelFromClient;
                auto endUplink = itr->second->m_timeEndReceivingModelFromClient;
                auto clientAddress = InetSocketAddress::ConvertFrom(itr->second->m_address).GetIpv4();

                NS_LOG_UNCOND(
                        "[GATEWAY TO SERVER]  " << clientAddress << " -> 10.1.1.1" << std::endl <<
                                     "  Sent=     " << itr->second->m_bytesSent << " bytes" << std::endl <<
                                     "  Recv=     " << itr->second->m_bytesReceived << " bytes" << std::endl <<
                                     "  Begin uplink=" << beginUplink.As(Time::S) << std::endl <<
                                     "  End uplink=" << endUplink.As(Time::S) << std::endl <<
                                     "  Difference=" << (endUplink - beginUplink).As(Time::S));
                stats[clientAddress].roundTime = endUplink.GetDouble();

                stats[clientAddress].throughput = itr->second->m_bytesReceived * 8.0 / 1000.0 /
                                                  ((endUplink.GetDouble() - beginUplink.GetDouble()) / 1000000000.0);

            }


            int gw_cnt = 0;
            for(auto o:gateways)
            {
                if(!o.second->GetInRound())
                {
                    continue;
                }
                
                auto s1 = gatewayApps.Get(o.first)->GetObject<ns3::Gateway>();
                auto sk = s1->GetAcceptedSockets();
                auto gw_address = (InetSocketAddress(gateway_c.Get(gw_cnt)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 80)).GetIpv4();

                for (auto itr = sk.begin(); itr != sk.end(); itr++) {
                    auto beginUplink = itr->second->m_timeBeginReceivingModelFromClient;
                    auto endUplink = itr->second->m_timeEndReceivingModelFromClient;
                    auto clientAddress = InetSocketAddress::ConvertFrom(itr->second->m_address).GetIpv4();

                    


                    NS_LOG_UNCOND(
                            "[CLIENT TO GATEWAY]  " << clientAddress << " -> " << gw_address << std::endl <<
                                        "  Sent=     " << itr->second->m_bytesSent << " bytes" << std::endl <<
                                        "  Recv=     " << itr->second->m_bytesReceived << " bytes" << std::endl <<
                                        "  Begin uplink=" << beginUplink.As(Time::S) << std::endl <<
                                        "  End uplink=" << endUplink.As(Time::S) << std::endl <<
                                        "  Difference=" << (endUplink - beginUplink).As(Time::S));
                    stats[clientAddress].roundTime = endUplink.GetDouble();

                    stats[clientAddress].throughput = itr->second->m_bytesReceived * 8.0 / 1000.0 /
                                                    ((endUplink.GetDouble() - beginUplink.GetDouble()) / 1000000000.0);

                }

                gw_cnt++;
            }



            for(auto o:gateways)
            {
                if(!o.second->GetInRound())
                {
                    continue;
                }
                auto app = o.second->GetClient()->GetNode()->GetApplication(0);
                    UintegerValue sent;
                    UintegerValue rec;
                    TimeValue begin;
                    TimeValue end;
                    Ipv4Address clientAddress;
                    app->GetAttribute("BytesSentToServer", sent);
                    app->GetAttribute("BytesReceivedFromServer", rec);
                    app->GetAttribute("BeginDownlink", begin);
                    app->GetAttribute("EndDownlink", end);

                    clientAddress = (InetSocketAddress(gateway_c.Get(o.first)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 80)).GetIpv4();

                    //clientAddress = InetSocketAddress::ConvertFrom(
                    //        InetSocketAddress(interfaces.GetAddress(o.first, 0), 80)).GetIpv4();
                    NS_LOG_UNCOND(
                            "[SERVER TO GATEWAY]  " << "10.1.1.1 -> " << clientAddress << std::endl <<
                                         "  Sent=" << sent.Get() << " bytes" << std::endl <<
                                         "  Recv=" << rec.Get() << " bytes" << std::endl <<
                                         "  Begin downlink=" << begin.Get().As(Time::S) << std::endl <<
                                         "  End downlink=" << end.Get().As(Time::S)
                    );
                   stats[clientAddress].roundTime =
                           (stats[clientAddress].roundTime - begin.Get().GetDouble()) / 1000000000.0;

            }


        

        for(auto o:gatewayClients)
        {

            int gateway_id = o.first;
            auto gateway_address = 
                gatewayApps.Get(gateway_id)->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

            for(auto j:o.second)
            {

                int client_id=j.first;

                //If not in round; skip
                if(!j.second->GetInRound())
                {
                    continue;
                }

                auto app = j.second->GetClient()->GetNode()->GetApplication(0);

                    UintegerValue sent;
                    UintegerValue rec;
                    TimeValue begin;
                    TimeValue end;
                    Ipv4Address clientAddress;


                    app->GetAttribute("BytesSent", sent);
                    app->GetAttribute("BytesReceived", rec);
                    app->GetAttribute("BeginDownlink", begin);
                    app->GetAttribute("EndDownlink", end);
 


                    clientAddress = clients_c.Get(client_id)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

                    NS_LOG_UNCOND(
                            "[GATEWAY TO CLIENT]  " <<   gateway_address <<" -> "  << clientAddress << std::endl <<
                                         "  Sent=" << sent.Get() << " bytes" << std::endl <<
                                         "  Recv=" << rec.Get() << " bytes" << std::endl <<
                                         "  Begin downlink=" << begin.Get().As(Time::S) << std::endl <<
                                         "  End downlink=" << end.Get().As(Time::S)
                    );
                    stats[clientAddress].roundTime =
                            (stats[clientAddress].roundTime - begin.Get().GetDouble()) / 1000000000.0;

            }

        }


            FILE *fp=fopen("gw5.csv","w");
            for (auto itr: stats) {

                auto idx = m_clientIdAddrMap.find(itr.first);
                if(idx==m_clientIdAddrMap.end()){
                    continue;
                }

                int id = idx->second;

                NS_LOG_UNCOND("ID " << id << "  ,ADDRESS: " << itr.first << " ,Round 1 Latency=" << itr.second.roundTime
                                    << "s ,Round 1 Throughput= "
                                    << itr.second.throughput << "kbps");

                roundStats[id].throughput = itr.second.throughput;
                roundStats[id].roundTime = itr.second.roundTime;

                fprintf(fp,"%i,%lf\n",id,itr.second.roundTime);
            }
            fclose(fp);
        }
        Simulator::Destroy();
        return roundStats;
    }

}

