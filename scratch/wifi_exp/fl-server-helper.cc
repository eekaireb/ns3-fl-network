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

#include "fl-server-helper.h"
#include "ns3/string.h"
#include "ns3/names.h"

namespace ns3 {

    ServerHelper::ServerHelper(std::string protocol, Address address) {
        m_factory.SetTypeId("ns3::Server");
        m_factory.Set("Protocol", StringValue(protocol));
        m_factory.Set("Local", AddressValue(address));
    }

    void
    ServerHelper::SetAttribute(std::string name, const AttributeValue &value) {
        m_factory.Set(name, value);
    }

    ApplicationContainer
    ServerHelper::Install(Ptr <Node> node) const {
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer
    ServerHelper::Install(std::string nodeName) const {
        Ptr <Node> node = Names::Find<Node>(nodeName);
        return ApplicationContainer(InstallPriv(node));
    }

    ApplicationContainer
    ServerHelper::Install(NodeContainer c) const {
        ApplicationContainer apps;
        for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
            apps.Add(InstallPriv(*i));
        }

        return apps;
    }

    Ptr <Application>
    ServerHelper::InstallPriv(Ptr <Node> node) const {
        Ptr <Application> app = m_factory.Create<Application>();
        node->AddApplication(app);

        return app;
    }

} // namespace ns3
