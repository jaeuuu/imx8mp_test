#ifndef DB_H__
#define DB_H__

#include <stdio.h>
#include <string.h>

/* MAX 2048 */
#define MAX_SQL_SIZE    2048

#define SQL_CREATE(str, table, columns)             sprintf(str, "CREATE TABLE %s (%s);", table, columns)
#define SQL_READ(str, table, column)                sprintf(str, "SELECT %s FROM %s;", column, table)
#define SQL_UPDATE(str, table, column, data)        sprintf(str, "UPDATE %s SET %s=\'%s\';", table, column, data)
#define SQL_INSERT(str, table, columns_data)        sprintf(str, "INSERT INTO %s VALUES (%s);", table, columns_data)
#define SQL_DELETE(table, column)

int open_db(sqlite3 **db, const char *filename);
void close_db(sqlite3 *db);
int exec_db(sqlite3 *db, const char *sql,
    int (*func)(void *, int, char **, char **),
    void *data);

#endif
