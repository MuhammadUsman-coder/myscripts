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

AnimationInterface * pAnim = 0;

int main (int argc, char *argv[])
{
uint16_t numberOfNodesENB = 4;
uint16_t numberOfNodesEU = 8;
double simTime = 0.05;
double distance = 250.0;
double interPacketInterval = 150.0;

std::string animFile = "usman.xml";

CommandLine cmd;
cmd.Parse (argc, argv);
ConfigStore inputConfig;
inputConfig.ConfigureDefaults ();
cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

//creation of nodes for eNB and UE
NodeContainer enbNodes;
enbNodes.Create (numberOfNodesENB);
NodeContainer ueNodes;
ueNodes.Create (numberOfNodesEU);

// creation  RemoteHost .
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);

  // Create the Internet

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("150Mb/s")));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));


  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

//to install the lte protocol for enbNodes and ueNodes.
NetDeviceContainer enbDevs;
enbDevs = lteHelper->InstallEnbDevice (enbNodes);
NetDeviceContainer ueDevs;
ueDevs = lteHelper->InstallUeDevice (ueNodes);

// Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  InternetStackHelper stack;
    stack.Install (remoteHostContainer);


  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");

    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

     ApplicationContainer clientApps;
  ApplicationContainer serverApps;

    serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.01));





