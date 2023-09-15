#include "ns3/core-module.h"
#include "ns3/network-module.h"
 

using namespace ns3;

int main (int argc, char *argv[])
{
NodeContainer nodes;
nodes.Create(10);

//Get pointer to the first node
Ptr<Node> n0 = nodes.Get (0);

//Iterate over a Nodecontainer
for (uint32_t i=0 ; i<nodes.GetN() ; i++)
{

	Ptr<Node> ni = nodes.Get(i);
	// Print some information about the nodes
        std::cout << "Node " << ni->GetId() << " is in the NodeContainer." << std::endl;
}

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
