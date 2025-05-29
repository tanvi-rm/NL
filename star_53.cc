/*-*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-*/ 
//Add required header files 
#include "ns3/core-module.h" 
#include "ns3/network-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/applications-module.h" 
#include "ns3/netanim-module.h" 
#include "ns3/point-to-point-layout-module.h" 
//Adding name declaration 
using namespace ns3; 

//Define log component where log msgs will be saved 
NS_LOG_COMPONENT_DEFINE("Star"); 
//Main Function 
int main (int argc, char *argv[]) 
{ 
//setting the default values of OnOff Application when app is in ON state 
Config::SetDefault("ns3::OnOffApplication::PacketSize",UintegerValue(137)); 
Config::SetDefault("ns3::OnOffApplication::DataRate",StringValue("14kb/s")); 
//specify no. of spoke nodes in topology 
uint32_t nSpokes = 8; 
//read and proceed the command line arguments 
CommandLine cmd(__FILE__); 
cmd.Parse(argc,argv); 
//configure p2p netdevices and channels to be installed on spoke nodes 
PointToPointHelper pointToPoint; 
pointToPoint.SetDeviceAttribute("DataRate", StringValue ("5Mbps")); 
pointToPoint.SetChannelAttribute("Delay", StringValue ("2ms")); 
PointToPointStarHelper star (nSpokes, pointToPoint); 
NS_LOG_INFO("Star topology created"); // display informational mgs

//install protocol stacks on nodes 
InternetStackHelper stack; 
star.InstallStack(stack); //install stack on all node in star topology 
NS_LOG_INFO("Installed Protocol stack on all nodes in topology"); 
//Assign IP Address to interfaces of Spoke nodes and hub 
star.AssignIpv4Addresses (Ipv4AddressHelper ("10.0.0.0", "255.0.0.0")); 
//configuring Packet Sink Application on Hub 
uint16_t port = 50000; //Specifying port no of hub 
//configuring socket address of hub 
Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny(), port)); 
PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",hubLocalAddress); 
//Install Packet Sink Application on Hub 
ApplicationContainer hubApp = packetSinkHelper.Install (star.GetHub ()); 
//configure start and stop time of packet sink application 
hubApp.Start (Seconds (1.0)); 
hubApp.Stop (Seconds (10.0)); 
// configure On-Off Application on spoke nodes 
OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());

onOffHelper.SetAttribute("OnTime", StringValue 
("ns3::ConstantRandomVariable[Constant=1]")); 
onOffHelper.SetAttribute("OffTime", StringValue 
("ns3::ConstantRandomVariable[Constant=0]")); 
// Install on off application of all spoke nodes  
// star.SpokeCount() return number of spoke nodes in star topology 
ApplicationContainer spokeApps; 
for(uint32_t i=0; i< star.SpokeCount(); ++i) 
{ 

AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port)); 
onOffHelper.SetAttribute ("Remote", remoteAddress); 
spokeApps.Add (onOffHelper.Install ( star.GetSpokeNode(i))); 
}
// configure start and stop time of ON OFF application 
spokeApps.Start (Seconds (1.0)); 
spokeApps.Stop (Seconds (10.0)); 
// enable routing so that packet will be routed across star topology 
Ipv4GlobalRoutingHelper::PopulateRoutingTables (); 
// enable packet capture on all nodes 
pointToPoint.EnablePcapAll("star"); 
// Animate the star topology 
AnimationInterface anim ("star_E.xml"); 
//set physical area in which nodes are placed. 
// sets hub node in middle

// spreads out spokes nodes evenly across hub 
star.BoundingBox(1,1,100,100); 
// run simulation 
Simulator::Run(); 
// Destroy the resources 
Simulator::Destroy(); 
NS_LOG_INFO("Done"); 
return 0; 
} 
