#
#  Copyright 2002, Univerity of Colorado at Boulder.                        
#                                                                            
#                         All Rights Reserved                                
#                                                                            
#  Permission to use, copy, modify, and distribute this software and its    
#  documentation for any purpose other than its incorporation into a        
#  commercial product is hereby granted without fee, provided that the      
#  above copyright notice appear in all copies and that both that           
#  copyright notice and this permission notice appear in supporting         
#  documentation, and that the name of the University not be used in        
#  advertising or publicity pertaining to distribution of the software      
#  without specific, written prior permission.                              
#                                                                            
#  UNIVERSITY OF COLORADO DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS      
#  SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND        
#  FITNESS FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL THE UNIVERSITY    
#  OF COLORADO BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL         
#  DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA      
#  OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER       
#  TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR         
#  PERFORMANCE OF THIS SOFTWARE.                                            
#
# nsclick-simple-lan.tcl
#
# A sample nsclick script simulating a small LAN
#

#
#	Modified by Nicolas Letor to support wifi elements.
# 	Performance Analysis of Telecommunication Systems (PATS) research group,
# 	Interdisciplinary Institute for Broadband Technology (IBBT) & Universiteit Antwerpen.
#

#
# Set some general simulation parameters
#

#
# Unity gain, omnidirectional antennas, centered 1.5m above each node.
# These values are lifted from the ns-2 sample files.
#

Antenna/OmniAntenna set X_ 0
Antenna/OmniAntenna set Y_ 0
Antenna/OmniAntenna set Z_ 1.5
Antenna/OmniAntenna set Gt_ 1.0
Antenna/OmniAntenna set Gr_ 1.0

#
# Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
# These are taken directly from the ns-2 sample files.
#
Phy/WirelessPhy set CPThresh_ 10.0
Phy/WirelessPhy set CSThresh_ 1.559e-11
Phy/WirelessPhy set RXThresh_ 3.652e-10
Phy/WirelessPhy set Rb_ 2*1e6
Phy/WirelessPhy set Pt_ 0.2818
Phy/WirelessPhy set freq_ 914e+6 
Phy/WirelessPhy set L_ 1.0

# Configure multirate aspect
Phy/WirelessPhy set RateCount_ 4
Phy/WirelessPhy set Rate0 11e6
Phy/WirelessPhy set Rate1 5.5e6
Phy/WirelessPhy set Rate2 2e6
Phy/WirelessPhy set Rate3 1e6
Phy/WirelessPhy set RXThresh0 1.427e-08 ;# 100m transmission range
Phy/WirelessPhy set RXThresh1 2.818e-09 ;# 150m transmission range
Phy/WirelessPhy set RXThresh2 8.916e-10 ;# 200m transmission range
Phy/WirelessPhy set RXThresh3 3.652e-10 ;# 250m transmission range

# 
# Set the size of the playing field and the topography.
#
set xsize 100
set ysize 100
set wtopo	[new Topography]
$wtopo load_flatgrid $xsize $ysize

#
# The network channel, physical layer, MAC, propagation model,
# and antenna model are all standard ns-2.
#  
set netchan	Channel/WirelessChannel
set netphy	Phy/WirelessPhy
set netmac	Mac/802_11
set netprop     Propagation/TwoRayGround
set antenna     Antenna/OmniAntenna

#
# We have to use a special queue and link layer. This is so that
# Click can have control over the network interface packet queue,
# which is vital if we want to play with, e.g. QoS algorithms.
#
set netifq	Queue/ClickQueue
set netll	LL/Ext
LL set delay_			1ms

#
# These are pretty self-explanatory, just the number of nodes
# and when we'll stop.
#
set nodecount   4
set stoptime	10.0

#
# With nsclick, we have to worry about details like which network
# port to use for communication. This sets the default ports to 5000.
#
Agent/Null set sport_		5000
Agent/Null set dport_		5000

Agent/CBR set sport_		5000
Agent/CBR set dport_		5000

#
# Standard ns-2 stuff here - create the simulator object.
#
Simulator set MacTrace_ ON
set ns_		[new Simulator]

#
# Create and activate trace files.
#
set tracefd	[open "nsclick-raw-wlan.tr" w]
set namtrace    [open "nsclick-raw-wlan.nam" w]
$ns_ trace-all $tracefd
$ns_ namtrace-all-wireless $namtrace $xsize $ysize
$ns_ use-newtrace


#
# Create the "god" object. This is another artifact of using
# the mobile node type. We have to have this even though
# we never use it.
#
set god_ [create-god $nodecount]

#
# Tell the simulator to create Click nodes.
#
Simulator set node_factory_ Node/MobileNode/ClickNode

#
# Create a network Channel for the nodes to use. One channel
# per LAN. Create 13 channels of the 802.11 MAC layer
for {set i 0} { $i <= 13} {incr i} {
	set chan_($i) [new Channel/WirelessChannel]	
}
# And implement a channel switching function to WirelessInfo 
# to switch from one channel to another channel
proc SwitchChannel { i whichif whichnewchannel } {
	global node_ chan_
	if { $whichnewchannel > 0 } {
#		set ns_ [Simulator instance] 
#		set node [ $ns_ Node($i) ]
		$node_($i) changechannel $whichif $chan_($whichnewchannel)
		puts "Node $i changes if $whichif to channel $whichnewchannel"
	}
}

#Also set the propagation model to be used.
set prop_ [new $netprop]

#
# In nsclick we have to worry about assigning IP and MAC addresses
# to out network interfaces. Here we generate a list of IP and MAC
# addresses, one per node since we've only got one network interface
# per node in this case. Also note that this scheme only works for
# fewer than 255 nodes, and we aren't worrying about subnet masks.
#
set iptemplate "192.168.1.%d"
set mactemplate "00:03:47:70:89:%0x"
for {set i 0} {$i < $nodecount} {incr i} {
    set node_ip($i) [format $iptemplate [expr $i+1]]
    set node_mac($i) [format $mactemplate [expr $i+1]]
}

#
# We set the routing protocol to "Empty" so that ns-2 doesn't do
# any packet routing. All of the routing will be done by the
# Click script.
#
$ns_ rtproto Empty

#
# Here is where we actually create all of the nodes.
#
for {set i 0} {$i < $nodecount } {incr i} {
    set node_($i) [$ns_ node]

    #
    # After creating the node, we add one wireless network interface to
    # it. By default, this interface will be named "eth0". If we
    # added a second interface it would be named "eth1", a third
    # "eth2" and so on.
    #
    $node_($i) add-interface $chan_(1) $prop_ $netll $netmac \
	$netifq 1 $netphy $antenna

    #
    # Now configure the interface eth0
    #
    $node_($i) setip "eth0" $node_ip($i)
    $node_($i) setmac "eth0" $node_mac($i)

    #
    # Set some node properties
    #
    $node_($i) random-motion 0
    $node_($i) topography $wtopo
    $node_($i) nodetrace $tracefd

    #
    # The node name is used by Click to distinguish information
    # coming from different nodes. For example, a "Print" element
    # prepends this to the printed string so it's clear exactly
    # which node is doing the printing.
    #
    [$node_($i) set classifier_] setnodename "node$i-simplelan"
    
    #
    # Load the appropriate Click router script for the node.
    # All nodes in this simulation are using the same script,
    # but there's no reason why each node couldn't use a different
    # script.
    #
    [$node_($i) entry] loadclick "nsclick-raw-wlan.click"
}


# 
# Define node network traffic. There isn't a whole lot going on
# in this simple test case, we're just going to have the first node
# send packets to the last node, starting at 1 second, and ending at 10.
# There are Perl scripts available to automatically generate network
# traffic.
#


#
# Start transmitting at $startxmittime, $xmitrate packets per second.
#
set startxmittime 1
set xmitrate 4
set xmitinterval 0.25
set packetsize 64

#
# We use the "raw" packet type, which sends real packet data
# down the pipe.
#
set raw_(0) [new Agent/Raw]
$ns_ attach-agent $node_(0) $raw_(0)

set lastnode [expr $nodecount-1]
set null_(0) [new Agent/Null]
$ns_ attach-agent $node_($lastnode) $null_(0)

#
# The CBR object is just the default ns-2 CBR object, so
# no change in the meaning of the parameters.
#
set cbr_(0) [new Application/Traffic/CBR]
$cbr_(0) set packetSize_ $packetsize
$cbr_(0) set interval_ $xmitinterval
$cbr_(0) set random_ 0
$cbr_(0) set maxpkts_ [expr ($stoptime - $startxmittime)*$xmitrate]
$cbr_(0) attach-agent $raw_(0)

#
# The Raw agent creates real UDP packets, so it has to know
# the source and destination IP addresses and port numberes.
#
$raw_(0) set-srcip [$node_(0) getip eth0]
$raw_(0) set-srcport 5000
$raw_(0) set-destport 5000
$raw_(0) set-destip [$node_($lastnode) getip eth0]

$ns_ at $startxmittime "$cbr_(0) start"


$node_(0) set X_ 10
$node_(0) set Y_ 50
$node_(0) set Z_ 0

$node_(1) set X_ 50
$node_(1) set Y_ 50
$node_(1) set Z_ 0

$node_(2) set X_ 90
$node_(2) set Y_ 50
$node_(2) set Z_ 0

$node_(3) set X_ 50
$node_(3) set Y_ 10
$node_(3) set Z_ 0


#
# This sizes the nodes for use in nam. Currently, the trace files
# produced by nsclick don't really work in nam.
#
for {set i 0} {$i < $nodecount} {incr i} {
    $ns_ initial_node_pos $node_($i) 20
    [$node_($i) entry] runclick
}

#
# Stop the simulation
#
$ns_ at  $stoptime.000000001 "puts \"NS EXITING...\" ; $ns_ halt"

#
# Let nam know that the simulation is done.
#
$ns_ at  $stoptime	"$ns_ nam-end-wireless $stoptime"


puts "Starting Simulation..."
$ns_ run




