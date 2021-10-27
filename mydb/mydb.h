#ifndef MYDB_H
#define MYDB_H

#include <stdarg.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>

typedef struct mydb_query mydb_query;

typedef struct mydb {
    /* mysql options to set:                  option                            default
    --------------------------------------    --------------------------------  -------- */
    const char        *opt_default_auth;   /* MYSQL_DEFAULT_AUTH                NULL     */
    const char *const *opt_init_commands;  /* MYSQL_INIT_COMMAND                NULL     */
    const char        *opt_bind;           /* MYSQL_OPT_BIND                    NULL     */
    int                opt_compress;       /* MYSQL_OPT_COMPRESS                -1       */
    int                opt_connect_timeout;/* MYSQL_OPT_CONNECT_TIMEOUT         -1       */
    int                opt_local_infile;   /* MYSQL_OPT_LOCAL_INFILE            -1       */
    int                opt_read_timeout;   /* MYSQL_OPT_READ_TIMEOUT            -1       */
    int                opt_verify_cert;    /* MYSQL_OPT_SSL_VERIFY_SERVER_CERT  -1       */
    int                opt_write_timeout;  /* MYSQL_OPT_WRITE_TIMEOUT           -1       */
    const char        *opt_plugin_dir;     /* MYSQL_PLUGIN_DIR                  NULL     */
    const char        *opt_default_file;   /* MYSQL_READ_DEFAULT_FILE           NULL     */
    const char        *opt_default_group;  /* MYSQL_READ_DEFAULT_GROUP          "client" */
    int                opt_secure_auth;    /* MYSQL_SECURE_AUTH                 -1       */
    const char        *opt_charset_dir;    /* MYSQL_SET_CHARSET_DIR             NULL     */
    const char        *opt_charset_name;   /* MYSQL_SET_CHARSET_NAME            "utf8"   */

    /* connection parameters. Default (NULL / 0) may work if ~/.my.cnf is configured */
    const char *host;
    const char *user;
    const char *passwd;
    const char *db;
    unsigned    port;
    const char *unix_socket;

    /* Server error code and message */
    unsigned mysql_last_errno;
    char     mysql_last_error[MYSQL_ERRMSG_SIZE];

    /* MYSQL handle. Don't modify! */
    MYSQL *sql;

    /* list of queries. Leave it alone! */
    mydb_query *queries;
} mydb;

/* represents a binary blob which may contain arbitrary data including null bytes
 * buf points to allocated storage iff (buf && size)
 * (for input parameters to mydb_query_execute, buf needs no be allocated storage and size is ignored.)
 */
typedef struct mydb_blob {
    unsigned long length; /* length of the contents of buf, in bytes */
    unsigned long size;   /* size of the allocation of buf */
    void *buf;            /* malloc'ed buffer */
} mydb_blob;

typedef union mydb_query_data {
    mydb_blob          b;
    double             d;
    float              f;
    MYSQL_TIME         t;
    signed char        sc;
    unsigned char      uc;
    signed short       ss;
    unsigned short     us;
    signed int         si;
    unsigned int       ui;
    signed long long   sll;
    unsigned long long ull;
} mydb_query_data;

typedef struct mydb_query_col {
    mydb_query_data data;
    my_bool is_null;
    my_bool error;
} mydb_query_col;

mydb *mydb_new(void);
void mydb_del(mydb *self);

mydb_query *mydb_query_new(mydb *db, const char *query, const char *param_fmt, const char *result_fmt);
void mydb_query_del(mydb_query *self);
int mydb_query_execute(mydb_query *self, my_ulonglong *id, my_ulonglong *affected, void(*result_cb)(void *arg, mydb_query_col col[]), void *arg, ...);
int mydb_query_vexecute(mydb_query *self, my_ulonglong *id, my_ulonglong *affected, void(*result_cb)(void *arg, mydb_query_col col[]), void *arg, va_list ap);

#define NULL_STRING (char*)0
#define NULL_BLOB (mydb_blob*)0
#define NULL_TIME (MYSQL_TIME*)0
#define NULL_STINY (signed char*)0
#define NULL_UTINY (unsigned char*)0
#define NULL_SSHORT (signed short*)0
#define NULL_USHORT (unsigned short*)0
#define NULL_SLONG (signed int*)0
#define NULL_ULONG (unsigned int*)0
#define NULL_SLLONG (signed long long*)0
#define NULL_ULLONG (unsigned long long*)0
#define NULL_FLOAT (float*)0
#define NULL_DOUBLE (double*)0
#define NULL_NULL (void*)0

#endif
