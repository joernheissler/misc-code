#include <stdio.h>
#include <stdlib.h>
#include <mydb.h>

static const struct querystring {
    const char *query;
    const char *input;
    const char *output;
} querystrings[] = {
    { "SELECT id, name, info, pi FROM example WHERE name = ? OR pi != ?", "#d", "ussd" },
    { "INSERT INTO example SET name=?, info=?, pi=?", "#sd", NULL }
};

enum {
    query_select,
    query_insert
};

static void cb(void *arg, mydb_query_col col[])
{
    (void) arg;
    printf("id = %u\n", col[0].data.ui);
    printf("name = %s\n", (char*)col[1].data.b.buf);
    printf("info = %s\n", col[2].is_null ? "(null)" : (char*)col[2].data.b.buf);
    printf("pi = %f\n", col[3].data.d);
    puts("------------------------------");
}

int main(void)
{
    /* create new mydb object */
    mydb *db = mydb_new();
    if(!db) {
        fprintf(stderr, "mydb_new: out of memory\n");
        abort();
    }

    mydb_query *query[sizeof querystrings / sizeof *querystrings];

    /* create the mydb_query objects */
    for(size_t i = 0; i < sizeof querystrings / sizeof *querystrings; ++i) {
        if(!(query[i] = mydb_query_new(db, querystrings[i].query, querystrings[i].input, querystrings[i].output))) {
            fprintf(stderr, "mydb_query_new: %s\n", db->mysql_last_error);
            mydb_del(db);
            abort();
        }
    }

    /* Insert something into the table */
    my_ulonglong id;
    double pi = 2.72;
    mydb_blob info = { 5, 5, (char *)"hello" };
    if(mydb_query_execute(query[query_insert], &id, NULL, NULL, NULL, "your name", &info, &pi)) {
        fprintf(stderr, "insert: %s\n", db->mysql_last_error);
        mydb_del(db);
        abort();
    }
    printf("new id = %lu\n", (unsigned long) id);
    puts("");

    /* Query database and call cb() for each result row */
    if(mydb_query_execute(query[query_select], NULL, NULL, cb, NULL, "name", 2.72)) {
        fprintf(stderr, "select: %s\n", db->mysql_last_error);
        mydb_del(db);
        abort();
    }

    /* clean up */
    mydb_del(db);
    return 0;
}
