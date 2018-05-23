#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (3);
  NodeContainer n1n2 = NodeContainer (nodes.Get(0),nodes.Get(2));
  NodeContainer n2n3 = NodeContainer (nodes.Get(2),nodes.Get(1)); 

  InternetStackHelper stack;
  stack.Install (nodes);  
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer devices1 = pointToPoint.Install(n1n2);
    
  NetDeviceContainer devices2 = pointToPoint.Install(n2n3);
     
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (devices1);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  //ipv4.Assign (devices2);
  Ipv4InterfaceContainer Ipv4IFC = ipv4.Assign (devices2); 
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (Ipv4IFC.GetAddress (1), port));
  onoff.SetConstantRate (DataRate ("300bps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (50));
  
  ApplicationContainer apps = onoff.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));  

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
