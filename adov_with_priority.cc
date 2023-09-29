#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/queue.h"
#include "ns3/queue-disc.h"
#include "ns3/red-queue-disc.h"

using namespace ns3;

int main(int argc, char* argv[]) {
    // Create nodes
    NodeContainer nodes;
    nodes.Create(5); // Create 5 nodes

    // Install mobility model (optional)
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY", DoubleValue(0.0));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
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
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    
// Create priority queue discipline
    RedQueueDisc redQueueDisc;
    redQueueDisc.SetAttribute("MinTh", DoubleValue(10));       // Minimum threshold
    redQueueDisc.SetAttribute("MaxTh", DoubleValue(30));       // Maximum threshold
    redQueueDisc.SetAttribute("QueueLimit", QueueSizeValue(QueueSize("100p"))); // Queue size limit

    // Set the queue discipline for network devices
   for (uint32_t i = 0; i < nodes.GetN(); ++i) {
    Ptr<Node> node = nodes.Get(i);
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    for (uint32_t j = 0; j < ipv4->GetNInterfaces(); ++j) {
        Ptr<NetDevice> device = ipv4->GetNetDevice(j);
        device->SetQueue(redQueueDisc);
    }
}

    // Create a simple UDP application
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps = echoClient.Install(nodes.Get(3));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(9.0));

    // Enable flow monitoring
    FlowMonitorHelper flowMonitor;
    Ptr<FlowMonitor> monitor = flowMonitor.InstallAll();

    // Run the simulation
    Simulator::Stop(Seconds(10.0));
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
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        totalBytesReceived += it->second.rxBytes;
    }
    double throughput = (totalBytesReceived * 8) / 10.0; // 10 seconds simulation time

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
