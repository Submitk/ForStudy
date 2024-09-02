#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char model[256];
    char capacity[256];
    char rate[256];
    char bandwidth[256];
    char fw_version[256];
    char vendor[256];
    char serial_number[256];
    char interface[256];
} DiskInfo;

void execute_command(const char *command, char *result, size_t max_len) {
    FILE *fp;
    char path[1035];

    fp = popen(command, "r");
    if (fp == NULL) {
        snprintf(result, max_len, "N/A");
        return;
    }

    if (fgets(path, sizeof(path) - 1, fp) != NULL) {
        strncpy(result, path, max_len);
    } else {
        snprintf(result, max_len, "N/A");
    }

    result[strcspn(result, "\n")] = '\0';

    pclose(fp);
}

void get_nvme_disk_info(const char *device, DiskInfo *disk) {
    char command[1024];
    
    snprintf(command, sizeof(command), "nvme list | grep /dev/%s | awk '{print $2, $3, $4}' | xargs", device);
    execute_command(command, disk->serial_number, sizeof(disk->serial_number));
    
    snprintf(command, sizeof(command), "nvme list | grep /dev/%s | awk '{print $5, $6, $7, $8}' | xargs", device);
    execute_command(command, disk->model, sizeof(disk->model));
    
    snprintf(command, sizeof(command), "nvme list | grep /dev/%s | awk '{print $9}'", device);
    execute_command(command, disk->fw_version, sizeof(disk->fw_version));
    
    snprintf(command, sizeof(command), "nvme list | grep /dev/%s | awk '{print $9, $10, $11, $12, $13, $14, $15, $16, $17}' | xargs", device);
    execute_command(command, disk->capacity, sizeof(disk->capacity));

    // Dummy placeholders for rate and bandwidth as they are not straightforward to get from CLI tools
    snprintf(disk->rate, sizeof(disk->rate), "N/A");
    snprintf(disk->bandwidth, sizeof(disk->bandwidth), "N/A");

    // Set interface type as NVMe
    snprintf(disk->interface, sizeof(disk->interface), "NVMe");
}

void get_smartctl_disk_info(const char *device, DiskInfo *disk) {
    char command[1024];
    
    // Use lsblk to get capacity and interface type
    snprintf(command, sizeof(command), "lsblk -dn -o SIZE,NAME | grep %s | awk '{print $1}'", device);
    execute_command(command, disk->capacity, sizeof(disk->capacity));

    snprintf(command, sizeof(command), "lsblk -dn -o TYPE,NAME | grep %s | awk '{print $1}'", device);
    execute_command(command, disk->interface, sizeof(disk->interface));

    // Use smartctl to get model, serial number, firmware version, vendor
    snprintf(command, sizeof(command), "sudo smartctl -i /dev/%s | grep 'Device Model' | awk -F: '{print $2}' | xargs", device);
    execute_command(command, disk->model, sizeof(disk->model));

    snprintf(command, sizeof(command), "sudo smartctl -i /dev/%s | grep 'Serial Number' | awk -F: '{print $2}' | xargs", device);
    execute_command(command, disk->serial_number, sizeof(disk->serial_number));

    snprintf(command, sizeof(command), "sudo smartctl -i /dev/%s | grep 'Firmware Version' | awk -F: '{print $2}' | xargs", device);
    execute_command(command, disk->fw_version, sizeof(disk->fw_version));

    snprintf(command, sizeof(command), "sudo smartctl -i /dev/%s | grep 'Vendor' | awk -F: '{print $2}' | xargs", device);
    execute_command(command, disk->vendor, sizeof(disk->vendor));

    // Dummy placeholders for rate and bandwidth as they are not straightforward to get from CLI tools
    snprintf(disk->rate, sizeof(disk->rate), "N/A");
    snprintf(disk->bandwidth, sizeof(disk->bandwidth), "N/A");
}

int is_nvme_device(const char *device) {
    char command[1024];
    char result[256];

    snprintf(command, sizeof(command), "lsblk -dn -o NAME,TYPE | grep %s | awk '{print $2}'", device);
    execute_command(command, result, sizeof(result));

    return strcmp(result, "disk") == 0 && strstr(device, "nvme") != NULL;
}

int main() {
    char device[256] = "nvme0n1"; // Change this to the device you want to query

    DiskInfo disk;
    memset(&disk, 0, sizeof(disk));

    if (is_nvme_device(device)) {
        get_nvme_disk_info(device, &disk);
    } else {
        get_smartctl_disk_info(device, &disk);
    }

    printf("Model:                %s\n", disk.model);
    printf("Capacity:             %s\n", disk.capacity);
    printf("Rate:                 %s\n", disk.rate);
    printf("Bandwidth:            %s\n", disk.bandwidth);
    printf("Firmware Version:     %s\n", disk.fw_version);
    printf("Vendor:               %s\n", disk.vendor);
    printf("Serial Number:        %s\n", disk.serial_number);
    printf("Interface:            %s\n", disk.interface);

    return 0;
}