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
#include "ns3/config-store.h"
#include <ns3/buildings-module.h>
#include "ns3/netanim-module.h"

#include "ns3/flow-monitor-module.h"
#include "ns3/log.h"
#include <sys/timeb.h>
#include <ns3/internet-trace-helper.h>
#include <ns3/spectrum-module.h>
#include <ns3/log.h>
#include <ns3/string.h>
#include <fstream>

using namespace ns3;

void
PrintGnuplottableEnbListToFile (std::string filename)
{
  
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
         
        }
    }
}

AnimationInterface * pAnim = 0;




void modify ()
{

  // Every update change the node description for node 2
  std::ostringstream node0Oss;
  node0Oss << "-----Node:" << Simulator::Now ().GetSeconds ();
  pAnim->UpdateNodeDescription (2, node0Oss.str ());



  if (Simulator::Now ().GetSeconds () < 10) // This is important or the simulation
    // will run endlessly
    Simulator::Schedule (Seconds (0.05), modify);

}

int main (int argc, char *argv[])
{

Time::SetResolution (Time::NS);
LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

uint16_t numberOfNodesENB = 4;
uint16_t numberOfNodesEU = 8;
double simTime = 0.05;
double distance = 250.0;
double interPacketInterval = 150.0;

std::string animFile = "usman.xml";

CommandLine cmd;
cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodesENB);
cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
cmd.AddValue("distance", "Distance between eNBs [m]", distance);
cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
cmd.AddValue ("animFile",  "File Name for Animation Output", animFile);
cmd.Parse (argc, argv);
ConfigStore inputConfig;
inputConfig.ConfigureDefaults ();
cmd.Parse (argc, argv);


//creation de l'objet lteHelper.
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100));
lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (18100));

//creation de l'objet epcHelper.
Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);

Ptr<Node> pgw = epcHelper->GetPgwNode (); 

// creation  RemoteHost .
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator>();
  positionAlloc1->Add(Vector(500.0, -100.0, 20.0));
  MobilityHelper mobility1;
  mobility1.SetPositionAllocator (positionAlloc1);
  mobility1.Install (remoteHostContainer);
// Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("150Mb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
//creation des noeuds pour eNB et UE
NodeContainer enbNodes;
enbNodes.Create (numberOfNodesENB);
NodeContainer ueNodes;
ueNodes.Create (numberOfNodesEU);

//pour configurer la position et le mouvement des neouds



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
AsciiTraceHelper ascii;
MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("mobility-trace-example.mob"));

//pour installer le protocol lte pour enbNodes et ueNodes.
NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);
NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);
//BuildingsHelper::MakeMobilityModelConsistent ();
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

//pour attacher ueDevs avec enbDevs
//lteHelper->Attach (ueDevs, enbDevs.Get (0));
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
// Attach all UEs to the closest eNodeB
//lteHelper->AttachToClosestEnb (ueDevs, enbDevs);
// Add X2 inteface
  //lteHelper->AddX2Interface (enbNodes);
// X2-based Handover
 // lteHelper->HandoverRequest (Seconds (0.100), ueDevs.Get (0), enbDevs.Get (0), enbDevs.Get (1));


//pour activer le support radio qui porte les données entre ueDevs et enbDevs
Ptr<EpcTft> tft = Create<EpcTft> ();
EpcTft::PacketFilter pf;
pf.localPortStart = 1234;
pf.localPortEnd = 1234;
tft->Add (pf);
lteHelper->ActivateDedicatedEpsBearer (ueDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), tft);

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
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(1000000));

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
 // lteHelper->EnableTraces ();

Simulator::Stop (Seconds (simTime));


lteHelper->EnablePhyTraces ();
lteHelper->EnableMacTraces ();
lteHelper->EnableRlcTraces ();

// Create the animation object and configure for specified output
  pAnim = new AnimationInterface (animFile.c_str ());

  pAnim->SetBackgroundImage ("/home/usman/ns-allinone-3.33/ns-3.33/scratch/Phone.png", -1, 1, 0.5, 0.5, 1);

  Simulator::Schedule (Seconds (simTime), modify);


Simulator::Run ();

Simulator::Destroy ();

delete pAnim;

return 0;

}
