 #include <iostream>
 #include <cmath>
 #include "ns3/aodv-module.h"
 #include "ns3/core-module.h"
 #include "ns3/network-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/mobility-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/v4ping-helper.h"
 #include "ns3/yans-wifi-helper.h"
 #include "ns3/applications-module.h"
  
 using namespace ns3;
  
 class AodvExample 
 {
 public:
   AodvExample ();
   bool Configure (int argc, char **argv);
   void Run ();
   void Report (std::ostream & os);
  
 private:
  
   // parameters
   uint32_t size;
   double step;
   double totalTime;
   bool pcap;
   bool printRoutes;
  
   // network
   NodeContainer nodes;
   NetDeviceContainer devices;
   Ipv4InterfaceContainer interfaces;

   // To calculate staticstic
   uint32_t packetsSent;
   uint32_t packetsReceived;
   uint64_t bytesSent;
   uint64_t bytesReceived;
  
 private:
   void CreateNodes ();
   void CreateDevices ();
   void InstallInternetStack ();
   void InstallApplications ();
   void SendPacket();
   void ReceivePacket();
 };
  
 int main (int argc, char **argv)
 {
   AodvExample test;
   if (!test.Configure (argc, argv))
     NS_FATAL_ERROR ("Configuration failed. Aborted.");
  
   test.Run ();
   test.Report (std::cout);
   return 0;
 }
  
 //-----------------------------------------------------------------------------
 AodvExample::AodvExample () :
   size (10),
   step (50),
   totalTime (100),
   pcap (true),
   packetsSent (0),
   packetsReceived (0),
   bytesSent (0),
   bytesReceived (0),
   printRoutes (true)
 {
 }
  
 bool
 AodvExample::Configure (int argc, char **argv)
 {
   // Enable AODV logs by default. Comment this if too noisy
   // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
  
   SeedManager::SetSeed (12345);
   CommandLine cmd (__FILE__);
  
   cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
   cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
   cmd.AddValue ("size", "Number of nodes.", size);
   cmd.AddValue ("time", "Simulation time, s.", totalTime);
   cmd.AddValue ("step", "Grid step, m", step);
  
   cmd.Parse (argc, argv);
   return true;
 }
  
 void
 AodvExample::Run ()
 {
 //  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
   CreateNodes ();
   CreateDevices ();
   InstallInternetStack ();
   InstallApplications ();
   std::cout << "Starting simulation for my version" << totalTime << " s ...\n";
   
   double throughput = static_cast<double>(bytesReceived) / totalTime;
   double packetDeliveryRatio = static_cast<double>(packetsReceived) / (packetsSent);
   double packetLossRate = 1.0 - packetDeliveryRatio;

   std::cout << "Throughput: " << throughput << " bytes/second\n";
   std::cout << "Packet Delivery Ratio: " << packetDeliveryRatio << "\n";
   std::cout << "Packet Loss Rate: " << packetLossRate << "\n";
  
   Simulator::Stop (Seconds (totalTime));
   Simulator::Run ();
   Simulator::Destroy ();
 }
  
 void
 AodvExample::Report (std::ostream &)
 { 
 }
  
 void
 AodvExample::CreateNodes ()
 {
   std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
   nodes.Create (size);
   // Name nodes
   for (uint32_t i = 0; i < size; ++i)
     {
       std::ostringstream os;
       os << "node-" << i;
       Names::Add (os.str (), nodes.Get (i));
     }
   // Create static grid
   MobilityHelper mobility;
   mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (step),
                                  "DeltaY", DoubleValue (0),
                                  "GridWidth", UintegerValue (size),
                                  "LayoutType", StringValue ("RowFirst"));
   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility.Install (nodes);
 }
  
 void
 AodvExample::CreateDevices ()
 {
   WifiMacHelper wifiMac;
   wifiMac.SetType ("ns3::AdhocWifiMac");
   YansWifiPhyHelper wifiPhy;
   YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
   wifiPhy.SetChannel (wifiChannel.Create ());
   WifiHelper wifi;
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
   devices = wifi.Install (wifiPhy, wifiMac, nodes); 
  
   if (pcap)
     {
       wifiPhy.EnablePcapAll (std::string ("aodv"));
     }
 }
  
 void
 AodvExample::InstallInternetStack ()
 {
   AodvHelper aodv;
   // you can configure AODV attributes here using aodv.Set(name, value)
   InternetStackHelper stack;
   stack.SetRoutingHelper (aodv); // has effect on the next Install ()
   stack.Install (nodes);
   Ipv4AddressHelper address;
   address.SetBase ("10.0.0.0", "255.0.0.0");
   interfaces = address.Assign (devices);
  
   if (printRoutes)
     {
       Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
       aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
     }
 }
  
 void
 AodvExample::InstallApplications ()
 {
   V4PingHelper ping (interfaces.GetAddress (size - 1));
   ping.SetAttribute ("Verbose", BooleanValue (true));
  
   ApplicationContainer p = ping.Install (nodes.Get (0));
   p.Start (Seconds (0));
   p.Stop (Seconds (totalTime) - Seconds (0.001));
  
   // move node away
   Ptr<Node> node = nodes.Get (size/2);
   Ptr<MobilityModel> mob = node->GetObject<MobilityModel> ();
   Simulator::Schedule (Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector (1e5, 1e5, 1e5));
 }
 
 void AodvExample::SendPacket()
 {
    // Create a packet and a packet sink on the destination node
    Ptr<Packet> packet = Create<Packet>(1024); // 1024 bytes packet size (you can change this)
    uint32_t destinationNodeIndex = 1; // Change this to the index of the destination node

    // Get the source and destination nodes
    Ptr<Node> sourceNode = nodes.Get(0);
    Ptr<Node> destinationNode = nodes.Get(destinationNodeIndex);

    // Create a packet source on the source node
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(destinationNodeIndex), 9));
    ApplicationContainer sinkApps = packetSinkHelper.Install(destinationNode);
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(totalTime));

    // Create a packet source on the source node
    UdpClientHelper udpClient(interfaces.GetAddress(destinationNodeIndex), 9);
    udpClient.SetAttribute("MaxPackets", UintegerValue(1)); // Send one packet
    udpClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); // Send the packet every 1 second

    ApplicationContainer sourceApps = udpClient.Install(sourceNode);
    sourceApps.Start(Seconds(1.0)); // Start sending after 1 second of simulation time
    sourceApps.Stop(Seconds(totalTime));
 }

 void AodvExample::ReceivePacket()
 {
      // Set up a packet sink on the destination node to receive packets
    uint32_t destinationNodeIndex = 1; // Change this to the index of the destination node
    Ptr<Node> destinationNode = nodes.Get(destinationNodeIndex);

    // Create a packet sink application to receive packets
    PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 9));
    ApplicationContainer sinkApps = packetSinkHelper.Install(destinationNode);
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(totalTime));
 }