# Config Tuxes

## 1.Config ips
No tux 1
```s
sudo ifconfig eth0 172.16.10.1/24
```
No tux 4

```s
sudo ifconfig eth0 172.16.10.254/24
sudo ifconfig eth1 172.16.11.253/24
```
No tux 2
```s
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
echo 0 >  /proc/sys/net/ipv4/imp_echo_ignore_broadcasts```

# Switches

## Reset Switch

```s
enable
configure terminal
no vlan 2-4094
exit
copy flash:tux1-clean startup config
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
configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 10 
end

configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 10 
end

configure terminal 
interface fastethernet 0/PORT
switchport mode access 
switchport access vlan 11 
end

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
copy flash:tux1-clean startup config
reload
```
## Config Router

```s
interface gigabitethernet 0/0
ip address 172.16.1.19 255.255.255.0
no shutdown
exit
show interface gigabitethernet 0/0```
