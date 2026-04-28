/**
 * fanet-dos.cc
 *
 * NS-3 simulation of a Flying Ad-hoc Network (FANET) with DoS attack injection.
 *
 * Scenario:
 *   - N UAV nodes moving according to a random-waypoint mobility model
 *   - Normal UDP/ICMP traffic between nodes
 *   - An attacker node floods the network (UDP flood) to simulate a DoS attack
 *   - PCAP traces are written to dataset/raw-pcap/ for offline feature extraction
 *
 * Build & run (from the NS-3 root directory):
 *   cp <repo>/ns3/fanet-dos.cc scratch/
 *   ./ns3 run "scratch/fanet-dos --nNodes=10 --simTime=60 --attack=true"
 *
 * Outputs:
 *   fanet-dos-*.pcap  (one file per node interface)
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/aodv-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FanetDos");

// ---------------------------------------------------------------------------
// Command-line parameters
// ---------------------------------------------------------------------------
static uint32_t g_nNodes   = 10;
static double   g_simTime  = 60.0;   // seconds
static bool     g_attack   = true;
static uint32_t g_pcapNode = 0;      // node index to capture (-1 = all)

// ---------------------------------------------------------------------------
// Helper: install a constant-rate UDP application
// ---------------------------------------------------------------------------
static void
InstallUdpApp(Ptr<Node> src, Ptr<Node> dstNode, Ipv4Address dst, uint16_t port,
              double startTime, double stopTime,
              uint32_t packetSize = 512,
              std::string dataRate = "1Mbps")
{
    // Server on the destination node
    UdpServerHelper server(port);
    ApplicationContainer serverApps = server.Install(dstNode);
    serverApps.Start(Seconds(startTime));
    serverApps.Stop(Seconds(stopTime));

    // Client on the source node
    UdpClientHelper client(dst, port);
    client.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    client.SetAttribute("Interval",
                        TimeValue(Seconds(static_cast<double>(packetSize * 8) /
                                          DataRate(dataRate).GetBitRate())));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));

    ApplicationContainer apps = client.Install(src);
    apps.Start(Seconds(startTime));
    apps.Stop(Seconds(stopTime));
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    CommandLine cmd(__FILE__);
    cmd.AddValue("nNodes",   "Number of UAV nodes",           g_nNodes);
    cmd.AddValue("simTime",  "Simulation duration (seconds)", g_simTime);
    cmd.AddValue("attack",   "Enable DoS attack traffic",     g_attack);
    cmd.AddValue("pcapNode", "Node index to capture PCAP (-1 = all)", g_pcapNode);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("FANET-DoS simulation | nodes=" << g_nNodes
                << " simTime=" << g_simTime
                << " attack=" << g_attack);

    // ------------------------------------------------------------------
    // 1. Create nodes
    // ------------------------------------------------------------------
    NodeContainer uavNodes;
    uavNodes.Create(g_nNodes);

    // ------------------------------------------------------------------
    // 2. 802.11p (WAVE) wireless channel – commonly used for UAV ad-hoc
    // ------------------------------------------------------------------
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode",    StringValue("OfdmRate6Mbps"),
                                 "ControlMode", StringValue("OfdmRate6Mbps"));

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    channel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                               "Frequency", DoubleValue(5.9e9));

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    phy.Set("TxPowerStart", DoubleValue(20.0));
    phy.Set("TxPowerEnd",   DoubleValue(20.0));

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, uavNodes);

    // ------------------------------------------------------------------
    // 3. Mobility – Random Waypoint inside a 1 km × 1 km area
    // ------------------------------------------------------------------
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0|Max=1000]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0|Max=1000]"));

    mobility.SetMobilityModel(
        "ns3::RandomWaypointMobilityModel",
        "Speed",   StringValue("ns3::UniformRandomVariable[Min=5|Max=20]"),
        "Pause",   StringValue("ns3::ConstantRandomVariable[Constant=2]"),
        "PositionAllocator",
        PointerValue(CreateObject<RandomRectanglePositionAllocator>()));

    mobility.Install(uavNodes);

    // ------------------------------------------------------------------
    // 4. Internet stack with AODV routing
    // ------------------------------------------------------------------
    AodvHelper aodv;
    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(uavNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // ------------------------------------------------------------------
    // 5. Normal traffic: random pairs exchange UDP data
    // ------------------------------------------------------------------
    uint16_t basePort = 9000;
    Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable>();

    for (uint32_t i = 0; i < g_nNodes; ++i)
    {
        uint32_t dst = rng->GetInteger(0, g_nNodes - 1);
        if (dst == i) dst = (i + 1) % g_nNodes;

        InstallUdpApp(uavNodes.Get(i),
                      uavNodes.Get(dst),
                      interfaces.GetAddress(dst),
                      basePort + i,
                      1.0,
                      g_simTime - 1.0,
                      512,
                      "256Kbps");
    }

    // ------------------------------------------------------------------
    // 6. DoS attack: attacker node 0 floods node 1 at high rate
    // ------------------------------------------------------------------
    if (g_attack)
    {
        NS_LOG_INFO("Installing DoS attacker on node 0 targeting node 1");
        // Start attack halfway through the simulation
        double attackStart = g_simTime / 2.0;

        InstallUdpApp(uavNodes.Get(0),
                      uavNodes.Get(1),
                      interfaces.GetAddress(1),
                      8888,
                      attackStart,
                      g_simTime - 1.0,
                      1400,          // large packets
                      "10Mbps");     // flood rate
    }

    // ------------------------------------------------------------------
    // 7. PCAP capture
    // ------------------------------------------------------------------
    phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.EnablePcapAll("fanet-dos", false);

    // ------------------------------------------------------------------
    // 8. Flow monitor
    // ------------------------------------------------------------------
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    // ------------------------------------------------------------------
    // 9. Run
    // ------------------------------------------------------------------
    Simulator::Stop(Seconds(g_simTime));
    Simulator::Run();

    monitor->SerializeToXmlFile("fanet-dos-flowmon.xml", true, true);

    Simulator::Destroy();
    NS_LOG_INFO("Simulation finished.");
    return 0;
}
