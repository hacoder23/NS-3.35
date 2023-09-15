#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h" 

using namespace ns3;

int main (int argc, char *argv[])
{
    // Enable logging
    LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_INFO);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(2); // You can change the number of nodes

    // Configure WiFi
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"));

    // Create a wireless channel
    YansWifiChannelHelper channel;
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    // Install WiFi devices on nodes
    //NetDeviceContainer devices = wifi.Install(phy, nodes);

    // Set mobility model
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(0.0),
                                 "MinY", DoubleValue(0.0),
                                 "DeltaX", DoubleValue(5.0),
                                 "DeltaY", DoubleValue(5.0),
                                 "GridWidth", UintegerValue(2),
                                 "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Install AODV on nodes
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    // You can add applications and simulation logic here
    // Create a simple UDP traffic source and sink
    uint16_t port = 9;  // Port number
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));  // Install on Node 1
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port); // Send to Node 1
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0)); // Install on Node 0
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // Run the simulation
    Simulator::Stop(Seconds(11.0)); // Stop simulation after 11 seconds
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
