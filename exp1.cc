#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/aodv-helper.h"


using namespace ns3;

int main(int argc, char *argv[]) {
    // Set up the command line arguments
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create nodes
    NodeContainer nodes;
    nodes.Create(10);

    // Set mobility model
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel");
    mobility.Install(nodes);

    // Set up Wi-Fi communication
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    //NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Set up Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Set up AODV routing
    AodvHelper aodv;
    Ipv4StaticRoutingHelper staticRouting;

    // Generate traffic
    uint16_t port = 9;
    OnOffHelper onOff("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress(1), port)));
    onOff.SetAttribute("DataRate", DataRateValue(DataRate("500kbps")));
    onOff.SetAttribute("PacketSize", UintegerValue(1500));

    ApplicationContainer apps = onOff.Install(nodes.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Run simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

