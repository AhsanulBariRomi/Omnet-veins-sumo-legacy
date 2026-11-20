//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/TraCIDemo11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        accidentResolved = false;
        currentSubscribedServiceId = -1;
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    //if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        findHost()->getDisplayString().setTagArg("i", 1, "green");

        // 1. SLAM ON THE BRAKES VIA TRACI (Replaces changeRoute)
        traciVehicle->setSpeedMode(0);
        traciVehicle->setSpeed(0.0);
        traciVehicle->setSignals(8);

        // 2. Rebroadcast the warning to cars behind
        // repeat the received traffic update once in 2 seconds plus some random delay
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

//void TraCIDemo11p::handlePositionUpdate(cObject* obj)
//{
//    DemoBaseApplLayer::handlePositionUpdate(obj);
//
//    // stopped for for at least 10s?
//    if (mobility->getSpeed() < 1) {
//        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
//            findHost()->getDisplayString().setTagArg("i", 1, "red");
//            sentMessage = true;
//
//            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
//            populateWSM(wsm);
//            wsm->setDemoData(mobility->getRoadId().c_str());
//
//            // host is standing still due to crash
//            if (dataOnSch) {
//                startService(Channel::sch2, 42, "Traffic Information Service");
//                // started service and server advertising, schedule message to self to send later
//                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
//            }
//            else {
//                // send right away on CCH, because channel switching is disabled
//                sendDown(wsm);
//            }
//        }
//    }
//    else {
//        lastDroveAt = simTime();
//    }
//}
void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // 1. Read parameters from omnetpp.ini
    int iniAccidentCount = mobility->par("accidentCount").intValue();
    double iniAccidentStart = mobility->par("accidentStart").doubleValue();
    double iniAccidentDuration = mobility->par("accidentDuration").doubleValue();

    // 2. MANUAL ACCIDENT TRIGGER
    if (findHost()->getIndex() == 0 && iniAccidentCount > 0 && simTime().dbl() >= iniAccidentStart && sentMessage == false) {

        findHost()->getDisplayString().setTagArg("i", 1, "red");
        sentMessage = true;

        // Slam the brakes via TraCI
        traciVehicle->setSpeedMode(0);
        traciVehicle->setSpeed(0.0);
        traciVehicle->setSignals(8);

        // Broadcast the warning
        TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
        populateWSM(wsm);
        wsm->setDemoData(mobility->getRoadId().c_str());

        if (dataOnSch) {
            startService(Channel::sch2, 42, "Traffic Information Service");
            scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
        } else {
            sendDown(wsm);
        }
    }

    // 3. ACCIDENT RECOVERY
    if (sentMessage == true && accidentResolved == false && simTime().dbl() >= (iniAccidentStart + iniAccidentDuration)) {
        accidentResolved = true;
        findHost()->getDisplayString().setTagArg("i", 1, "blue");

        // Give control back to SUMO (-1.0 resets the speed to default driver logic)
        traciVehicle->setSpeedMode(31);
        traciVehicle->setSpeed(-1.0);
        traciVehicle->setSignals(0);
    }
}
