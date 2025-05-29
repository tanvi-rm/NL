/*-*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-*/ 
#include "ns3/core-module.h" 
#include "ns3/csma-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/applications-module.h"

#include "ns3/ipv4-global-routing-helper.h" 
#include "ns3/internet-apps-module.h" 
 
using namespace ns3; 
NS_LOG_COMPONENT_DEFINE ("DhcpExample"); 
 
int main(int argc, char *argv[]) 
{ 
    // read and parse command line argument 
    CommandLine cmd (__FILE__); 
    cmd.Parse (argc, argv); 
 
    // set time resolution 
    Time::SetResolution (Time::NS); 
     
    LogComponentEnable("DhcpServer", LOG_LEVEL_ALL); 
    LogComponentEnable("DhcpClient", LOG_LEVEL_ALL); 
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); 
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); 
     
    // create nodes 
    NS_LOG_INFO ("Create nodes."); 
    NodeContainer nodes; // represnts client nodes 
    NodeContainer router; // represent roters R0, R1 
    nodes.Create(3); 
    router.Create(2);
    
    // combine routers and nodes into single object 
    NodeContainer bus_nodes (nodes, router); 
     
    //Configuring the csma net devices and csma channel 
    CsmaHelper csma; 
    csma.SetChannelAttribute("DataRate", StringValue("5Mbps")); 
    csma.SetChannelAttribute("Delay", StringValue("2ms")); 
    csma.SetDeviceAttribute("Mtu", UintegerValue (1500)); 
     
    // Install configured net devices and channel on nodes 
    NetDeviceContainer busDevices = csma.Install(bus_nodes); 
     
    // Create a point-to-point topology 
    // Create new node A 
    // Add node R1 to point to point topology 
     
    NodeContainer p2pNodes; 
    p2pNodes.Add (bus_nodes.Get(4)); // add node R1 to p2p 
    p2pNodes.Create(1); // create nwe node A 
     
    // configure and install p2p net devices on R1 and A 
    // connect them with p2p channel 
    PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue ("5Mbps")); 
    pointToPoint.SetChannelAttribute("Delay", StringValue ("2ms"));
    
       // install net devices on R1 and A 
    NetDeviceContainer p2pDevices; 
    p2pDevices = pointToPoint.Install (p2pNodes); 
     
    // install protocol stock 
    InternetStackHelper stack; 
    stack.Install(nodes); // Install on N0 N1 and N2 
    stack.Install(router); // install on R0 and R1 
    stack.Install (p2pNodes.Get(1)); // install on node A 
     
    // Configure an assign ip addresses to p2p interfaces 
    Ipv4AddressHelper address; 
    address.SetBase("20.0.0.0", "255.0.0.0"); 
     
    Ipv4InterfaceContainer p2pInterfaces; 
    p2pInterfaces = address.Assign(p2pDevices); 
     
    // Assign fixed IP address to default router R1 
    DhcpHelper dhcpHelper; 
    Ipv4InterfaceContainer fixedNodes = dhcpHelper.InstallFixedAddress(busDevices.Get(4), 
Ipv4Address("10.0.0.17"), Ipv4Mask("/8")); 
     
    // enable forwarding of packets from R1 
    fixedNodes.Get(0).first->SetAttribute("IpForward", BooleanValue(true)); 
    // enable routing between 2 networks 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
     // configure and install dhcp server on R0 
    ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer(busDevices.Get(3), 
Ipv4Address("10.0.0.12"), Ipv4Address("10.0.0.0"), Ipv4Mask("/8"), Ipv4Address("10.0.0.1"), 
Ipv4Address ("10.0.0.15"), Ipv4Address("10.0.0.17")); 
     
    // configure and start and stop time of server 
    dhcpServerApp.Start (Seconds(0.0)); 
    dhcpServerApp.Stop(Seconds(20.0)); 
     
    // configure DHCP clients 
    // Combining net devices on nodes N0, N1, N2 in single net device object 
    NetDeviceContainer dhcpClientNetDevs; 
    dhcpClientNetDevs.Add(busDevices.Get(0)); 
    dhcpClientNetDevs.Add(busDevices.Get(1)); 
    dhcpClientNetDevs.Add(busDevices.Get(2)); 
     
    // install DHCP clients on N0, N1 and N2 
    ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient(dhcpClientNetDevs); 
     
    // configure start and stop time of dhcpclient 
    dhcpClients.Start(Seconds(1.0)); 
    dhcpClients.Stop(Seconds (20.0)); 
     
    //Udp server applciation and udp client application 
    // node n1 udp client application 
    // node r1 udp server application
    
     //configure and install UdpEchoServer App on node A 
    UdpEchoServerHelper echoServer(9); 
    ApplicationContainer serverApp = echoServer.Install(p2pNodes.Get(1)); 
    serverApp.Start(Seconds(1.0)); 
    serverApp.Stop(Seconds(10.0)); 
     
    // Configure and install UdpEchoClientApp on node N1 
    UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9); 
    echoClient.SetAttribute("MaxPackets", UintegerValue(1)); 
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    echoClient.SetAttribute("PacketSize", UintegerValue(1024)); 
 
    ApplicationContainer clientApp = echoClient.Install(nodes.Get(1)); 
    clientApp.Start(Seconds(2.0)); 
    clientApp.Stop(Seconds(10.0)); 
  
    // run simulation 
    // destroy resources 
    NS_LOG_INFO("Run Simulation."); 
    Simulator::Run(); 
    Simulator::Destroy(); 
    NS_LOG_INFO("Done."); 
    return 0; 
} 
  
