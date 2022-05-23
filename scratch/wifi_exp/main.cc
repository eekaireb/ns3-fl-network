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
#include <random>
#include <chrono>

#include<stdio.h>
#include<stdint.h>
#include<vector>


class Point{
        public:
        double m_x;
        double m_y;
        double m_z;

        Point(double x, double y, double z):m_x(x), m_y(y), m_z(z){}
};

//NodeConfiguration

void ReadCsvCfg(const char *fn, std::vector<NodeConfiguration> &points)
{
        FILE *stream = fopen(fn,"r");
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;
        int32_t x,y;


        while ((nread = getline(&line, &len, stream)) != -1) {
                sscanf(line,"%i,%i",&x,&y);
                points.emplace_back(NodeConfiguration(x,y));
        }
}


void ReadCsvLoc(const char *fn, std::vector<Point> &points)
{
        FILE *stream = fopen(fn,"r");
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;
        double x,y,z;


        while ((nread = getline(&line, &len, stream)) != -1) {
                sscanf(line,"%lf,%lf,%lf",&x,&y,&z);
                points.push_back(Point(x,y,z));
        }
}



using sysclock_t = std::chrono::system_clock;

using namespace ns3;
FLSimProvider g_fLSimProvider(8080);
std::map<int, std::shared_ptr<ClientSession> > g_clients;
std::map<int, std::shared_ptr<ClientSession> > g_gateways;

NS_LOG_COMPONENT_DEFINE ("Wifi-Adhoc");

int main(int argc, char *argv[]) {

   //LogComponentEnable("PropagationLossModel", LOG_LEVEL_ALL);

    FLSimProvider *flSimProvider = nullptr;//&g_fLSimProvider;


    std::vector<Point> client_points;
    std::vector<Point> gateway_points;
    std::vector<NodeConfiguration> nodeConfiguration;
    ReadCsvLoc("client_loc2.txt",client_points);
    ReadCsvLoc("gateway_loc2.txt",gateway_points);
    ReadCsvCfg("gw_10.txt", nodeConfiguration);


    std::string dataRate = "256kbps";                  /* Application layer datarate. */
    int numClients = client_points.size();
    ; //when numClients is 50 or greater, packets are not recieved by server
    int numGateways = gateway_points.size();
    std::string NetworkType = "wifi";
    int MaxPacketSize = 1024; //bytes
    double Loss = 0.0; //dB + 30 = dBm
    double ModelSize = 600; // kb
    std::string learningModel = "sync";
    std::string deviceType = "4";


    CommandLine cmd(__FILE__);

    cmd.AddValue("NumClients", "Number of clients", numClients);
    cmd.AddValue("NetworkType", "Type of network", NetworkType);
    cmd.AddValue("MaxPacketSize", "Maximum size packet that can be sent", MaxPacketSize);
    cmd.AddValue("Loss", "Mean Propogation Loss", Loss);
    cmd.AddValue("ModelSize", "Size of model", ModelSize);
    cmd.AddValue("DataRate", "Application data rate", dataRate);
    cmd.AddValue("LearningModel", "Async or Sync federated learning", learningModel);
    cmd.AddValue("DeviceType", "Device Type for Clients", deviceType);

    cmd.Parse(argc, argv);

    bool bAsync = false;
    if (learningModel.compare("async") == 0) {
        bAsync = true;
    }



    ModelSize = ModelSize * 1000; // conversion to bytes

    NS_LOG_UNCOND(
            "{NumGateways: " <<numGateways << "NumClients:" << numClients << ","
                                            "NetworkType:" << NetworkType << ", MaxPacketSize:"
                                            << MaxPacketSize << ", Loss:" << Loss << "}"
    );
    //Experiment experiment(numClients,NetworkType,MaxPacketSize,TxGain);


    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());

    char buf[80] = { 0 };
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%H-%S.csv", std::localtime(&now));

    char strbuff[100];
    snprintf(strbuff,99,"%s_%s_%.2f_%s",
             learningModel.c_str(),
             NetworkType.c_str(),
             Loss,
             buf);

    FILE *fp=fopen(strbuff,"w");



    std::default_random_engine generator;
    std::uniform_real_distribution<double> r_dist(1.0, 4.0);
    //std::uniform_real_distribution<double> t_dist(0,1.0);

    int cnt=0;
    for(auto o:gateway_points)
    {
        g_gateways[cnt] = std::shared_ptr<ClientSession>(new ClientSession(cnt, o.m_x, o.m_y, o.m_z));
	cnt++;
    }

    cnt=0;
    for(auto o:client_points)
    {
        g_clients[cnt] = std::shared_ptr<ClientSession>(new ClientSession(cnt, o.m_x, o.m_y, o.m_z));
	cnt++;
    }



    ns3::Time timeOffset(0);

    if (flSimProvider) {
        g_fLSimProvider.waitForConnection();
      }

    int round = 0;

    while (true) {

        round ++;

        if (flSimProvider) {
            FLSimProvider::COMMAND::Type type = g_fLSimProvider.recv(g_clients);

            if (type == FLSimProvider::COMMAND::Type::EXIT) {
                g_fLSimProvider.Close();
                break;
            }
        }

        auto experiment = Experiment(numClients,
                                     NetworkType,
                                     MaxPacketSize,
                                     Loss,
                                     ModelSize,
                                     dataRate,
                                     bAsync,
                                     flSimProvider,
                                      fp, round

        );
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        auto roundStats = experiment.WeakNetwork(g_gateways, g_clients, timeOffset,nodeConfiguration);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::string s = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count());
        //NS_LOG_UNCOND("TIME IN NETWORk SIM " << s);
        NS_LOG_UNCOND(">>>>>>>>>>>>>>>>>>>>>>>>>\nTIME_OFFSET:" << timeOffset << "IN SIM "<< s << "\n" ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

        if (flSimProvider && !bAsync) {
            g_fLSimProvider.send(roundStats);
        }
        if (!flSimProvider) {
            break;
        }

        fflush(fp);

    }

    fclose(fp);
    NS_LOG_UNCOND("Exiting c++");

    return 0;
}
