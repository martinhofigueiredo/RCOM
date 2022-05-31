# Config Tuxes

## 1.Config ips
No tux 1
```s
sudo ifconfig eth0 down
sudo ifconfig eth0 172.16.10.1/24
```
No tux 4

```s

sudo ifconfig eth0 down
sudo ifconfig eth0 172.16.10.254/24
sudo ifconfig eth1 down
sudo ifconfig eth1 172.16.11.253/24
```
No tux 2
```s
sudo ifconfig eth0 down
sudo ifconfig eth0 172.16.11.1/24
```
## 2.Config routes

No tux1
```s 
sudo route add -net 172.16.11.0/24 gw 172.16.10.254
sudo route add default gw 172.16.1.19
```
No tux2
```s 
sudo route add -net 172.16.10.0/24 gw 172.16.11.253
sudo route add default gw 172.16.1.19
```
## IP forwarding

```s 
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 >  /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
```
# Switches

root
8nortel


## Reset Switch

```s
enable
configure terminal
no vlan 2-4094
exit
copy flash:tux1-clean startup-config
reload
```

## Configure Vlan

```s
configure terminal 
vlan 10
end
configure terminal 
vlan 11
end
```
## Add Ports

- `PORT` Depende das ligaçoes fisicas no switche

```s
#tux 1 
configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 10 
end


# tux 4
configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 10 
end

# tux 4
configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 11 
end


# tux 2
configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 11 
end

# router
configure terminal 
interface fastethernet 0/PORT 
switchport mode access 
switchport access vlan 11 
end

```


# Router

## Reset Router

```s
enable
copy flash:tux1-clean startup-config
reload
```
## Config Router

```s
interface gigabitethernet 0/0
ip address 172.16.1.19 255.255.255.0
no shutdown
exit
show interface gigabitethernet 0/0
```

# DNS


```s
nano /etc/resolv.conf
search netlab.fe.up.pt
nameserver  172.16.1.2 
```



## Config RoUTER NAT

```s
conf t
interface gigabitethernet 0/0
ip address 172.16.11.254 255.255.255.0  
no shutdown  
ip nat inside  
exit   
interface gigabitethernet 0/1
ip address 172.16.1.19 255.255.255.0  
no shutdown  
ip nat outside  exit   
ip nat pool ovrld 172.16.1.19 172.16.1.19 prefix 24  
ip nat inside source list 1 pool ovrld overload   
access-list 1 permit 172.16.10.0 0.0.0.7 
access-list 1 permit 172.16.11.0 0.0.0.7   
ip route 0.0.0.0 0.0.0.0 172.16.1.254  
ip route 172.16.10.0 255.255.255.0 172.16.11.253  
end 
```