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
#include "fl-sim-interface.h"

namespace ns3 {
    void FLSimProvider::waitForConnection() {
        m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_server_fd < 0) {
            NS_LOG_UNCOND("Could not create a socket");
            exit(-1);
        }
        int opt = 1;
        setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt));

        m_address.sin_family = AF_INET;
        m_address.sin_addr.s_addr = INADDR_ANY;
        m_address.sin_port = htons(m_port);

        if (bind(m_server_fd, (struct sockaddr *) &m_address, sizeof(m_address)) == -1) {
            NS_LOG_UNCOND("Could not bind to port");
        }
        listen(m_server_fd, 3);

        int addrlen = sizeof(m_address);
        m_new_socket = accept(m_server_fd, (struct sockaddr *) &m_address,
                            (socklen_t * ) & addrlen);

        if (m_new_socket < 0) {
            exit(-1);
        }
    }

    FLSimProvider::COMMAND::Type FLSimProvider::recv(std::map<int, std::shared_ptr<ClientSession> > &packetsReceived) {
        COMMAND c;
        int len = read(m_new_socket, (char *) &c, sizeof(c));

        if (len != sizeof(COMMAND)) {
            if (len != 0) {
                NS_LOG_UNCOND("Invalid Command: Len(" << len << ")!=(" << sizeof(COMMAND) << ")");
            } else {
                NS_LOG_UNCOND("Socket closed by Python");
            }
            close(m_new_socket);
            return COMMAND::Type::EXIT;
        }

        if (c.command == COMMAND::Type::EXIT) {
            NS_LOG_UNCOND("Exit Called");
            close(m_new_socket);
            return COMMAND::Type::EXIT;
        } else if (c.command != COMMAND::Type::RUN_SIMULATION) {
            NS_LOG_UNCOND("Invalid command");
            close(m_new_socket);
            return COMMAND::Type::EXIT;
        } else if (packetsReceived.size() != c.nItems) {
            NS_LOG_UNCOND("Invalid number of clients");
            close(m_new_socket);
            return COMMAND::Type::EXIT;
        }


        int i = 0;
        for (auto it = packetsReceived.begin(); it != packetsReceived.end(); it++, i++) {
            uint32_t temp;

            if (sizeof(temp) != read(m_new_socket, (char *) &temp, sizeof(temp))) {
                NS_LOG_UNCOND("Invalid valid length received");
                return COMMAND::Type::EXIT;
            }

            it->second->SetInRound((temp == 0) ? false : true);


        }


        return c.command;
    }

    void FLSimProvider::Close() {
        close(m_new_socket);
    }

    void FLSimProvider::send(AsyncMessage *pMessage) {
        COMMAND r;
        r.command = COMMAND::Type::RESPONSE;
        r.nItems = 1;
        write(m_new_socket, (char *) &r, sizeof(r));
        write(m_new_socket, pMessage, sizeof(AsyncMessage));
    }

    void FLSimProvider::end() {
        COMMAND r;
        r.command = COMMAND::Type::ENDSIM;
        r.nItems = 0;
        write(m_new_socket, (char *) &r, sizeof(r));
    }


    void FLSimProvider::send(std::map<int, Message> &roundTime) {
        //NS_LOG_FUNCTION(this);

        COMMAND r;
        r.command = COMMAND::Type::RESPONSE;
        r.nItems = roundTime.size();
        write(m_new_socket, (char *) &r, sizeof(r));

        for (auto it = roundTime.begin(); it != roundTime.end(); it++) {
            Message &temp = it->second;
            temp.id = it->first;
            write(m_new_socket, (char *) &it->second, sizeof(Message));
        }

        roundTime.clear();
    }
}