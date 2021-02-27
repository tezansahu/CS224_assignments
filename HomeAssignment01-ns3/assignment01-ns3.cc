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
 * Topology: ?
 *
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
#include "ns3/applications-module.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"


using namespace ns3;
//using namespace std;

void experiment (bool enableCtsRts, std::string wifiManager)
{
  int M = 4;
  // Enter the number of nodes for simulation
  std::cout << "Number of Nodes (M) [Multiple of 4] = ";
  std::cin >> M;

  // Enable or disable CTS/RTS based on argument enableCtsRts
  //ctsThr is the frame size over which RTS/CTS will be applied
  // It is set to a low value of enableRtsCts is true (so that)
  // it's mostly applied, and set to a high value when enableRtsCts is false
  //so that it's mostly not applied
  
  //This statement sets the threshold variable
  UintegerValue ctsThr = (enableCtsRts ? UintegerValue (100) : UintegerValue (10000));
  //This statement passes the threshold variable to the configuration method
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

  // Declare a NodeContainer variable called "nodes"
  NodeContainer nodes;
  
  // Call the create method of that object, asking it to create given number of nodes
  nodes.Create (M);

  // Place nodes somehow, this is required by every wireless simulation
  for (uint8_t i = 0; i < M; ++i)
    {
      nodes.Get (i)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
    }

  //Create propagation loss matrix. This is a matrix that defines the amount of loss
  //in the signal strength that happens due to the distance it travels 
  // (Recall signal strength vs distance graph shown in class
 
 //First create the loss matrix object called "lossModel"
  Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
  // set default loss to 50 dB 
  lossModel->SetDefaultLoss (50); 
  
  // Create a YansWifiChannel type object in the variable called "wifiChannel"
  Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
  
  //set wifiChannel's loss model to the lossModel we just defined
  wifiChannel->SetPropagationLossModel (lossModel);
  
  //set Propagation delay based on some constant speed (value built in the library)
  wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

  // Install wireless devices
  
  //Declare a wifi helper object called "wifi"
  WifiHelper wifi;
  
  //Set the PHY standard of this helper object to PHY of 802.11a
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);
  
  //RemotStationManager - method for setting some characteristics of the channel
  
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
  Ipv4InterfaceContainer allIPs = ipv4.Assign (devices);

  // Now Install applications
 
 
  
  //Declare the application container
  ApplicationContainer cbrApps;
  
 uint16_t cbrPort = 12345;
  
  
 uint32_t payloadSize = 2200;          
              
 std::string dataRate = "2Mbps";  // Transport layer payload size in bytes. 
  
//Now Install this helper object and add the application to the cbrApps container

    for (uint8_t i = 0; i < M/2; i+=2){
       OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (allIPs.GetAddress(i+1), cbrPort));
	double startTimeCBR=0;
	startTimeCBR = 1.0000+  (double) 1/100.0;  
	onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (startTimeCBR)));
       onOffHelper.SetConstantRate(DataRate (dataRate), payloadSize);

       cbrApps.Add (onOffHelper.Install (nodes.Get (i)));
     }


// Install application:  FTP data stream on remaining half
  ApplicationContainer ftpApps;
  uint16_t ftpPort = 54321;


   int maxBytes = 600000;
   int DONOTCHANGETHIS = 10; //in Segments
   Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue(DONOTCHANGETHIS));    

   //ns3::TcpSocket::SetDelAckMaxCount	(	(uint32_t) 	1)	

   
   int senderWindowSize = 1100; //in bytes. This determines the sender window size used. 
   std::cout<<"Window Size (in bytes) = ";
   std::cin>>senderWindowSize;

   Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue(senderWindowSize)); 

   for (uint8_t i = M/2; i < M; i+=2) {
     BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (allIPs.GetAddress(i+1), ftpPort));
     // Set the amount of data to send in bytes.  Zero is unlimited.
     source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
     ApplicationContainer sourceApps = source.Install (nodes.Get (i));
	double startTimeFTP =0;
     startTimeFTP = 1.0001+  (double) 1/100.0;  
     sourceApps.Start (Seconds (startTimeFTP));
    
     PacketSinkHelper sinkFTP ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny(), ftpPort));
     ApplicationContainer sinkApps = sinkFTP.Install (nodes.Get (i+1));
     sinkApps.Start (Seconds (startTimeFTP)); 
   }

  /** \internal
   * The slightly different start times and data rates are a workaround
   * for \bugid{388} and \bugid{912}
   */

  /** \internal
   * We also use separate UDP applications that will send a single
   * packet before the CBR flows start.
   * This is a workaround for the lack of perfect ARP, see \bugid{187}
   */

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Stop (Seconds (8));
  Simulator::Run ();

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  double totalTput = 0.0, ftpDelay=0.0, ftpDelaySum=0.0, count=0, ftpTput =0.0;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {

      
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << ", " << t.sourcePort << " -> " 
                << t.destinationAddress << ", " << t.destinationPort << ")\n";
          std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Input Load\t\t" << i->second.txBytes * 8.0 / 
			         (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / 
					1024 /1024 << " Mbps" << std::endl;
          double_t tput =  i->second.rxBytes * 8.0 / 
					(i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024;
					
          std::cout << "  Observed Throughput\t" << tput << " Mbps" << std::endl;
          totalTput +=  tput;
          //Write code below to output the FILE transfer delay. i.e time from transmission of first packet at sender
          // to time of receiving of last packet at receiver
          //Hint: observe the flow monitor variables used in calculation of "tput" above (denominator). 
          //Complete below line and uncomment
		  ftpDelay =  i->second.timeLastRxPacket.GetSeconds()-i->second.timeFirstTxPacket.GetSeconds();
		  std::cout << "Full Data transfer delay = "  << ftpDelay  << " seconds " << std::endl  ;
		  if (t.destinationPort == 54321) { ftpDelaySum +=  ftpDelay; count++; ftpTput += tput;}



  }
  std::cout << "Total channel throughput = " << totalTput << "Mbps" << std::endl;
  std::cout << "FTP throughput = " << ftpTput << "Mbps" << std::endl;
 std::cout << "Average File Transfer Delay = " << ftpDelaySum/count << " seconds" << std::endl;
 

  // Cleanup
  Simulator::Destroy ();
}

int main (int argc, char **argv)
{
  std::string wifiManager ("Arf");
  //Ignore this command line setup
  CommandLine cmd;
  cmd.AddValue ("wifiManager", "Set wifi rate manager (Aarf, Aarfcd, Amrr, Arf, Cara, Ideal, Minstrel, Onoe, Rraa)", wifiManager);
  cmd.Parse (argc, argv);
  //**Upto here
  
  std::cout << "FTP-CBR Experiment with RTS/CTS disabled:\n" << std::flush;
  experiment (false, wifiManager);
  std::cout << "------------------------------------------------\n";

  return 0;
}
