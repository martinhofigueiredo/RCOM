#!/bin/bash

read -e -p "Bancada? > " -i "1" Y

read -e -p "Serial Port for Router configuration? > ttyS" -i "0" SPORT
read -e -p "Baudrate? > " -i "9600" BAUD
printf "Setting up Connection to router\n"
#stty -F /dev/ttyS$PORT $BAUD

printf "Reseting router \n"
#echo "enable" > /dev/ttyS$PORT
#echo "copy flash:tux1-clean startup-config" > /dev/ttyS$PORT
#echo "reload" > /dev/ttyS$PORT
printf "Configuring network acess\n"
# echo "interface gigabitethernet 0/0" >/dev/ttyS%PORT
# echo "ip address 172.16.1.19 255.255.255.0" >/dev/ttyS%PORT
# echo "no shutdown" >/dev/ttyS%PORT
# echo "exit" >/dev/ttyS%PORT
# echo "show interface gigabitethernet 0/0" >/dev/ttyS%PORT
printf "Configuring NAT"
# echo "conf t" >/dev/ttyS$PORT
# echo "interface gigabitethernet 0/0" >/dev/ttyS$PORT
# echo "ip address 172.16.11.254 255.255.255.0  " >/dev/ttyS$PORT
# echo "no shutdown  " >/dev/ttyS$PORT
# echo "ip nat inside  " >/dev/ttyS$PORT
# echo "exit   " >/dev/ttyS$PORT
# echo "interface gigabitethernet 0/1" >/dev/ttyS$PORT
# echo "ip address 172.16.1.19 255.255.255.0  " >/dev/ttyS$PORT
# echo "no shutdown  " >/dev/ttyS$PORT
# echo "ip nat outside  exit   " >/dev/ttyS$PORT
# echo "ip nat pool ovrld 172.16.1.19 172.16.1.19 prefix 24  " >/dev/ttyS$PORT
# echo "ip nat inside source list 1 pool ovrld overload   " >/dev/ttyS$PORT
# echo "access-list 1 permit 172.16.10.0 0.0.0.7 " >/dev/ttyS$PORT
# echo "access-list 1 permit 172.16.11.0 0.0.0.7   " >/dev/ttyS$PORT
# echo "ip route 0.0.0.0 0.0.0.0 172.16.1.254  " >/dev/ttyS$PORT
# echo "ip route 172.16.10.0 255.255.255.0 172.16.11.253  " >/dev/ttyS$PORT
# echo "end " >/dev/ttyS$PORT