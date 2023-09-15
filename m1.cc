#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-model.h"
using namespace ns3;


int main(int argc, char* argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create a node container
    NodeContainer nodes;
    nodes.Create(2);

    // Add WiFi channel and physical layer
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    // Add WiFi to the nodes
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate54Mbps"));

    // Create a mobility helper
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)));

    // Install the mobility model on nodes
    mobility.Install(nodes);

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();
    
    std::cout << "Successfully done" << std::endl;

    return 0;
}

