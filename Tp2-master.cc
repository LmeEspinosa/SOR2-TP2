/* Dumbbel Topology with 3 nodes has left side, in the middle 2 routers
 * as bridge and 3 nodes in the right side. 2 of the 6 nodes with UDP protocol
 * and the others 4 with TCP protocol
 * 
 *		 _								  _
		|	 N1------+		 +------N4 	   |
		|	   		 |		 |			   |
Senders	|UDP N2------R1------R2-----N5 UDP |	Receivers
		|			 |		 |			   |
		|_	 N3------+		 +------N6	 _ |
 * 
 * 
 *  
 */ 
 
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");

int
main (int argc, char *argv[])
{
   LogComponentEnable ("UdpEchoClientApplication",LOG_LEVEL_INFO);
   LogComponentEnable ("UdpEchoServerApplication",LOG_LEVEL_INFO);	

  bool tracing = false;
  uint32_t maxBytes = 0;
  
  CommandLine cmd;
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("maxBytes",
                "Total number of bytes for application to send", maxBytes);
  cmd.Parse (argc, argv);
  
  NS_LOG_INFO ("Create nodes.");
  NodeContainer senders,receivers,IzqRouter, DerRouter, routers;

  senders.Create(3);
  receivers.Create(3);
  routers.Create(2);
    
  PointToPointHelper p2pLow;
  p2pLow.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  p2pLow.SetChannelAttribute ("Delay", StringValue ("5ms"));
    
  PointToPointHelper p2pHigh;
  p2pHigh.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2pHigh.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  NS_LOG_INFO ("Create channels.");
  
  NetDeviceContainer senderDevices,Sd_1,Sd_2,Sd_3;
    
  NetDeviceContainer receiverDevices,Rd_1,Rd_2,Rd_3;

  NetDeviceContainer routerDevices = p2pLow.Install (routers);
    
  Sd_1 = p2pHigh.Install (routers.Get(0),senders.Get(0));
  Sd_2 = p2pHigh.Install (routers.Get(0),senders.Get(1));
  Sd_3 = p2pHigh.Install (routers.Get(0),senders.Get(2));
  
  senderDevices.Add(Sd_1);
  senderDevices.Add(Sd_2);
  senderDevices.Add(Sd_3);
  
  Rd_1 = p2pHigh.Install (routers.Get(1),receivers.Get(0));
  Rd_2 = p2pHigh.Install (routers.Get(1),receivers.Get(1));
  Rd_3 = p2pHigh.Install (routers.Get(1),receivers.Get(2));
  
  receiverDevices.Add(Rd_1);
  receiverDevices.Add(Rd_2);
  receiverDevices.Add(Rd_3);  
  
  InternetStackHelper stack;
  stack.Install(senders);
  stack.Install(routers);
  stack.Install(receivers);  
  
  Ipv4AddressHelper addressSenders;
  addressSenders.SetBase   ("10.1.1.0", "255.255.255.0");
  addressSenders.Assign (senderDevices);
  
  Ipv4AddressHelper addressRouters;
  addressRouters.SetBase   ("10.1.2.0", "255.255.255.0");
  addressRouters.Assign (routerDevices);
  
  Ipv4AddressHelper addressReceivers;
  addressReceivers.SetBase ("10.1.3.0", "255.255.255.0");
  addressReceivers.Assign (receiverDevices);  

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  
      
  Ipv4InterfaceContainer interfaces = addressReceivers.Assign(receiverDevices);
  
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (interfaces.GetAddress (1), port));
  onoff.SetConstantRate (DataRate ("300bps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (50));
  
  ApplicationContainer apps = onoff.Install (senders.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));    
  
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));  
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO ("Done.");
  
  return 0; 
}
