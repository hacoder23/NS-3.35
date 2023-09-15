#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h" // Include the NetAnim module
#include "ns3/wifi-module.h" // Include the WiFi module
#include "ns3/mobility-model.h"// For mobility model 



using namespace ns3;

int main (int argc, char* argv[])
{
    CommandLine cmd;
    cmd.Parse (argc, argv);

    // Create a node container
    NodeContainer nodes;
    nodes.Create(2);
    
   //PointToPointHelper pointToPoint;
   //pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
   //pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
   
   //NetDeviceContainer devices;
   //devices = pointToPoint.Install (nodes);
   
   //InternetStackHelper stack;
   //stack.Install (nodes);

   //Ipv4AddressHelper address;
   //address.SetBase ("10.1.1.0", "255.255.255.0");
   
   //Ipv4InterfaceContainer interfaces = address.Assign (devices);
   
   // Add WiFi channel and physical layer
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    // Add WiFi to the nodes
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate54Mbps"));
    //NetDeviceContainer wifiDevices = wifi.Install(wifiPhy, nodes);
   
   // Create a mobility helper
    MobilityHelper mobility;
    
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                          "Bounds", RectangleValue(Rectangle(-500, 500, -500, 500)));

    
   // Set mobility model to ConstantPositionMobilityModel
   //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    
   // Install the mobility model on nodes
    mobility.Install(nodes);
    
    // Get pointers to the mobility models of the nodes
    Ptr<MobilityModel> m0 = nodes.Get(0)->GetObject<MobilityModel>();
    Ptr<MobilityModel> m1 = nodes.Get(1)->GetObject<MobilityModel>();
    
    // Set initial positions for the nodes
    //m0->SetPosition(Vector(50, 10, 0)); // Adjust the coordinates as needed
    //m1->SetPosition(Vector(75, 20, 0)); // Adjust the coordinates as needed
    
    // Print initial positions (optional)
    std::cout << "Node 0 initial position: " << m0->GetPosition() << std::endl;
    std::cout << "Node 1 initial position: " << m1->GetPosition() << std::endl;
    
    Simulator::Run();
    Simulator::Destroy();

    return 0;
    
}

