/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pavel Boyko <boyko@iitp.ru>
 *
 * Classical hidden terminal problem and its RTS/CTS solution.
 *
 * Topology: [node 1] <-- -50 dB --> [node 0] <-- -50 dB --> [node 2]
 *
 * This example illustrates the use of
 *  - Wifi in ad-hoc mode
 *  - Matrix propagation loss model
 *  - Use of OnOffApplication to generate CBR stream
 *  - IP flow monitor
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/on-off-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"

using namespace ns3;

/// Run single 10 seconds experiment
void experiment (bool enableCtsRts, std::string wifiManager)
{
  
  
  // Enable or disable CTS/RTS based on argument enableCtsRts
  //ctsThr is the frame size over which RTS/CTS will be applied
  // It is set to a low value of enableRtsCts is true (so that)
  // it's mostly applied, and set to a high value when enableRtsCts is fales
  //so that it's mostly applied applied
  
  //This statement sets the threshold variable
  UintegerValue ctsThr = (enableCtsRts ? UintegerValue (100) : UintegerValue (10000));
  //This statement passes the threshold variable to the configuration method
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

  // Declare a NodeContainer variable called "nodes"
  NodeContainer nodes;
  
  // Call the create method of that object, asking it to create given number of nodes
  nodes.Create (3);

  // Place nodes somehow, this is required by every wireless simulation
  for (uint8_t i = 0; i < 3; ++i)
    {
      nodes.Get (i)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
    }

  //Create propagation loss matrix. This is a matrix that defines the amount of loss
  //in the signal strength that happens due to the distance it travels 
  // (Recall signal strength vs distance graph shown in class
 
 //First create the loss matrix object called "lossModel"
  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  // set default loss to 200 dB - so much loss that there's essentially no signal coverage
  lossModel->SetDefaultLoss (200); 
  //See above: we are creating a model in which n0 - n1  and n1 - n2 are connected
  //So set their loss to lesser amount
  
  // set symmetric loss 0 <-> 1 to 50 dB, good coverage
    lossModel->SetLoss (nodes.Get (0)->GetObject<MobilityModel> (), nodes.Get (1)->GetObject<MobilityModel> (), 50); 
  
  // set symmetric loss 0 <-> 2 to 50 dB, good coverage
    lossModel->SetLoss (nodes.Get (0)->GetObject<MobilityModel> (), nodes.Get (2)->GetObject<MobilityModel> (), 50); 
  
  
  // Create a YansWifiChannel type object in the variable called "wifiChannel"
  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
  
  //set wifiChannel's loss model to the lossModel we just defined
  wifiChannel->SetPropagationLossModel (lossModel);
  
  //set Propagation delay based on some constant speed (value built in the library)
  wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

  // 5. Install wireless devices
  
  //Declare a wifi helper object called "wifi"
  WifiHelper wifi;
  
  //Set the PHY standard of this helper object to PHY of 802.11a
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  
  //RemotStationManager - method for setting rate characteristics of the channel 
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate54Mbps"));
  
  
  //Create the wifi Phy helper object, call it "wifiPhy"
  // Yans stands for Yet Another Network Simulator
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  
  //Assign "wifiChannel" we created earlier, to the channel of this helper object
  wifiPhy.SetChannel (wifiChannel);
  
 //Declare a WifiMac helper object called wifiMac
  WifiMacHelper wifiMac;
  
  //Set its type to Adhoc (as opposed to infrastructure, we are not simulating APs
  wifiMac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
  
 //Now, finally create the actual devices, in the Device container called devices
 // and "Install" the Phy and the Mac helper objects on the "nodes" created earlier
 // This "installation" returns the container of netDevices which which actually 
 //represent the network interfaces of these nodes
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // uncomment the following to have pcap output
  wifiPhy.EnablePcap (enableCtsRts ? "rtscts-pcap-node" : "basic-pcap-node" , nodes);


  // Do the usual routine for Internet stack installation 
  
  //Declare the helper object called "internet"
  InternetStackHelper internet;
  
  //Install it on all the nodes
  internet.Install (nodes);
  
  //Declare the address helper object "ipv4"
  Ipv4AddressHelper ipv4;
  
  //Call the SetBase function, which defines the network prefix and the subnet mask
  //In essence this command is saying that IP addresses in this network
  //will be of the pattern 10.*.*.*
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  
 //Assign these IP addresses to the interfaces in the device container 
  ipv4.Assign (devices);

  // Now Install applications: two CBR streams each saturating the channel
  
  //Declare the application container
  ApplicationContainer cbrApps;
  
  //port for the CBR stream
  uint16_t cbrPort = 12345;
  
  //OnOff source helper. An On off source generates packets for some fixed amount of time, then is silent
  //for some fixed amount of time and this cycle continues
 //onOffhelper object created with UDP as transport layer, and (10.0.0.1,cbrPort) as destination IP and port
 //With this statement we are assuming that 10.0.0.1 as the IP address of node "n0"
  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address ("10.0.0.1"), cbrPort));
  
  //Packet size, datarate attributes of the OnOffHelper object
  uint32_t payloadSize = 2200;                       /* Transport layer payload size in bytes. */
  std::string dataRate = "10Mbps"; 
   
  //OnOff source can be set to just generate packets at some data rate)
   onOffHelper.SetConstantRate(DataRate (dataRate), payloadSize);
  

  // flow 1:  node 1 -> node 0. Set start time attributes  
  //Now Install this helper object on ni, and add the returned object, an application to the cbrApps container
  
	 double stime;
	 stime = 1.0000+  (double) 1/100.0;  
	 onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (stime)));
	 cbrApps.Add (onOffHelper.Install (nodes.Get (1)));

  // flow 2:  node 1 -> node 0. Set start time attributes slightly different
  /** \internal
   * The slightly different start times and data rates are a workaround
   * for \bugid{388} and \bugid{912}
   */

	 stime = 1.0000+  (double) 2/100.0;  
	 onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (stime)));
	 cbrApps.Add (onOffHelper.Install (nodes.Get (2)));

  /** \internal
   * We also use separate UDP applications that will send a single
   * packet before the CBR flows start.
   * This is a workaround for the lack of perfect ARP, see \bugid{187}
   */
  uint16_t  echoPort = 9;
  UdpEchoClientHelper echoClientHelper (Ipv4Address ("10.0.0.1"), echoPort);
  echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10));
  ApplicationContainer pingApps;

  // again using different start times to workaround Bug 388 and Bug 912
  echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds ((double)1/1000)));
  pingApps.Add (echoClientHelper.Install (nodes.Get (1)));

echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds ((double)2/1000)));
  pingApps.Add (echoClientHelper.Install (nodes.Get (2)));


  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Run simulation for 8 seconds
  Simulator::Stop (Seconds (8));
  Simulator::Run ();

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  double totalTput = 0.0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // first 2 FlowIds are for ECHO apps, we don't want to display them
      //
      // Duration for throughput measurement is 7.0 seconds, since
      //   StartTime of the OnOffApplication is at about "second 1"
      // and
      //   Simulator::Stops at "second 8".
      if (i->first > 2)
        {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 7.0 / 1000 / 1000  << " Mbps\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 7.0 / 1000 / 1000  << " Mbps\n";
          totalTput += i->second.rxBytes * 8.0 / 7.0 / 1000 / 1000 ;
        }
    }
  std::cout << "Total channel throughput = " << totalTput << std::endl;
  // Cleanup
  Simulator::Destroy ();
}

int main (int argc, char **argv)
{
  //Ignore this command line setup

  std::string wifiManager ("Arf");
  CommandLine cmd;
  cmd.AddValue ("wifiManager", "Set wifi rate manager (Aarf, Aarfcd, Amrr, Arf, Cara, Ideal, Minstrel, Onoe, Rraa)", wifiManager);
  cmd.Parse (argc, argv);
  //**Upto here
  
  std::cout << "Hidden station experiment with RTS/CTS disabled:\n" << std::flush;
  experiment (false, wifiManager);
  std::cout << "------------------------------------------------\n";
  std::cout << "Hidden station experiment with RTS/CTS enabled:\n";
  experiment (true, wifiManager);

  return 0;
}
