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

using sysclock_t = std::chrono::system_clock;

using namespace ns3;
FLSimProvider g_fLSimProvider(8080);
std::map<int, std::shared_ptr<ClientSession> > g_clients;

NS_LOG_COMPONENT_DEFINE ("Wifi-Adhoc");

int main(int argc, char *argv[]) {

   //LogComponentEnable("PropagationLossModel", LOG_LEVEL_ALL);

    FLSimProvider *flSimProvider = &g_fLSimProvider;


    std::string dataRate = "250kbps";                  /* Application layer datarate. */
    int numClients = 20; //when numClients is 50 or greater, packets are not recieved by server
    std::string NetworkType = "wifi";
    int MaxPacketSize = 1024; //bytes
    double TxGain = 0.0; //dB + 30 = dBm
    double ModelSize = 1.500 * 10; // kb
    std::string learningModel = "sync";


    CommandLine cmd(__FILE__);

    cmd.AddValue("NumClients", "Number of clients", numClients);
    cmd.AddValue("NetworkType", "Type of network", NetworkType);
    cmd.AddValue("MaxPacketSize", "Maximum size packet that can be sent", MaxPacketSize);
    cmd.AddValue("TxGain", "Power transmitted from clients and server", TxGain);
    cmd.AddValue("ModelSize", "Size of model", ModelSize);
    cmd.AddValue("DataRate", "Application data rate", dataRate);
    cmd.AddValue("LearningModel", "Async or Sync federated learning", learningModel);


    cmd.Parse(argc, argv);

    bool bAsync = false;
    if (learningModel.compare("async") == 0) {
        bAsync = true;
    }


    ModelSize = ModelSize * 1000; // conversion to bytes

    NS_LOG_UNCOND(
            "{NumClients:" << numClients << ","
                                            "NetworkType:" << NetworkType << ","
                                                                             "MaxPacketSize:" << MaxPacketSize << ","
                                                                                                                  "TxGain:"
                           << TxGain << "}"
    );
    //Experiment experiment(numClients,NetworkType,MaxPacketSize,TxGain);


    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());

    char buf[80] = { 0 };
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%H-%S.csv", std::localtime(&now));

    char strbuff[100];
    snprintf(strbuff,99,"%s_%s_%.2f_%s",
             learningModel.c_str(),
             NetworkType.c_str(),
             TxGain,
             buf);

    FILE *fp=fopen(strbuff,"w");





    std::default_random_engine generator;
    std::uniform_real_distribution<double> r_dist(1.0, 4.0);
    //std::uniform_real_distribution<double> t_dist(0,1.0);

    //initialize structure for all clients
    for (int j = 0; j < numClients; j++) {

        //place the nodes at random spots from the base station

        double radius = (double) (5 << (j % 4 + 2));
        //double theta = t_dist(generator);
        double theta = (1.0 / numClients) * (j);

        NS_LOG_UNCOND("INIT:J=" << j << " r=" << radius << " th=" << theta);
        g_clients[j] = std::shared_ptr<ClientSession>(new ClientSession(j, radius, theta));
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
                                     TxGain,
                                     ModelSize,
                                     dataRate,
                                     bAsync,
                                     flSimProvider,
                                      fp, round

        );
        auto roundStats = experiment.WeakNetwork(g_clients, timeOffset);

        NS_LOG_UNCOND(">>>>>>>>>>>>>>>>>>>>>>>>>\nTIME_OFFSET:" << timeOffset << "\n" ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

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
