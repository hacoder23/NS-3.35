#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

int main(int argc, char* argv[]) {
    // Create nodes
    NodeContainer nodes;
    nodes.Create(20); // Create 5 nodes

    // Install mobility model (optional)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)));
    
    mobility.Install(nodes);

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Create wireless channels and physical layers
    YansWifiChannelHelper channel;
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    // Install Wi-Fi interfaces on nodes
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate54Mbps"), "ControlMode", StringValue("OfdmRate6Mbps"));
    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // Install AODV routing protocol
    AodvHelper aodv;
    Ipv4ListRoutingHelper list;
    list.Add(aodv, 100);
    internet.SetRoutingHelper(list);
    

    // Create nodes' interfaces
    Ipv4AddressHelper address;
    address.SetBase("172.11.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Create a simple UDP application
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(200.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2048));
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(3));
    clientApps.Start(Seconds(1.0));
    clientApps.Stop(Seconds(199.0));

    // Enable flow monitoring
    FlowMonitorHelper flowMonitor;
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

    // Run the simulation
    Simulator::Stop(Seconds(200.0));
    Simulator::Run();

    // Calculate packet delivery ratio
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitor.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    uint32_t totalPacketsSent = 0;
    uint32_t totalPacketsReceived = 0;

    for (auto it = stats.begin(); it != stats.end(); ++it) {
        totalPacketsSent += it->second.txPackets;
        totalPacketsReceived += it->second.rxPackets;
    }

    double deliveryRatio = static_cast<double>(totalPacketsReceived) / static_cast<double>(totalPacketsSent);

    // Calculate network throughput
   double totalBytesReceived = 0;
   for (auto it = stats.begin(); it != stats.end(); ++it) 
   {
     totalBytesReceived += it->second.rxBytes;
     }
   double throughput = (totalBytesReceived * 8) / 200.0; // 10 seconds simulation time

   // Calculate packet loss rate
   uint32_t totalPacketsLost = totalPacketsSent - totalPacketsReceived;
   double lossRate = static_cast<double>(totalPacketsLost) / static_cast<double>(totalPacketsSent);

    // Print results
    std::cout << "Packet Delivery Ratio: " << deliveryRatio << std::endl;
    std::cout << "Network Throughput (bps): " << throughput << std::endl;
    std::cout << "Packet Loss Rate: " << lossRate << std::endl;

    // Clean up
    Simulator::Destroy();

    return 0;
}
