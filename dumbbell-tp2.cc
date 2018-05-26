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
    
    #include <iostream>
    
    #include "ns3/core-module.h"
    #include "ns3/network-module.h"
    #include "ns3/internet-module.h"
    #include "ns3/point-to-point-module.h"
    #include "ns3/netanim-module.h"
    #include "ns3/applications-module.h"
    #include "ns3/point-to-point-layout-module.h"
    #include "ns3/flow-monitor-module.h"
    
    using namespace ns3;
    
    int main (int argc, char *argv[])
    {
      Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (512));
      Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("500kb/s"));
    
      uint32_t    nLeftLeaf = 3;
      uint32_t    nRightLeaf = 3;
      uint32_t    nLeaf = 0; // If non-zero, number of both left and right
      std::string animFile = "dumbbell-animation-tp2.xml" ;  // Name of file for animation output
    
      CommandLine cmd;
      cmd.AddValue ("nLeftLeaf", "Number of left side leaf nodes", nLeftLeaf);
      cmd.AddValue ("nRightLeaf","Number of right side leaf nodes", nRightLeaf);
      cmd.AddValue ("nLeaf",     "Number of left and right side leaf nodes", nLeaf);
      cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
    
      cmd.Parse (argc,argv);
      if (nLeaf > 0)
        {
          nLeftLeaf = nLeaf;
          nRightLeaf = nLeaf;
        }
    
      // Create the point-to-point link helpers
      PointToPointHelper pointToPointRouter;
      pointToPointRouter.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
      pointToPointRouter.SetChannelAttribute ("Delay", StringValue ("1ms"));
      PointToPointHelper pointToPointLeaf;
      pointToPointLeaf.SetDeviceAttribute    ("DataRate", StringValue ("10Mbps"));
      pointToPointLeaf.SetChannelAttribute   ("Delay", StringValue ("1ms"));
    
      PointToPointDumbbellHelper d (nLeftLeaf, pointToPointLeaf,
                                    nRightLeaf, pointToPointLeaf,
                                    pointToPointRouter);
    
      // Install Stack
      InternetStackHelper stack;
      d.InstallStack (stack);
    
      // Assign IP Addresses
      d.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                             Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                             Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));
    
      // Install on/off app on all right side nodes
      OnOffHelper clientHelper ("ns3::UdpSocketFactory", Address ());
      clientHelper.SetAttribute ("OnTime", StringValue ("ns3::UniformRandomVariable"));
      clientHelper.SetAttribute ("OffTime", StringValue ("ns3::UniformRandomVariable"));
      ApplicationContainer clientApps;
    
      for (uint32_t i = 0; i < d.RightCount (); ++i)
        {
          // Create an on/off app sending packets to the same leaf right side
          AddressValue remoteAddress (InetSocketAddress (d.GetLeftIpv4Address (i), 1000));
          clientHelper.SetAttribute ("Remote", remoteAddress);
          clientApps.Add (clientHelper.Install (d.GetRight (i)));
        }
    
      clientApps.Start (Seconds (0.0));
      clientApps.Stop (Seconds (10.0));
    
      // Set the bounding box for animation
      d.BoundingBox (1, 1, 100, 100);
    
      // Create the animation object and configure for specified output
      AnimationInterface anim (animFile);
      anim.EnablePacketMetadata (); // Optional
      anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
      
     // Set up the acutal simulation
     Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   
     // Flow monitor
     Ptr<FlowMonitor> flowMonitor;
     FlowMonitorHelper flowHelper;
     flowMonitor = flowHelper.InstallAll(); 


     Simulator::Run ();
     std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
     Simulator::Destroy ();

     flowMonitor->SerializeToXmlFile("monitor_dumbbell-tp2.xml", true, true);  
   return 0;
 }
