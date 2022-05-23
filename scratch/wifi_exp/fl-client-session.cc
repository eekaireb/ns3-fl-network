//
// Created by emily on 1/5/22.
//


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

#include "fl-client-session.h"

namespace ns3 {

    ClientSession::ClientSession(int clientID_, double x, double y, double z) :
            m_client(nullptr), m_x(x), m_y(y),m_z(z), m_clientID(clientID_), m_cycle(0), m_inRound(true),
            m_dropOut(false)
    {

    }

    Ptr<ns3::Socket> ClientSession::GetClient()
    {
        return m_client;
    }

    void ClientSession::SetClient(Ptr<ns3::Socket> client)
    {
        m_client=client;
    }

    bool ClientSession::GetInRound()
    {
        return m_inRound;
    }

    void ClientSession::SetInRound(bool inRound)
    {
        m_inRound=inRound;
    }

    int ClientSession::GetCycle()
    {
        return m_cycle;
    }

    void ClientSession::SetCycle(int cycle)
    {
        m_cycle=cycle;
    }
    void ClientSession::IncrementCycle()
    {
        m_cycle++;
    }

    double ClientSession::GetX()
    {
        return m_x;
    }

    double ClientSession::GetY()
    {
        return m_y;
    }

    double ClientSession::GetZ()
    {
        return m_z;
    }


    int ClientSession::GetClientId()
    {
        return m_clientID;
    }


void ClientSessionManager::Init()
{
    NS_LOG_UNCOND("SIZE=" << m_clientSessionById.size() << " " << this);
    
        for (auto itr = m_clientSessionById.begin(); itr != m_clientSessionById.end(); itr++) {
            if (itr->second->GetInRound()) {
                auto clientAddress = itr->second->GetClient()->GetNode()->
                        GetObject<ns3::Ipv4>()->GetAddress(1, 0).GetLocal();

                m_clientSessionByAddress[clientAddress] = itr->second->GetClientId();
                m_nInRound++;
            }
        }
        
}


    ClientSessionManager::ClientSessionManager(std::map<int, std::shared_ptr<ClientSession> > &inn) :
            m_clientSessionById(inn), m_nInRound(0), m_nInRoundFirstCycleDone(0) {


    NS_LOG_UNCOND("PSIZE=" << m_clientSessionById.size() << " " << this);
    }

    int ClientSessionManager::ResolveToId(ns3::Ipv4Address &address) {
        return m_clientSessionByAddress[address];
    }

    void ClientSessionManager::IncrementCycleCountFromServer(ns3::Ptr<ns3::Socket> socket) {
        auto id = ResolveToIdFromServer(socket);
        if (m_clientSessionById[id]->GetCycle() == 0) {
            m_nInRoundFirstCycleDone++;
        }
        m_clientSessionById[id]->IncrementCycle();
    }

    bool ClientSessionManager::HasAllClientsFinishedFirstCycle() {
        return m_nInRoundFirstCycleDone == m_nInRound;
    }


    int ClientSessionManager::GetRound(ns3::Ptr<ns3::Socket> socket) {
        auto id = ResolveToIdFromServer(socket);
        return m_clientSessionById[id]->GetCycle();
    }




    int ClientSessionManager::ResolveToIdFromServer(ns3::Ptr<ns3::Socket> socket) {
        ns3::Address addr;
        socket->GetPeerName(addr);
        auto temp = ns3::InetSocketAddress::ConvertFrom(addr).GetIpv4();
        return ResolveToId(temp);
    }

    void ClientSessionManager::Close() {
        for (auto itr = m_clientSessionById.begin(); itr != m_clientSessionById.end(); itr++) {
            if (itr->second->GetInRound()) {
                itr->second->GetClient()->Close();
            }
        }
    }


    ns3::Ipv4Address ClientSessionManager::ResolveToAddress(int id) {
        return m_clientSessionById[id]->GetClient()->GetNode()->
                GetObject<ns3::Ipv4>()->GetAddress(1, 0).GetLocal();
    }



}

