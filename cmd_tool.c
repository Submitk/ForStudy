#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

// 日志文件名称
#define LOG_FILE "execution.log"

// 获取当前时间字符串
void get_current_time(char* buffer, size_t length) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, length, "%Y-%m-%d %H:%M:%S", t);
}

// 记录日志（带时间戳）
void log_time(FILE* log_file, const char* event) {
    char time_str[20];
    get_current_time(time_str, sizeof(time_str));
    fprintf(log_file, "%s - %s\n", time_str, event);
    printf("%s - %s\n", time_str, event);
}

// 记录日志（不带时间戳）
void log_message(FILE* log_file, const char* message) {
    fprintf(log_file, "%s", message);
    printf("%s", message);
}

// 执行命令并记录日志
int execute_command(const char* command) {
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return EXIT_FAILURE;
    }

    log_time(log_file, "start");
    log_message(log_file, command);
    log_message(log_file, "\n");

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        log_message(log_file, "Failed to run command\n");
        perror("popen");
        fclose(log_file);
        return EXIT_FAILURE;
    }

    char output[1024];
    while (fgets(output, sizeof(output), fp) != NULL) {
        log_message(log_file, output);
    }

    int exit_code = pclose(fp);
    if (exit_code == -1) {
        log_message(log_file, "Failed to close command stream\n");
        perror("pclose");
        fclose(log_file);
        return EXIT_FAILURE;
    } else if (WIFEXITED(exit_code) && WEXITSTATUS(exit_code) != 0) {
        char error_message[100];
        snprintf(error_message, sizeof(error_message), "Command failed with exit code %d\n", WEXITSTATUS(exit_code));
        log_message(log_file, error_message);
    }

    log_time(log_file, "end");

    fclose(log_file);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 连接所有的命令参数为一个字符串
    int command_len = 0;
    for (int i = 1; i < argc; ++i) {
        command_len += strlen(argv[i]) + 1;
    }

    char* command = (char*)malloc(command_len);
    if (command == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(command, argv[1]);
    for (int i = 2; i < argc; ++i) {
        strcat(command, " ");
        strcat(command, argv[i]);
    }

    int exit_code = execute_command(command);
    free(command);

    return exit_code;
}