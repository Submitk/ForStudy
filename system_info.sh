#!/bin/bash

clear

LogFileName=polling.$(date +%F-%T)

EchoFormat=$(for (( i=0;i<30;i++ ));do echo -n "=";done)

# System Information

SystemInfo(){

printf "${EchoFormat} System Information ${EchoFormat}\n"

printf "System Type: %-10s\n" $(uname -a| awk '{print $NF}')

printf "System Version: %-10s\n" "$(cat /etc/redhat-release)"

printf "Kernel Information: %-10s\n" $(uname -r)

printf "Hostname: %-10s\n" $(uname -n)

printf "Encoding Format: %-10s\n" ${LANG}

printf "Current System Time: %-10s %-10s\n" $(date +%F) $(date +%T)

printf "System Load Average: %-4s %-1s\n" $(uptime | awk -F: '{print $5 }'|awk -F, '{print $1,"%"}')

printf "System Uptime (days): %-10s\n" $(uptime |awk '{print $3}')

printf "Number of Online Users: %-3s\n" $(w|tail -n +3|wc -l)

printf "SELinux Status: %-10s\n" $(grep "SELINUX=[d|e|p]" /etc/selinux/config |awk -F= '{print $2}')

echo -e "Last Modification Time: $(uptime -p)"

echo -e "IP Address: $(hostname -I | cut -d' ' -f1)"

echo -e "CPU Processor: $(lscpu | grep "Model name:" | sed 's/Model name:\s*//')"

echo -e "Memory Usage: $(free -h | awk '/^Mem:/ { print $3 "/" $2 }')"

echo -e "Swap Usage: $(free -h | awk '/^Swap:/ { print $3 "/" $2 }')"

}


# CPU Information

CpuInfo(){

MemonyId=$(top -b -n1|awk 'NR==3'|awk -F, '{print $4}'| cut -c 1-5)

MemonyUse=$(echo "100-${MemonyId}" |bc)

printf "${EchoFormat} CPU Information ${EchoFormat}\n"

printf "Logical CPU Cores: %-3s\n" $(grep "processor" /proc/cpuinfo|sort -u|wc -l)

printf "Physical CPU Cores: %-3s\n" $(grep "physical id" /proc/cpuinfo |sort -u|wc -l)

printf "CPU Architecture: %-3s\n" $(uname -m)

printf "CPU Model: %-3s\n" "$(grep "model name" /proc/cpuinfo |awk -F: '{print $2}'|sort -u|cut -c 2-50)"

echo -e "CPU Load in 1 Minute: `awk  '{printf "%15s",$1}' /proc/loadavg`"

echo -e "CPU Load in 5 Minutes: `awk  '{printf "%15s",$2}' /proc/loadavg`"

echo -e "CPU Load in 10 Minutes: `awk  '{printf "%15s",$3}' /proc/loadavg`"

printf "CPU Usage Percentage: %-1s %-1s\n" ${MemonyUse} %

printf "CPU Idle Percentage: %-1s %-1s\n" ${MemonyId} %

printf "Top 10 CPU-consuming Processes:\n\n"

ps -eo user,pid,pcpu,pmem,args --sort=-pcpu  |head -n 10

}


# Memory Information

MemoryInfo(){

printf "${EchoFormat} Memory Information ${EchoFormat}\n"

printf "Total Memory: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $2}')

printf "Used Memory: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $3}')

printf "Free Memory: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $4}')

printf "Memory Usage Percentage: %-1s %-1s\n" $(free | grep -i mem |awk '{print $6/$2*100}'|cut -c1-5) %

printf "Top 10 Memory-consuming Processes:\n\n"

ps -eo user,pid,pcpu,pmem,args --sort=-pmem  |head -n 10

}


# Disk Usage Information

Disk_Info() {

printf "${EchoFormat} Disk Partition Usage ${EchoFormat}\n"

df -h

echo

}


# Swap Information

SwapInfo(){

printf "${EchoFormat} Swap Information ${EchoFormat}\n"

printf "Total Swap: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $2}')

printf "Used Swap: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $3}')

printf "Free Swap: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $4}')

}


# Network Information

NetworkInfo(){

printf "${EchoFormat} Network Information ${EchoFormat}\n"

printf "IP Address: %-1s %-1s %-1s %-1s\n" $(ifconfig -a|grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d "addr:")

printf "Netmask: %-1s %-1s %-1s %-1s\n" $(ifconfig -a|grep "netmask"|grep -v 127.0.0.1|awk '{print $4}') 

printf "DNS Servers: %-1s %-1s %-1s %-1s %-1s\n" $(grep "nameserver" /etc/resolv.conf | awk '{print $2}')

if (ping -c2 -w2 www.baidu.com &>/dev/null);then

    printf "Network Connectivity: %s\n" Yes
    printf "Public IP Address: %s\n" $(curl -s ip.sb)  

else

    printf "Network Connectivity: %s\n" No

fi

}


# Docker Information

DockerInfo(){

    printf "${EchoFormat} Docker Status ${EchoFormat}\n"

    printf "Running Containers:\n $(docker ps --format "{{.Names}}")\n"

    printf "Stopped Containers:\n $(docker ps --format "{{.Names}}"|grep Exited)\n"

}


# Execute Functions

SystemInfo 

CpuInfo

MemoryInfo

Disk_Info

SwapInfo

NetworkInfo
