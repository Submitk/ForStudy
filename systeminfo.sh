#!/bin/bash

clear

LogFileName=polling.$(date +%F-%T)

    EchoFormat=$(for (( i=0;i<30;i++ ));do echo -n "=";done)

# 系统信息

SystemInfo(){

printf "${EchoFormat} 系统信息 ${EchoFormat}\n"

printf "系统类型: %-10s\n" $(uname -a| awk '{print $NF}')

    printf "系统版本: %-10s\n" "$(cat /etc/redhat-release)"

printf "内核信息: %-10s\n" $(uname -r)

    printf "主机名: %-10s\n" $(uname -n)

printf "编码格式: %-10s\n" ${LANG}

    printf "系统当前时间: %-10s %-10s\n" $(date +%F) $(date +%T)

    printf "系统运行负载: %-4s %-1s\n" $(uptime | awk -F: '{print $5 }'|awk -F, '{print $1,"%"}')

    printf "系统运行天数: %-10s\n" $(uptime |awk '{print $3}')

    printf "在线用户人数: %-3s\n" $(w|tail -n +3|wc -l)

printf "SELinux: %-10s\n" $(grep "SELINUX=[d|e|p]" /etc/selinux/config |awk -F= '{print $2}')

    echo -e "最后一次修改时间: $(uptime -p)"

echo -e "IP地址: $(hostname -I | cut -d' ' -f1)"

    echo -e "Cpu处理器: $(lscpu | grep "Model name:" | sed 's/Model name:\s*//')"

echo -e "内存空间: $(free -h | awk '/^Mem:/ { print $3 "/" $2 }')"

    echo -e "交换空间: $(free -h | awk '/^Swap:/ { print $3 "/" $2 }')"

}


# CPU信息

CpuInfo(){

MemonyId=$(top -b -n1|awk 'NR==3'|awk -F, '{print $4}'| cut -c 1-5)

    MemonyUse=$(echo "100-${MemonyId}" |bc)

    printf "${EchoFormat} CPU信息 ${EchoFormat}\n"

    printf "逻辑CPU核数: %-3s\n" $(grep "processor" /proc/cpuinfo|sort -u|wc -l)

printf "物理CPU核数: %-3s\n" $(grep "physical id" /proc/cpuinfo |sort -u|wc -l)

    printf "CPU架构: %-3s\n" $(uname -m)

printf "CPU设置型号: %-3s\n" "$(grep "model name" /proc/cpuinfo |awk -F: '{print $2}'|sort -u|cut -c 2-50)"

    echo -e "CPU 1分钟负载: `awk  '{printf "%15s",$1}' /proc/loadavg`"

    echo -e "CPU 5分钟负载: `awk  '{printf "%15s",$2}' /proc/loadavg`"

    echo -e "CPU10分钟负载: `awk  '{printf "%15s",$3}' /proc/loadavg`"

    printf "使用CPU占比: %-1s %-1s\n" ${MemonyUse} %

printf "空闲CPU占比: %-1s %-1s\n" ${MemonyId} %

printf "占用CPU Top10信息:\n\n"

ps -eo user,pid,pcpu,pmem,args --sort=-pcpu  |head -n 10

}


# Memory信息

MemoryInfo(){

printf "${EchoFormat} 内存信息 ${EchoFormat}\n"

printf "总共内存: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $2}')

    printf "使用内存: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $3}')

    printf "剩余内存: %-1s\n" $(free -mh|awk "NR==2"|awk '{print $4}')

    printf "内存使用占比: %-1s %-1s\n" $(free | grep -i mem |awk '{print $6/$2*100}'|cut -c1-5) %

printf "占用内存排名前10的soft:\n\n"

ps -eo user,pid,pcpu,pmem,args --sort=-pmem  |head -n 10

}


# 磁盘使用量排序：

Disk_Info() {

printf "${EchoFormat} 各分区使用率 ${EchoFormat}\n"

df -h

echo

}


# Swap信息

SwapInfo(){

printf "${EchoFormat} Swap信息 ${EchoFormat}\n"

printf "Swap总大小: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $2}')

    printf "已用Swap: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $3}')

    printf "可用Swap: %-1s\n" $(free -mh|awk "NR==3"|awk '{print $4}')

}


# 网络信息

NetworkInfo(){

printf "${EchoFormat} 网络信息 ${EchoFormat}\n"

echo -e "\nIPMI LAN Configuration:"
ipmitool lan print
echo -e "\n"
ipmitool lan print 8
echo -e "\n"

printf "IP地址: %-1s %-1s %-1s %-1s\n" $(ifconfig -a|grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d "addr:")

    printf "网关: %-1s %-1s %-1s %-1s\n" $(ifconfig -a|grep "netmask"|grep -v 127.0.0.1|awk '{print $4}') 

    printf "DNS: %-1s %-1s %-1s %-1s %-1s\n" $(grep "nameserver" /etc/resolv.conf | awk '{print $2}')

if (ping -c2 -w2 www.baidu.com &>/dev/null);then

    printf "网络是否连通: %s\n" 是
    printf "公网IP地址: %s\n" $(curl -s ip.sb)  

else

    printf "网络是否连通: %s\n" 否

fi



}


# docker检查

DockerInfo(){

    printf "${EchoFormat} docker运行情况 ${EchoFormat}\n"

    printf "当前正在运行的容器：\n $(docker ps --format "{{.Names}}")\n"

    printf "当前没有运行的容器：\n $(docker ps --format "{{.Names}}"|grep Exited)\n"

}


#直接登录后打印

SystemInfo 

CpuInfo

MemoryInfo

Disk_Info

SwapInfo

NetworkInfo


#登录后进入查看模式

# All(){

# SystemInfo 

# CpuInfo

# MemoryInfo

# Disk_Info

# SwapInfo

# NetworkInfo

# }

# All >${LogFileName};less ${LogFileName}