#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

void execute_query(sqlite3 *db, const char *sql) {
    char *errmsg = NULL;
    int rc;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot execute query: %s\n", sqlite3_errmsg(db));
        return;
    }

    // 获取列数
    int col_count = sqlite3_column_count(stmt);

    // 执行查询并打印结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int col = 0; col < col_count; col++) {
            const char *col_name = sqlite3_column_name(stmt, col);
            const char *col_value = (const char*)sqlite3_column_text(stmt, col);
            printf("%s = %s\n", col_name, col_value ? col_value : "NULL");
        }
        printf("-----\n"); // 分隔每条记录
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution finished with an error: %s\n", sqlite3_errmsg(db));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <database> <table> <columns> [condition]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* database = argv[1];
    const char* table = argv[2];
    const char* columns = argv[3];
    const char* condition = argc > 4 ? argv[4] : "1=1"; // 如果没有提供条件，默认为 1=1

    sqlite3 *db;
    int rc = sqlite3_open(database, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return EXIT_FAILURE;
    }

    // 构造SQL查询字符串
    char sql[1000];
    snprintf(sql, sizeof(sql), "SELECT %s FROM %s WHERE %s;", columns, table, condition);

    // 执行查询
    execute_query(db, sql);

    sqlite3_close(db);
    return EXIT_SUCCESS;
}