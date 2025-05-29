// Required headers 
#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h"

#include "ns3/applications-module.h" 
#include "ns3/netanim-module.h" 
#include "ns3/csma-module.h" 
#include "ns3/ipv4-global-routing-helper.h" 
#include "ns3/mobility-module.h" 
 
using namespace ns3; 
 
NS_LOG_COMPONENT_DEFINE ("Bus_topology"); 
 
int main(int argc, char *argv[]) 
{ 
    uint32_t extra_nodes = 3; 
 
    CommandLine cmd (__FILE__); 
    cmd.Parse (argc, argv); 
 
    Time::SetResolution (Time::NS); 
 
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO); 
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO); 
 
    // Create Point-to-Point Topology
    
      NodeContainer p2pNodes; 
    p2pNodes.Create(2); 
    
      PointToPointHelper pointToPoint; 
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps")); 
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms")); 
 
    NetDeviceContainer p2pDevices = pointToPoint.Install(p2pNodes); 
 
    // Create Bus Topology 
    NodeContainer bus_nodes; 
    bus_nodes.Add(p2pNodes.Get(1)); 
    bus_nodes.Create(extra_nodes); 
 
    CsmaHelper csma; 
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps")); 
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    
      NetDeviceContainer busDevices = csma.Install(bus_nodes); 
 
    // Install Protocol Stack 
    InternetStackHelper stack; 
    stack.Install(p2pNodes.Get(0)); 
    stack.Install(bus_nodes); 
 
    // Assign IP Addresses 
    Ipv4AddressHelper address; 
    address.SetBase("10.0.0.0", "255.0.0.0"); 
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);
    
       address.SetBase("20.0.0.0", "255.0.0.0"); 
    Ipv4InterfaceContainer bus_interfaces = address.Assign(busDevices); 
 
    // Configure Server Application 
    UdpEchoServerHelper echoServer(9); 
    ApplicationContainer serverApp = echoServer.Install(bus_nodes.Get(3)); 
    serverApp.Start(Seconds(1.0)); 
    serverApp.Stop(Seconds(10.0)); // FIXED
    
      // Configure Client Application 
    UdpEchoClientHelper echoClient(bus_interfaces.GetAddress(3), 9); 
    echoClient.SetAttribute("MaxPackets", UintegerValue(1)); 
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0))); 
    echoClient.SetAttribute("PacketSize", UintegerValue(1024)); 
 
    ApplicationContainer clientApp = echoClient.Install(p2pNodes.Get(0)); 
    clientApp.Start(Seconds(2.0)); 
    clientApp.Stop(Seconds(10.0)); 
 
    // Enable Routing 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables(); 
 
    // Capture Packets 
    csma.EnablePcap("bus_packet", busDevices.Get(1), true);
    
     // ADD MOBILITY MODEL TO AVOID ERRORS 
    MobilityHelper mobility; 
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel"); 
 
    mobility.Install(p2pNodes.Get(0)); 
    mobility.Install(p2pNodes.Get(1)); 
    mobility.Install(bus_nodes); 
 
    // Animation Interface 
    AnimationInterface anim("bus_E.xml"); 
 
    // Set positions of nodes in bus topology 
    anim.SetConstantPosition(p2pNodes.Get(0), 10.0, 15.0); 
    anim.SetConstantPosition(p2pNodes.Get(1), 30.0, 15.0); 
    anim.SetConstantPosition(bus_nodes.Get(0), 30.0, 15.0);  // p2pNodes(1) -> bus_nodes(0) 
    anim.SetConstantPosition(bus_nodes.Get(1), 40.0, 15.0);
    
        anim.SetConstantPosition(bus_nodes.Get(2), 50.0, 15.0); 
    anim.SetConstantPosition(bus_nodes.Get(3), 60.0, 15.0); 
 
    // Run Simulation 
    Simulator::Run(); 
    Simulator::Destroy(); 
 
    return 0; 
}  
