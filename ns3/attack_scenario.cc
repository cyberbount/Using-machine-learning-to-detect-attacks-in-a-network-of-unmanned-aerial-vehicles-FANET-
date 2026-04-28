/**
 * attack_scenario.cc
 *
 * Extended NS-3 attack scenario for the FANET DoS detection project.
 *
 * Implements three attack phases:
 *   Phase 1 (0–20 s)  : Normal traffic only (baseline)
 *   Phase 2 (20–40 s) : UDP flood DoS attack by one compromised node
 *   Phase 3 (40–60 s) : Mixed traffic (partial attack + normal nodes)
 *
 * PCAP files are written per-node so the feature extraction script can
 * label packets based on the known attack time-window.
 *
 * Usage:
 *   cp attack_scenario.cc <ns3-root>/scratch/
 *   ./ns3 run "scratch/attack_scenario --nNodes=12 --simTime=60"
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AttackScenario");

// ---------------------------------------------------------------------------
static void
InstallCbr(Ptr<Node> src, Ipv4Address dstAddr,
           uint16_t port, double start, double stop,
           uint32_t pktSize, std::string rate)
{
    UdpClientHelper client(dstAddr, port);
    client.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client.SetAttribute("Interval",
                        TimeValue(Seconds(static_cast<double>(pktSize * 8) /
                                          DataRate(rate).GetBitRate())));
    client.SetAttribute("PacketSize", UintegerValue(pktSize));
    ApplicationContainer a = client.Install(src);
    a.Start(Seconds(start));
    a.Stop(Seconds(stop));
}

// ---------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    uint32_t nNodes  = 12;
    double   simTime = 60.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes",  "Number of UAV nodes (>=3)", nNodes);
    cmd.AddValue("simTime", "Total simulation time (s)", simTime);
    cmd.Parse(argc, argv);

    if (nNodes < 3)
    {
        NS_FATAL_ERROR("Need at least 3 nodes.");
    }

    double phase2Start = simTime / 3.0;
    double phase3Start = 2.0 * simTime / 3.0;

    NS_LOG_INFO("Attack scenario | nodes=" << nNodes
                << "  phase2=" << phase2Start
                << "  phase3=" << phase3Start);

    // ------------------------------------------------------------------
    // Nodes & wireless
    // ------------------------------------------------------------------
    NodeContainer nodes;
    nodes.Create(nNodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",    StringValue("OfdmRate6Mbps"),
                                 "ControlMode", StringValue("OfdmRate6Mbps"));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                               "Frequency", DoubleValue(5.9e9));

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // ------------------------------------------------------------------
    // Mobility
    // ------------------------------------------------------------------
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=800]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=800]"));
    mobility.SetMobilityModel(
        "ns3::RandomWaypointMobilityModel",
        "Speed",  StringValue("ns3::UniformRandomVariable[Min=5|Max=15]"),
        "Pause",  StringValue("ns3::ConstantRandomVariable[Constant=1]"),
        "PositionAllocator",
        PointerValue(CreateObject<RandomRectanglePositionAllocator>()));
    mobility.Install(nodes);

    // ------------------------------------------------------------------
    // Internet + AODV
    // ------------------------------------------------------------------
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    Ipv4AddressHelper addr;
    addr.SetBase("10.3.0.0", "255.255.0.0");
    Ipv4InterfaceContainer ifaces = addr.Assign(devices);

    // ------------------------------------------------------------------
    // Phase 1 & 3 – normal CBR traffic between random pairs
    // ------------------------------------------------------------------
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();
    for (uint32_t i = 0; i < nNodes; ++i)
    {
        uint32_t j = (i + 1 + rng->GetInteger(0, nNodes - 2)) % nNodes;

        // Phase 1: normal
        InstallCbr(nodes.Get(i), ifaces.GetAddress(j),
                   9000 + i, 1.0, phase2Start - 0.1, 512, "128Kbps");

        // Phase 3: residual normal
        InstallCbr(nodes.Get(i), ifaces.GetAddress(j),
                   9100 + i, phase3Start, simTime - 1.0, 512, "128Kbps");
    }

    // ------------------------------------------------------------------
    // Phase 2 & 3 – DoS flood from node 0 → node 1
    // ------------------------------------------------------------------
    // Flood starts at phase2Start and continues to end of simulation
    InstallCbr(nodes.Get(0), ifaces.GetAddress(1),
               8888, phase2Start, simTime - 1.0, 1400, "20Mbps");

    // ------------------------------------------------------------------
    // PCAP
    // ------------------------------------------------------------------
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcapAll("attack-scenario", false);

    // ------------------------------------------------------------------
    // Flow monitor
    // ------------------------------------------------------------------
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    monitor->SerializeToXmlFile("attack-scenario-flowmon.xml", true, true);

    Simulator::Destroy();
    NS_LOG_INFO("Attack scenario simulation finished.");
    return 0;
}
