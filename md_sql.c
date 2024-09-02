#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>    // 添加此行以包含 isspace 函数
#include <sqlite3.h>

// 用于执行SQL查询并将结果写入INI文件，同时打印输出到控制台
void execute_query(sqlite3 *db, const char *sql, FILE *file) {
    char *errmsg = NULL;
    int rc;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot execute query: %s\n", sqlite3_errmsg(db));
        return;
    }

    int col_count = sqlite3_column_count(stmt);

    int row_id = 1;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        fprintf(file, "[Row%d]\n", row_id++);
        for (int col = 0; col < col_count; col++) {
            const char *col_name = sqlite3_column_name(stmt, col);
            const char *col_value = (const char*)sqlite3_column_text(stmt, col);
            fprintf(file, "%s = %s\n", col_name, col_value ? col_value : "NULL");
            // 同步打印到控制台
            printf("%s = %s\n", col_name, col_value ? col_value : "NULL");
        }
        fprintf(file, "\n");
        printf("\n");
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution finished with an error: %s\n", sqlite3_errmsg(db));
    }
}

// 从数据库中获取指定条件的数据并存入缓冲区
int fetch_existing_data(sqlite3 *db, const char *table, const char *condition, char *output) {
    char sql[1000];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s;", table, condition);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot execute query: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    int col_count = sqlite3_column_count(stmt);
    int has_data = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;
        for (int col = 0; col < col_count; col++) {
            const char *col_name = sqlite3_column_name(stmt, col);
            const char *col_value = (const char*)sqlite3_column_text(stmt, col);
            sprintf(output + strlen(output), "%s = %s\n", col_name, col_value ? col_value : "NULL");
        }
    }

    sqlite3_finalize(stmt);
    return has_data;
}

// 去除字符串两端多余的空格
char* trim_whitespace(char *str) {
    char *end;

    // 去除前导空格
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // 全是空格
        return str;

    // 去除尾部空格
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // 在终止位置加上空字符
    *(end + 1) = '\0';
    return str;
}

// 从INI文件导入数据到数据库并比较修改前后值
void import_ini_to_db(sqlite3 *db, const char *table, const char *ini_file) {
    FILE *file = fopen(ini_file, "r");
    if (!file) {
        fprintf(stderr, "Cannot open INI file for reading.\n");
        return;
    }

    char line[256];
    char sql[1000];
    char current_section[50] = "";
    char key[50], value[200];
    char existing_data[2000] = "";
    char condition[500] = "";
    int needs_update = 0;
    int rc;  // 声明 rc 变量

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "[%49[^]]]", current_section) == 1) {
            snprintf(sql, sizeof(sql), "INSERT OR REPLACE INTO %s (", table);
        } else if (sscanf(line, "%49[^=]=%199[^\n]", key, value) == 2) {
            strcat(sql, trim_whitespace(key));
            strcat(sql, ", ");
            if (strcmp(trim_whitespace(key), "Barcode") == 0) {
                snprintf(condition, sizeof(condition), "Barcode='%s'", trim_whitespace(value));
            }
        } else if (strlen(line) == 1) {
            sql[strlen(sql) - 2] = ')';
            strcat(sql, " VALUES (");

            fseek(file, 0, SEEK_SET);
            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, current_section)) {
                    while (fgets(line, sizeof(line), file) && strlen(line) > 1) {
                        sscanf(line, "%49[^=]=%199[^\n]", key, value);
                        strcat(sql, "'");
                        strcat(sql, trim_whitespace(value));
                        strcat(sql, "', ");
                    }
                    break;
                }
            }

            sql[strlen(sql) - 2] = ')';
            strcat(sql, ";");

            // Fetch existing data for comparison
            if (fetch_existing_data(db, table, condition, existing_data)) {
                printf("Existing Data:\n%s\n", existing_data);
            }

            // Check if the new data is different from existing data
            int data_changed = strcmp(existing_data, sql) != 0;
            if (data_changed) {
                printf("New Data: %s\n", sql);

                char *errmsg;
                rc = sqlite3_exec(db, sql, 0, 0, &errmsg);
                if (rc == SQLITE_OK) {
                    printf("Update successful.\n");
                } else {
                    fprintf(stderr, "SQL error: %s\n", errmsg);
                    sqlite3_free(errmsg);
                }
            } else {
                printf("Data unchanged, no updates.\n");
            }

            memset(current_section, 0, sizeof(current_section));
            memset(existing_data, 0, sizeof(existing_data));
        }
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <database> <table> [condition] [columns or ini_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* database = argv[1];
    const char* table = argv[2];

    sqlite3 *db;
    int rc = sqlite3_open(database, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    if (argc == 4 && strstr(argv[3], ".ini")) {
        // 指定的INI文件名，导入数据
        import_ini_to_db(db, table, argv[3]);
    } else if (argc >= 4) {
        // 执行查询并导出到指定INI文件，同时打印结果
        const char* condition = argv[3];
        const char* columns = argc > 4 ? argv[4] : "*";
        const char* ini_file = argc > 5 ? argv[5] : "output.ini";

        char sql[1000];
        snprintf(sql, sizeof(sql), "SELECT %s FROM %s WHERE %s;", columns, table, condition);

        FILE *file = fopen(ini_file, "w");
        if (!file) {
            fprintf(stderr, "Cannot open INI file for writing.\n");
            sqlite3_close(db);
            return EXIT_FAILURE;
        }

        printf("Query Result:\n");
        execute_query(db, sql, file);
        fclose(file);
        printf("Data exported to %s.\n", ini_file);
    } else {
        fprintf(stderr, "Invalid usage. Provide a condition and optionally columns and an ini file.\n");
    }

    sqlite3_close(db);
    return EXIT_SUCCESS;
}