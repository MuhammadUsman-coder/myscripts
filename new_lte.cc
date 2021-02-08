#include <iostream>
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include <ns3/buildings-module.h>
#include "ns3/netanim-module.h"
#include "ns3/log.h"
#Declared Libraries 

using namespace ns3;

#Declaring pointer to store Animation interface object
AnimationInterface * pAnim = 0;

#Starting of main function
int main (int argc, char *argv[])
{

#Assigning number of nodes to variables and simulation time  
uint16_t numberOfNodesENB = 4;
uint16_t numberOfNodesEU = 8;
double simTime = 0.05;

#Assigning name of xml file for Netanim in variable
std::string animFile = "usman.xml";

CommandLine cmd;
cmd.Parse (argc, argv);

//creation of the lteHelper object which is a software library that allows the simulation of LTE networks, optionally including the EPC
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

//creation of the epc helper object ,Set the EpcHelper to be used to setup the EPC network in conjunction with the setup of the LTE radio access network
Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();

lteHelper->SetEpcHelper (epcHelper);

Ptr<Node> pgw = epcHelper->GetPgwNode (); 

// creation  RemoteHost .
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);

  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

// Template in c++ object creation
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator>();

  positionAlloc1->Add(Vector(500.0, -100.0, 20.0));

  MobilityHelper mobility1;
  mobility1.SetPositionAllocator (positionAlloc1);
  mobility1.Install (remoteHostContainer);

// Create the Internet

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("150Mb/s")));

  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

//creation of nodes for eNB and UE
NodeContainer enbNodes;
enbNodes.Create (numberOfNodesENB);
NodeContainer ueNodes;
ueNodes.Create (numberOfNodesEU);

//to configure the position and movement of the nodes
MobilityHelper mobility;
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                               "MinX", DoubleValue (-350.0),
                               "MinY", DoubleValue (-200.0),
                               "DeltaX", DoubleValue (0.0),
                               "DeltaY", DoubleValue (150.0),
                               "GridWidth", UintegerValue (1),
                               "LayoutType", StringValue ("RowFirst"));

mobility.Install (enbNodes);
BuildingsHelper::Install (enbNodes);

for (uint16_t i = 0; i < numberOfNodesEU; i++)
{
mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                               "MinX", DoubleValue (-900.0),
                               "MinY", DoubleValue (-250.0),
                               "DeltaX", DoubleValue (120.0),
                               "DeltaY", DoubleValue (150.0),
                               "GridWidth", UintegerValue (4),
                               "LayoutType", StringValue ("RowFirst"));

mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                           "Mode", StringValue ("Time"),
                           "Time", StringValue ("0.5s"),
                           "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=50.0]"),
                           "Bounds", RectangleValue (Rectangle (-12000.0, 12000.0, -12000.0, 12000.0)));
}
mobility.Install (ueNodes);

//to install the lte protocol for enbNodes and ueNodes.
NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);
NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);


// Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }


//to attach ueDevs with enbDevs
uint16_t j = 0;
for (uint16_t i = 0; i < numberOfNodesEU; i++)
  {  
     if (j < numberOfNodesENB)
        {
          lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
          j++;
        }
      else
        {
          j = 0;
          lteHelper->Attach (ueDevs.Get(i), enbDevs.Get(j));
        }   
  }


//to activate the radio medium which carries the data between ueDevs and enbDevs
uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
     
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
 
      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }
    }

// Install and start applications on UEs and remote host
  serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.01));

Simulator::Stop (Seconds (simTime));

// Create the animation object and configure for specified output
  
pAnim  = new AnimationInterface (animFile.c_str ());

#Running and destroying simulation 
Simulator::Run ();
Simulator::Destroy ();

#Deleting Pointer for anim interface object  
delete pAnim;
return 0;
}
