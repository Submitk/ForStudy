#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SLOTS 128

typedef struct {
    char locator[256];
    char size[256];
    char speed[256];
    char manufacturer[256];
    char part_number[256];
    char serial_number[256];
} MemorySlot;

void update_max_lengths(int *max_lengths, MemorySlot *slot) {
    if (strlen(slot->locator) > max_lengths[0]) max_lengths[0] = strlen(slot->locator);
    if (strlen(slot->size) > max_lengths[1]) max_lengths[1] = strlen(slot->size);
    if (strlen(slot->speed) > max_lengths[2]) max_lengths[2] = strlen(slot->speed);
    if (strlen(slot->manufacturer) > max_lengths[3]) max_lengths[3] = strlen(slot->manufacturer);
    if (strlen(slot->part_number) > max_lengths[4]) max_lengths[4] = strlen(slot->part_number);
    if (strlen(slot->serial_number) > max_lengths[5]) max_lengths[5] = strlen(slot->serial_number);
}

int main() {
    FILE *fp;
    char path[1035];
    MemorySlot slots[MAX_SLOTS];
    int slot_count = 0;
    int max_lengths[6] = {0, 0, 0, 0, 0, 0};

    // Use dmidecode command to get memory information
    fp = popen("dmidecode -t memory", "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        exit(1);
    }

    MemorySlot current_slot = {0};
    int memory_info_started = 0;
    while (fgets(path, sizeof(path) - 1, fp) != NULL) {
        if (memory_info_started) {
            if (path[0] == '\n') {
                memory_info_started = 0;
                slots[slot_count++] = current_slot;
                update_max_lengths(max_lengths, &current_slot);
                memset(&current_slot, 0, sizeof(MemorySlot)); // Reset current slot
            } else {
                if (strstr(path, "Locator:")) {
                    sscanf(path, " Locator: %255[^\n]", current_slot.locator);
                } else if (strstr(path, "Size:")) {
                    sscanf(path, " Size: %255[^\n]", current_slot.size);
                } else if (strstr(path, "Speed:")) {
                    sscanf(path, " Speed: %255[^\n]", current_slot.speed);
                } else if (strstr(path, "Manufacturer:")) {
                    sscanf(path, " Manufacturer: %255[^\n]", current_slot.manufacturer);
                } else if (strstr(path, "Part Number:")) {
                    sscanf(path, " Part Number: %255[^\n]", current_slot.part_number);
                } else if (strstr(path, "Serial Number:")) {
                    sscanf(path, " Serial Number: %255[^\n]", current_slot.serial_number);
                }
            }
        } else if (strstr(path, "Memory Device")) {
            memory_info_started = 1;
        }
    }
    pclose(fp);

    // Adding extra spaces for better readability
    int extra_spaces = 5; // Adjust the extra space value to ensure readability

    // Calculate total width for properly spaced columns
    for (int i = 0; i < 6; i++) {
        max_lengths[i] += extra_spaces;
    }

    // Print the header with dynamically determined widths
    printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
           max_lengths[0] + 2, "Locator", // Add 2 extra spaces for visual separation
           max_lengths[1] + 2, "Size",
           max_lengths[2] + 2, "Speed",
           max_lengths[3] + 2, "Manufacturer",
           max_lengths[4] + 2, "Part Number",
           max_lengths[5] + 2, "Serial Number");

    // Print memory slot information with dynamically determined widths
    for (int i = 0; i < slot_count; i++) {
        printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
               max_lengths[0] + 2, slots[i].locator,
               max_lengths[1] + 2, slots[i].size,
               max_lengths[2] + 2, slots[i].speed,
               max_lengths[3] + 2, slots[i].manufacturer,
               max_lengths[4] + 2, slots[i].part_number,
               max_lengths[5] + 2, slots[i].serial_number);
    }

    return 0;
}