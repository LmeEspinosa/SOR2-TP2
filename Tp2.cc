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
  
/*  
  NodeContainer sender_1,sender_2,sender_3,receiver_1,receiver_2,receiver_3,senders,routers,receivers;

  routers = NodeContainer(nodes.Get(3),nodes.Get(4));
  
  sender_1 = NodeContainer(nodes.Get(0),nodes.Get(3)); 
  sender_2 = NodeContainer(nodes.Get(1),nodes.Get(3));
  sender_3 = NodeContainer(nodes.Get(2),nodes.Get(3));
  
  receiver_1 = NodeContainer(nodes.Get(4),nodes.Get(5));
  receiver_2 = NodeContainer(nodes.Get(4),nodes.Get(6));
  receiver_3 = NodeContainer(nodes.Get(4),nodes.Get(7));
  
  senders.Add(sender_1);
  senders.Add(sender_2);
  senders.Add(sender_3);
  
  receivers.Add(receiver_1);
  receivers.Add(receiver_2);
  receivers.Add(receiver_3);
  
*/
  
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
  
  Ipv4AddressHelper addressRouters;
  addressRouters.SetBase   ("10.1.2.0", "255.255.255.0");
  
  Ipv4AddressHelper addressReceivers;
  addressReceivers.SetBase ("10.1.3.0", "255.255.255.0");

  
  Ipv4InterfaceContainer iSe,iRo,iRe,interfaces;
   
  iSe   = addressSenders.Assign (senderDevices);
  iRo   = addressRouters.Assign (routerDevices);
  iRe   = addressReceivers.Assign (receiverDevices);
  
  interfaces.Add(iSe);
  interfaces.Add(iRo);
  interfaces.Add(iRe);
    	    	    
  uint16_t port = 8;  // well-known echo port number
  
  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (interfaces.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps = source.Install (senders.Get (0));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));
  
  PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (interfaces.GetAddress (4), port)); //Ipv4Address::GetAny ()
  for(uint i = 0; i < interfaces.GetN(); ++i) 
  {
     std::cout << "interface IPs: " << interfaces.GetAddress(i) << std::endl;
  }
  ApplicationContainer sinkApps = sink.Install (receivers.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));
  
  UdpEchoServerHelper echoServer (9);
  
  ApplicationContainer serverApps = echoServer.Install (receivers.Get (0));
  std::cout << "Receiver IPs: " << receivers.Get(0) << std::endl;
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  UdpEchoClientHelper echoClient(interfaces.GetAddress(0),9);
  
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps = echoClient.Install (senders.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  
    /*generamos trazabilidad*/
  AsciiTraceHelper ascii; 
  p2pLow.EnableAsciiAll (ascii.CreateFileStream ("Trace/Tp2_P2PLow.tr"));
  p2pHigh.EnableAsciiAll (ascii.CreateFileStream ("Trace/Tp2_P2PHigh.tr"));
  /*trazabilidad Pcap para abrir con WhireShark*/
  p2pLow.EnablePcapAll ("Pcaps/Tp2_P2PLow");
  p2pHigh.EnablePcapAll ("Pcaps/Tp2_P2PHigh");
  
  

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));  
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO ("Done.");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
  
  return 0;
 
}
