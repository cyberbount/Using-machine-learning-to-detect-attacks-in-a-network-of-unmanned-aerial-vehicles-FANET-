/**
 * topology.cc
 *
 * Stand-alone NS-3 script that creates a static FANET topology and
 * prints node positions, routing tables, and link statistics.
 *
 * Usage (from NS-3 root):
 *   cp <repo>/ns3/topology.cc scratch/
 *   ./ns3 run "scratch/topology --nNodes=6"
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FanetTopology");

int
main(int argc, char *argv[])
{
    uint32_t nNodes  = 6;
    double   simTime = 20.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes",  "Number of UAV nodes",  nNodes);
    cmd.AddValue("simTime", "Simulation time (s)",  simTime);
    cmd.Parse(argc, argv);

    // ---------------------------------------------------------------
    // Nodes
    // ---------------------------------------------------------------
    NodeContainer nodes;
    nodes.Create(nNodes);

    // ---------------------------------------------------------------
    // Wi-Fi (ad-hoc)
    // ---------------------------------------------------------------
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",    StringValue("OfdmRate6Mbps"),
                                 "ControlMode", StringValue("OfdmRate6Mbps"));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // ---------------------------------------------------------------
    // Static grid layout: nodes placed in a circle
    // ---------------------------------------------------------------
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> posAlloc =
        CreateObject<ListPositionAllocator>();

    double radius = 200.0; // metres
    for (uint32_t i = 0; i < nNodes; ++i)
    {
        double angle = 2.0 * M_PI * i / nNodes;
        posAlloc->Add(Vector(radius * std::cos(angle),
                             radius * std::sin(angle),
                             50.0)); // altitude 50 m
    }

    mobility.SetPositionAllocator(posAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // ---------------------------------------------------------------
    // Internet stack
    // ---------------------------------------------------------------
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.2.0.0", "255.255.0.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // ---------------------------------------------------------------
    // Simple ping between node 0 and node nNodes-1
    // ---------------------------------------------------------------
    uint16_t echoPort = 7;
    UdpEchoServerHelper echoServer(echoPort);
    ApplicationContainer serverApps =
        echoServer.Install(nodes.Get(nNodes - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(simTime));

    UdpEchoClientHelper echoClient(interfaces.GetAddress(nNodes - 1), echoPort);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval",   TimeValue(Seconds(0.1)));
    echoClient.SetAttribute("PacketSize", UintegerValue(256));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(simTime));

    // ---------------------------------------------------------------
    // Print node positions
    // ---------------------------------------------------------------
    for (uint32_t i = 0; i < nNodes; ++i)
    {
        Ptr<MobilityModel> mob = nodes.Get(i)->GetObject<MobilityModel>();
        Vector pos = mob->GetPosition();
        NS_LOG_UNCOND("Node " << i << " IP=" << interfaces.GetAddress(i)
                      << " pos=(" << pos.x << ", " << pos.y
                      << ", " << pos.z << ")");
    }

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Topology simulation complete.");
    return 0;
}
