#include <sqlite3.h>
#include <api/debug.h>

int open_db(sqlite3 **db, const char *filename)
{
    int ret;

    ret = sqlite3_open(filename, db);
    if (ret != SQLITE_OK) {
        dlp_err("sqlite3_open() fail (%s)", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return -1;
    }
    return 0;
}

void close_db(sqlite3 *db)
{
    sqlite3_close(db);
}

int exec_db(sqlite3 *db, const char *sql,
    int (*func)(void *, int, char **, char **),
    void *data)
{
    char *err = NULL;

    dlp_dbg("sql: [%s]", sql);

    if (sqlite3_exec(db, sql, func, data, &err) != SQLITE_OK) {
        dlp_err("%s", err);
        return -1;
    }
    sqlite3_free(err);
}