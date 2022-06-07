#!/bin/bash
# 
#              VLANy0                VLANy1
#        ┌─────────────────┬─────────────────────────┐
#        │                 │                         │
#        │ ┌─────┐ ┌─────┐ │ ┌─────┐ ┌─────┐ ┌─────┐ │
# PORTs  │ │  p  │ │  p  │ │ │  p  │ │  p  │ │  p  │ │
#        │ └─┐ ┌─┘ └─┐ ┌─┘ │ └─┐ ┌─┘ └─┐ ┌─┘ └─┐ ┌─┘ │
#        │   └┬┘     └┬┘   │   └┬┘     └┬┘     └┬┘   │
#        │    │       │    │    │       │       │    │
#        └────┼───────┼────┴────┼───────┼───────┼────┘
#             │       │         │       │       │
#             │       └──┐   ┌──┘       │       │
#             │          │   │          │       │
#           .1│      .254│   │.235      │.1     │.254
#        ┌────┴┐        ┌┴───┴┐      ┌──┴──┐ ┌──┴──┐       Internet
#        │tuxy1│        │tuxy4│      │tuxy2│ │ RC  ├─────────────────
#        └─────┘        └─────┘      └─────┘ └─────┘    172.16.1.19/24
#           172.16.y0.0/24 | 172.16.y1.0/24



read -e -p "Bancada? > " -i "1" Y

read -e -p "Serial Port for switch configuration? > ttyS" -i "0" SPORT
read -e -p "Baudrate? > " -i "9600" BAUD



#printf "              VLAN%s0                VLAN%s1\n        ┌─────────────────┬─────────────────────────┐\n        │                 │                         │\n        │ ┌─────┐ ┌─────┐ │ ┌─────┐ ┌─────┐ ┌─────┐ │\n PORTs  │ │  p  │ │  p  │ │ │  p  │ │  p  │ │  p  │ │\n        │ └─┐ ┌─┘ └─┐ ┌─┘ │ └─┐ ┌─┘ └─┐ ┌─┘ └─┐ ┌─┘ │\n        │   └┬┘     └┬┘   │   └┬┘     └┬┘     └┬┘   │\n        │    │       │    │    │       │       │    │\n        └────┼───────┼────┴────┼───────┼───────┼────┘\n             │       │         │       │       │\n             │       └──┐   ┌──┘       │       │\n             │          │   │          │       │\n           .1│      .254│   │.235      │.1     │.254\n        ┌────┴┐        ┌┴───┴┐      ┌──┴──┐ ┌──┴──┐       Internet\n        │tux%s1│        │tux%s4│      │tux%s2│ │ RC  ├─────────────────\n        └─────┘        └─────┘      └─────┘ └─────┘    172.16.1.19/24\n           172.16.%s0.0/24 | 172.16.%s1.0/24" "$Y" "$Y" "$Y" "$Y" "$Y" "$Y" "$Y"


printf "Setting up Connection to router\n"
#stty -F /dev/ttyS$PORT $BAUD
#printf "Login\n"
#echo "root" > /dev/ttyS$PORT
#echo "8nortel" > /dev/ttyS$PORT
printf "Reseting router \n"
#echo "enable" >/dev/ttyS$port
#echo "configure terminal" >/dev/ttyS$port
#echo "no vlan 2-4094" >/dev/ttyS$port
#echo "exit" >/dev/ttyS$port
#echo "copy flash:tux1-clean startup-config" >/dev/ttyS$port
#echo "reload" >/dev/ttyS$port
printf "Configuring both VLANs\n"
# echo "configure terminal " > /dev/ttyS$PORT
# echo "vlan 10" > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
# echo "configure terminal " > /dev/ttyS$PORT
# echo "vlan 11" > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
read -p "Which Port is tuxY1 connected? > " TUX1
# echo "configure terminal " > /dev/ttyS$PORT
# echo "interface fastethernet 0/$TUX1" > /dev/ttyS$PORT
# echo "switchport mode access " > /dev/ttyS$PORT
# echo "switchport access vlan 10 " > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
read -p "Which Port is tuxY4 eth0 connected? > " TUX40
# echo "configure terminal " > /dev/ttyS$PORT
# echo "interface fastethernet 0/$TUX40" > /dev/ttyS$PORT
# echo "switchport mode access " > /dev/ttyS$PORT
# echo "switchport access vlan 10 " > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
read -p "Which Port is tuxY4 eth1 connected? > " TUX41
# echo "configure terminal " > /dev/ttyS$PORT
# echo "interface fastethernet 0/$TUX41" > /dev/ttyS$PORT
# echo "switchport mode access " > /dev/ttyS$PORT
# echo "switchport access vlan 11 " > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
read -p "Which Port is tuxY2 connected? > " TUX2
# echo "configure terminal " > /dev/ttyS$PORT
# echo "interface fastethernet 0/$TUX2" > /dev/ttyS$PORT
# echo "switchport mode access " > /dev/ttyS$PORT
# echo "switchport access vlan 11 " > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT
read -p "Which Port is Router connected? > " ROUT
# echo "configure terminal " > /dev/ttyS$PORT
# echo "interface fastethernet 0/$ROUT" > /dev/ttyS$PORT
# echo "switchport mode access " > /dev/ttyS$PORT
# echo "switchport access vlan 11 " > /dev/ttyS$PORT
# echo "end" > /dev/ttyS$PORT


printf "              VLAN%s0                VLAN%s1\n        ┌─────────────────┬─────────────────────────┐\n        │                 │                         │\n        │ ┌─────┐ ┌─────┐ │ ┌─────┐ ┌─────┐ ┌─────┐ │\n PORTs  │ │  %s  │ │  %s  │ │ │  %s  │ │  %s  │ │  %s  │ │\n        │ └─┐ ┌─┘ └─┐ ┌─┘ │ └─┐ ┌─┘ └─┐ ┌─┘ └─┐ ┌─┘ │\n        │   └┬┘     └┬┘   │   └┬┘     └┬┘     └┬┘   │\n        │    │       │    │    │       │       │    │\n        └────┼───────┼────┴────┼───────┼───────┼────┘\n             │       │         │       │       │\n             │       └──┐   ┌──┘       │       │\n             │          │   │          │       │\n           .1│      .254│   │.235      │.1     │.254\n        ┌────┴┐        ┌┴───┴┐      ┌──┴──┐ ┌──┴──┐       Internet\n        │tux%s1│        │tux%s4│      │tux%s2│ │ RC  ├─────────────────\n        └─────┘        └─────┘      └─────┘ └─────┘    172.16.1.19/24\n           172.16.%s0.0/24 | 172.16.%s1.0/24" "$Y" "$Y" "$TUX1" "$TUX40" "$TUX41" "$TUX2" "$ROUT" "$Y" "$Y" "$Y" "$Y" "$Y" "$Y"




