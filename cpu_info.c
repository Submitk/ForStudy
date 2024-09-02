#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char model_name[256];
    char current_frequency[256];
    char max_frequency[256];
    char min_frequency[256];
    char l1_cache_size[256];
    char l2_cache_size[256];
    char l3_cache_size[256];
    char cores[256];
    char threads[256];
    char architecture[256];
    char voltage[256];
    char serial_number[256];
} CPUInfo;

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

    // remove any trailing newline character
    result[strcspn(result, "\n")] = '\0';

    pclose(fp);
}

void get_cpu_info(CPUInfo *cpu) {
    char buffer[1035];

    // Use lscpu to get basic info
    execute_command("lscpu | grep -m 1 'Model name' | awk -F: '{print $2}' | xargs", cpu->model_name, sizeof(cpu->model_name));
    execute_command("lscpu | grep -m 1 'CPU min MHz' | awk -F: '{print $2}' | xargs", cpu->min_frequency, sizeof(cpu->min_frequency));
    execute_command("lscpu | grep -m 1 'Architecture' | awk -F: '{print $2}' | xargs", cpu->architecture, sizeof(cpu->architecture));
    execute_command("lscpu | grep -m 1 'Thread(s) per core' | awk -F: '{print $2}' | xargs", buffer, sizeof(buffer));
    int threads_per_core = atoi(buffer);
    execute_command("lscpu | grep -m 1 'Core(s) per socket' | awk -F: '{print $2}' | xargs", buffer, sizeof(buffer));
    int cores_per_socket = atoi(buffer);
    int threads = threads_per_core * cores_per_socket;
    snprintf(cpu->threads, sizeof(cpu->threads), "%d", threads);

    // Use dmidecode -t processor to get detailed info
    execute_command("dmidecode -t 4 | grep -m 1 'Max Speed' | awk -F: '{print $2}' | xargs", cpu->max_frequency, sizeof(cpu->max_frequency));
    // Strip off extra "MHz" if already present
    if (strstr(cpu->max_frequency, " MHz") == NULL) {
        strncat(cpu->max_frequency, " MHz", sizeof(cpu->max_frequency) - strlen(cpu->max_frequency) - 1);
    }

    execute_command("dmidecode -t 4 | grep -m 1 'Serial Number' | awk -F: '{print $2}' | xargs", cpu->serial_number, sizeof(cpu->serial_number));
    execute_command("dmidecode -t 4 | grep -m 1 'Voltage' | awk -F: '{print $2}' | xargs", cpu->voltage, sizeof(cpu->voltage));
    execute_command("dmidecode -t 4 | grep -m 1 'Core Count' | awk -F: '{print $2}' | xargs", cpu->cores, sizeof(cpu->cores));

    // Use dmidecode -t 7 to get cache sizes
    execute_command("dmidecode -t 7 | grep -A 12 'L1 - Cache' | grep -m 1 'Installed Size' | awk -F: '{print $2}' | xargs", cpu->l1_cache_size, sizeof(cpu->l1_cache_size));
    execute_command("dmidecode -t 7 | grep -A 12 'L2 - Cache' | grep -m 1 'Installed Size' | awk -F: '{print $2}' | xargs", cpu->l2_cache_size, sizeof(cpu->l2_cache_size));
    execute_command("dmidecode -t 7 | grep -A 12 'L3 - Cache' | grep -m 1 'Installed Size' | awk -F: '{print $2}' | xargs", cpu->l3_cache_size, sizeof(cpu->l3_cache_size));

    // Use /proc/cpuinfo to get current frequency
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        snprintf(cpu->current_frequency, sizeof(cpu->current_frequency), "N/A");
        return;
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strncmp(buffer, "cpu MHz", 7) == 0) {
            sscanf(buffer, "cpu MHz\t: %s", cpu->current_frequency);
            break;
        }
    }

    fclose(fp);
}

int main() {
    CPUInfo cpu = {0};
    get_cpu_info(&cpu);

    printf("Model Name:           %s\n", cpu.model_name);
    printf("Current Frequency:    %s MHz\n", strcmp(cpu.current_frequency, "") == 0 ? "N/A" : cpu.current_frequency);
    printf("Max Frequency:        %s\n", cpu.max_frequency);
    printf("Min Frequency:        %s MHz\n", cpu.min_frequency);
    printf("L1 Cache Size:        %s\n", cpu.l1_cache_size);
    printf("L2 Cache Size:        %s\n", cpu.l2_cache_size);
    printf("L3 Cache Size:        %s\n", cpu.l3_cache_size);
    printf("Cores:                %s\n", cpu.cores);
    printf("Threads:              %s\n", cpu.threads);
    printf("Architecture:         %s\n", cpu.architecture);
    printf("Voltage:              %s\n", cpu.voltage);
    printf("Serial Number:        %s\n", cpu.serial_number);

    return 0;
}