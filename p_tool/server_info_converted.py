# -*- coding: utf-8 -*-
import os
import platform
import psutil
import json
import subprocess

def save_to_file(filename, content):
    with open(filename, 'w') as f:
        f.write(content)

def get_system_info(base_dir):
    info = {}

    # Create serverlog directory if it doesn't exist
    if not os.path.exists(base_dir):
        os.makedirs(base_dir)
    
    # »ñÈ¡CPUÐÅÏ¢
    cpu_info = {
        "physical_cores": psutil.cpu_count(logical=False),
        "total_cores": psutil.cpu_count(logical=True),
        "frequency": psutil.cpu_freq()._asdict(),
        "usage": psutil.cpu_percent(interval=1, percpu=True)
    }
    save_to_file(os.path.join(base_dir, 'cpu_info.txt'), json.dumps(cpu_info, indent=4))
    info["cpu_summary"] = cpu_info

    # »ñÈ¡ÄÚ´æÐÅÏ¢
    memory_info = psutil.virtual_memory()._asdict()
    save_to_file(os.path.join(base_dir, 'memory_info.txt'), json.dumps(memory_info, indent=4))
    info["memory_summary"] = memory_info

    # »ñÈ¡Ó²ÅÌÐÅÏ¢
    partitions = psutil.disk_partitions()
    disk_info = {}
    for partition in partitions:
        usage = psutil.disk_usage(partition.mountpoint)._asdict()
        disk_info[partition.device] = {
            'mountpoint': partition.mountpoint,
            'fstype': partition.fstype,
            'opts': partition.opts,
            'usage': usage
        }
    save_to_file(os.path.join(base_dir, 'disk_info.txt'), json.dumps(disk_info, indent=4))
    info["disk_summary"] = disk_info

    # »ñÈ¡PCIÐÅÏ¢
    pci_info = subprocess.run(['lspci'], stdout=subprocess.PIPE, text=True).stdout
    save_to_file(os.path.join(base_dir, 'pci_info.txt'), pci_info)

    # »ñÈ¡DMIÐÅÏ¢
    dmi_info = subprocess.run(['dmidecode'], stdout=subprocess.PIPE, text=True).stdout
    save_to_file(os.path.join(base_dir, 'dmi_info.txt'), dmi_info)

    # »ñÈ¡IPMI¹¤¾ßÐÅÏ¢
    ipmi_info = subprocess.run(['ipmitool', 'sensor'], stdout=subprocess.PIPE, text=True).stdout
    save_to_file(os.path.join(base_dir, 'ipmitool_info.txt'), ipmi_info)

    # »ñÈ¡dmesgÈÕÖ¾
    dmesg_info = subprocess.run(['dmesg'], stdout=subprocess.PIPE, text=True).stdout
    save_to_file(os.path.join(base_dir, 'dmesg_log.txt'), dmesg_info)

    # »ñÈ¡/var/log/messagesÈÕÖ¾
    with open('/var/log/messages', 'r') as file:
        messages_log = file.read()
    save_to_file(os.path.join(base_dir, 'messages_log.txt'), messages_log)

    # »ñÈ¡MCEÈÕÖ¾
    mce_info = subprocess.run(['mcelog'], stdout=subprocess.PIPE, text=True).stdout
    save_to_file(os.path.join(base_dir, 'mce_log.txt'), mce_info)

    # »ñÈ¡¿ØÖÆ¿¨ÐÅÏ¢
    ctrl_card_info = subprocess.run(['lspci -vv'], stdout=subprocess.PIPE, text=True, shell=True).stdout
    save_to_file(os.path.join(base_dir, 'control_card_info.txt'), ctrl_card_info)

    return info

def main():
    base_dir = os.path.join(os.getcwd(), 'serverlog')
    system_info = get_system_info(base_dir)

    # ÔÚÖÕ¶ËÏÔÊ¾»ù±¾ÐÅÏ¢
    print("\n----- Basic System Information -----\n")
    print("CPU Information Summary:\n", json.dumps(system_info["cpu_summary"], indent=4))
    print("\nMemory Information Summary:\n", json.dumps(system_info["memory_summary"], indent=4))
    print("\nDisk Information Summary:\n", json.dumps(system_info["disk_summary"], indent=4))
    print("\n-------------------------------------\n")
    print(f"Detailed logs have been saved to the directory: {base_dir}")

if __name__ == "__main__":
    main()