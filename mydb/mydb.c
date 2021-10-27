#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "mydb.h"
#include "config.h"

/*{{{
 *  Macro to call mydb_err with filename and line number
 *
 *  Parameters
 *      db          pointer to mydb object
 *      err         mysql error number
 *      ...         format string and arguments
 */
#define MYDB_ERR(db, err, ...) mydb_err(db, err, __FILE__, __LINE__, __VA_ARGS__)
/*}}}*/
/*{{{
 *  malloc with error handling.
 *  assign pointer to count * size bytes to dst.
 *  if malloc fails, call MYDB_ERR and goto err
 *
 *  Parameters
 *      dst         where to store pointer to new memory
 *      count       number of items to allocate
 *      db          pointer to mydb object, in case of errors
 *      err         goto label for errors
 */
#define MYDB_MALLOC(dst, count, db, err) do {\
    (dst) = malloc((count) * sizeof *(dst)); \
    if(!(dst)) { \
        MYDB_ERR(db, CR_OUT_OF_MEMORY, "Could not malloc %zu bytes: Out of memory", (size_t)(count) * sizeof *(dst)); \
        goto err; \
    } \
} while(0) /*}}}*/

struct mydb_query {/*{{{*/
    mydb_query *next;
    mydb *db;
    MYSQL_STMT *stmt;
    mydb_blob query;

    unsigned long param_count;
    MYSQL_BIND *param_bind;

    unsigned long result_count;
    MYSQL_BIND *result_bind;
    mydb_query_col *result_columns;
};/*}}}*/
struct mydb_format {/*{{{*/
    char t;                     /* format character */
    enum enum_field_types ft;   /* MYSQL field type */
    int u;                      /* is unsigned?     */
};/*}}}*/

/*{{{
 *
 *  Write error message / number to mydb object
 *
 *  Parameters
 *      db          pointer to mydb object
 *      err         mysql error number
 *      file        file name of error
 *      line        line number of error
 *      fmt         printf format string
 *      ...         arguments for format
 */
static void mydb_err(mydb *db, int err, const char *file, int line, const char *fmt, ...)
{
    int n;

    db->mysql_last_errno = err;
    n = snprintf(db->mysql_last_error, MYSQL_ERRMSG_SIZE, "%s:%d ", file, line);
    if(n < MYSQL_ERRMSG_SIZE) {
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(db->mysql_last_error + n, MYSQL_ERRMSG_SIZE - n, fmt, ap);
        va_end(ap);
    }
}/*}}}*/
/*{{{
 *  Iterate through a 0 terminated mydb_format array and search for `t'
 *
 *  Parameters
 *      a           Array of mydb_format structs
 *      t           format character to search from
 *
 *  Returns
 *      NULL        not found
 *      pointer     found, pointer to entry
 */
static const struct mydb_format *mydb_getformat(const struct mydb_format *a, char t)
{
    while(a->t) {
        if(a->t == t) return a;
        ++a;
    }
    return NULL;
}/*}}}*/
/*{{{
 *  operation on MYSQL handle failed
 */
static void mydb_dberr(mydb *self)
{
    self->mysql_last_errno = mysql_errno(self->sql);
    snprintf(self->mysql_last_error, sizeof self->mysql_last_error, "%s", mysql_error(self->sql));
}/*}}}*/
/*{{{
 *  Operation on a prepared statement handle failed.
 *  Save error message to mydb object, close the failed prepstmt handle.
 *  Check if mysql connection is still alive; if not, close it and all other prepstmt handles.
 */
static void mydb_stmterr(mydb_query *self)
{
    mydb_query *p;
    self->db->mysql_last_errno = mysql_stmt_errno(self->stmt);
    snprintf(self->db->mysql_last_error, sizeof self->db->mysql_last_error, "%s", mysql_stmt_error(self->stmt));
    (void) mysql_stmt_close(self->stmt);
    self->stmt = NULL;
    if(!mysql_ping(self->db->sql)) return;

    /* sql connection failed. Close all statements, then close connection */
    for(p = self->db->queries; p; p = p->next) {
        if(p->stmt) {
            (void) mysql_stmt_close(p->stmt);
            p->stmt = NULL;
        }
    }
    (void) mysql_close(self->db->sql);
    self->db->sql = NULL;
}/*}}}*/
/*{{{
 *  Helper function for setting init commands in mydb_connect()
 *
 *  Parameters
 *      sql         MYSQL handle
 *      opt         mysql option type
 *      cmd         NULL terminated array of init commands
 *
 *  Returns
 *      0           success
 *      -1          error
 */
static inline int mydb_set_init_commands(MYSQL *sql, enum mysql_option opt, const char *const *cmd)
{
    while(*cmd)
        if(mysql_options(sql, opt, *cmd++)) return -1;
    return 0;
}/*}}}*/
/*{{{
 *  Create a new MYSQL handle, set options and initiate mysql connection
 *
 *  Returns
 *      0   success
 *      -1  fail
 */
static int mydb_connect(mydb_query *self)
{
    static const my_bool off = 0, on = 1;
    const char *e;

    self->db->sql = mysql_init(NULL);
    if(!self->db->sql) {
        MYDB_ERR(self->db, CR_OUT_OF_MEMORY, "Could not init mysql connection: Out of memory");
        return -1;
    }

#define MYOPT_STRG(opt, par) (self->db->par != 0 && (e = #opt, opt == -1 || mysql_options(self->db->sql, opt, self->db->par)))
#define MYOPT_BOOL(opt, par) (self->db->par >= 0 && (e = #opt, opt == -1 || mysql_options(self->db->sql, opt, (const char *)(self->db->par ? &on : &off))))
#define MYOPT_UINT(opt, par) (self->db->par >= 0 && (e = #opt, opt == -1 || mysql_options(self->db->sql, opt, (const char *)&self->db->par)))
#define MYOPT_EVRY(opt, par)                        (e = #opt,              mysql_options(self->db->sql, opt, (const char *)par))
#define MYOPT_INIT(opt, par) (self->db->par != 0 && (e = #opt, opt == -1 || mydb_set_init_commands(self->db->sql, opt, self->db->par)))

    /* short circuit if an option is requested and setting it fails; else run through all the options and eventually initiate connection */
    if( MYOPT_STRG(MYSQL_DEFAULT_AUTH, opt_default_auth)              ||
        MYOPT_INIT(MYSQL_INIT_COMMAND, opt_init_commands)             ||
        MYOPT_STRG(MYSQL_OPT_BIND, opt_bind)                          ||
        MYOPT_BOOL(MYSQL_OPT_COMPRESS, opt_compress)                  ||
        MYOPT_UINT(MYSQL_OPT_CONNECT_TIMEOUT, opt_connect_timeout)    ||
        MYOPT_BOOL(MYSQL_OPT_LOCAL_INFILE, opt_local_infile)          ||
        MYOPT_UINT(MYSQL_OPT_READ_TIMEOUT, opt_read_timeout)          ||
        MYOPT_BOOL(MYSQL_OPT_SSL_VERIFY_SERVER_CERT, opt_verify_cert) ||
        MYOPT_UINT(MYSQL_OPT_WRITE_TIMEOUT, opt_write_timeout)        ||
        MYOPT_STRG(MYSQL_PLUGIN_DIR, opt_plugin_dir)                  ||
        MYOPT_STRG(MYSQL_READ_DEFAULT_FILE, opt_default_file)         ||
        MYOPT_STRG(MYSQL_READ_DEFAULT_GROUP, opt_default_group)       ||
        MYOPT_BOOL(MYSQL_SECURE_AUTH, opt_secure_auth)                ||
        MYOPT_STRG(MYSQL_SET_CHARSET_DIR, opt_charset_dir)            ||
        MYOPT_STRG(MYSQL_SET_CHARSET_NAME, opt_charset_name)          ||
        MYOPT_EVRY(MYSQL_OPT_RECONNECT, &off)                         ||
        MYOPT_EVRY(MYSQL_REPORT_DATA_TRUNCATION, &on))
            MYDB_ERR(self->db, CR_UNKNOWN_ERROR, "Unknown option, %s", e);
    else if(!mysql_real_connect(self->db->sql, self->db->host, self->db->user, self->db->passwd, self->db->db,
                                  self->db->port, self->db->unix_socket, 0))
        mydb_dberr(self->db);
    else 
        return 0;

    /* something went wrong; clean up */
    
    (void) mysql_close(self->db->sql);
    self->db->sql = NULL;
    return -1;
}/*}}}*/
/*{{{
 *  create or reuse a MYSQL_STMT handle for a query
 */
static MYSQL_STMT *mydb_query_init(mydb_query *self)
{
    /* reuse existing handle if possible */
    if(self->stmt) return self->stmt;

    /* connect to database if neccessary */
    if(!self->db->sql)
        if(mydb_connect(self)) return NULL;

    /* get handle from mysql */
    self->stmt = mysql_stmt_init(self->db->sql);
    if(!self->stmt) {
        mydb_dberr(self->db);
        return NULL;
    }

    /* prepare the mysql statement */
    if(mysql_stmt_prepare(self->stmt, self->query.buf, self->query.length)) {
        mydb_stmterr(self);
        return NULL;
    }

    /* check if character count in the format strings matches the parameter count mysql gives us */
    if(mysql_stmt_param_count(self->stmt) != self->param_count) {
        MYDB_ERR(self->db, CR_UNKNOWN_ERROR, "Invalid param count: expected %lu but got %lu", self->param_count, mysql_stmt_param_count(self->stmt));
    } else if(mysql_stmt_field_count(self->stmt) != self->result_count) {
        MYDB_ERR(self->db, CR_UNKNOWN_ERROR, "Invalid result count: expected %lu but got %lu", self->result_count, mysql_stmt_field_count(self->stmt));
    } else {
        return self->stmt;
    }

    /* error. clean up */
    (void) mysql_stmt_close(self->stmt);
    self->stmt = NULL;
    return NULL;
}/*}}}*/
/*{{{
 *
 */
static void mydb_bind_param(mydb_query *self, va_list ap)
{
    static my_bool is_null = 1;
    void *p;
    unsigned long i;

#define VA_SGN(ap, type) self->param_bind[i].is_unsigned ? (void*)va_arg(ap, unsigned type) : (void*)va_arg(ap, signed type)

    for(i = 0; i < self->param_count; ++i) {
        switch(self->param_bind[i].buffer_type) {
            case MYSQL_TYPE_STRING:
                if(self->param_bind[i].is_unsigned) { /* '#' format */
                    p = va_arg(ap, char*);
                    self->param_bind[i].buffer_length = p ? strlen(p) : 0;
                    break;
                } /* else 's' format. Fall through to MYSQL_TYPE_BLOB. */

            case MYSQL_TYPE_BLOB: {
                mydb_blob *b = va_arg(ap, mydb_blob*);
                p = b ? b->buf : NULL;
                self->param_bind[i].buffer_length = b ? b->length : 0;
            } break;

            case MYSQL_TYPE_DATE     : p = va_arg(ap, MYSQL_TIME *); break;
            case MYSQL_TYPE_TIMESTAMP: p = va_arg(ap, MYSQL_TIME *); break;
            case MYSQL_TYPE_DATETIME : p = va_arg(ap, MYSQL_TIME *); break;
            case MYSQL_TYPE_TIME     : p = va_arg(ap, MYSQL_TIME *); break;
            case MYSQL_TYPE_TINY     : p = VA_SGN(ap, char       *); break;
            case MYSQL_TYPE_SHORT    : p = VA_SGN(ap, short      *); break;
            case MYSQL_TYPE_LONG     : p = VA_SGN(ap, int        *); break;
            case MYSQL_TYPE_LONGLONG : p = VA_SGN(ap, long long  *); break;
            case MYSQL_TYPE_FLOAT    : p = va_arg(ap, float      *); break;
            case MYSQL_TYPE_DOUBLE   : p = va_arg(ap, double     *); break;
            case MYSQL_TYPE_NULL     : p = va_arg(ap, void       *); break;
            default:
                /* not reachable */
                abort();
        }
        self->param_bind[i].is_null = p ? NULL : &is_null;
        self->param_bind[i].buffer = p;
    }
}/*}}}*/
/*{{{
 *  check if a buffer type is a string / blob type
 *
 *  Parameters
 *      buffer_type         type to check
 *
 *  Returns
 *      0   not a string type
 *      1   yes, it's a string type
 */
static int mydb_is_string(enum enum_field_types buffer_type)
{
    switch(buffer_type) {
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_BIT:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_STRING:
        case MYSQL_TYPE_VAR_STRING:
            return 1;
        default:
            return 0;
    }
}/*}}}*/
/*{{{
 *
 */
static int mydb_get_result(mydb_query *self, void(*result_cb)(void *arg, mydb_query_col col[]), void *arg)
{
    for(;;) {
        unsigned long i;

        /* for string types, bring data.b into a sane state and set buffer/buffer_length accordingly */
        for(i = 0; i < self->result_count; ++i) {
            if(mydb_is_string(self->result_bind[i].buffer_type)) {
                mydb_blob *b = &self->result_columns[i].data.b;
                if(!b->buf || !b->size) {
                    b->buf = malloc(1);
                    b->size = 1;
                    if(!b->buf) {
                        MYDB_ERR(self->db, CR_OUT_OF_MEMORY, "Could not malloc 1 byte:  Out of memory");
                        return -1;
                    }
                }
                self->result_bind[i].buffer_length = b->size - 1;
                self->result_bind[i].buffer = b->buf;
            }
        }

        /* bind results */
        if(mysql_stmt_bind_result(self->stmt, self->result_bind)) {
            mydb_stmterr(self);
            return -1;
        }

        /* fetch row */
        switch(mysql_stmt_fetch(self->stmt)) {
            case MYSQL_NO_DATA: /* we're finished */
                return 0;

            case 0: /* row fetched */
                break;

            default: /* something bad happened */
                mydb_stmterr(self);
                return -1;

            case MYSQL_DATA_TRUNCATED: { /* at least one column is truncated. Realloc if needed */
                for(i = 0; i < self->result_count; ++i) {
                    void *tmp;
                    mydb_blob *b = &self->result_columns[i].data.b;

                    if(!self->result_columns[i].error) continue;

                    tmp = realloc(b->buf, b->length + 1);
                    if(!tmp) {
                        MYDB_ERR(self->db, CR_OUT_OF_MEMORY, "Could not realloc %lu bytes: Out of memory", b->length + 1);
                        if(mysql_stmt_reset(self->stmt)) mydb_stmterr(self);
                        return -1;
                    }
                    b->size = b->length + 1;
                    b->buf = tmp;
                    self->result_bind[i].buffer_length = b->length;
                    self->result_bind[i].buffer = tmp;
                    if(mysql_stmt_fetch_column(self->stmt, self->result_bind + i, i, 0)) {
                        mydb_stmterr(self);
                        return -1;
                    }
                }
            } break;
        }
        /* null terminate all strings */
        for(i = 0; i < self->result_count; ++i) {
            if(!self->result_columns[i].is_null && mydb_is_string(self->result_bind[i].buffer_type)) {
                ((char*)self->result_columns[i].data.b.buf)[self->result_columns[i].data.b.length] = 0;
            }
        }
        result_cb(arg, self->result_columns);
    }
}/*}}}*/

/*{{{
 *  Create a new mydb object with default parameters
 */
mydb *mydb_new(void)
{
    static const mydb mydb_default = { NULL, NULL, NULL, -1, -1, -1, -1, -1, -1, NULL, NULL, "client", -1, NULL, "utf8",
                                       NULL, NULL, NULL, NULL, 0, NULL,
                                       0, "", NULL, NULL };
    mydb *self = malloc(sizeof *self);
    if(!self) return NULL;
    *self = mydb_default;
    return self;
}/*}}}*/
/*{{{
 *  Create a new query object
 *
 *  Parameters
 *      db          Pointer to mydb object
 *      query       string with the SQL query
 *      param_fmt   parameter types, may be null
 *      result_fmt  result types, may be null
 *
 *  Returns
 *      NULL        error
 *      *           pointer to query object
 */
mydb_query *mydb_query_new(mydb *db, const char *query, const char *param_fmt, const char *result_fmt)
{
    static const struct mydb_format param_types[] = {/*{{{*/
        { 'c', MYSQL_TYPE_TINY,      0 }, { 'C', MYSQL_TYPE_TINY,      1 },
        { 'h', MYSQL_TYPE_SHORT,     0 }, { 'H', MYSQL_TYPE_SHORT,     1 },
        { 'l', MYSQL_TYPE_LONG,      0 }, { 'u', MYSQL_TYPE_LONG,      1 },
        { 'L', MYSQL_TYPE_LONGLONG,  0 }, { 'U', MYSQL_TYPE_LONGLONG,  1 },
        { 'f', MYSQL_TYPE_FLOAT,     0 }, { 'd', MYSQL_TYPE_DOUBLE,    0 },
        { 't', MYSQL_TYPE_TIME,      0 }, { 'D', MYSQL_TYPE_DATE,      0 },
        { 'T', MYSQL_TYPE_DATETIME,  0 }, { 'S', MYSQL_TYPE_TIMESTAMP, 0 },
        { 's', MYSQL_TYPE_STRING,    0 }, { 'b', MYSQL_TYPE_BLOB,      0 },
        { '0', MYSQL_TYPE_NULL,      0 }, { '#', MYSQL_TYPE_STRING,    1 },
        { 0, 0, 0 }
    };/*}}}*/
    static const struct mydb_format result_types[] = {/*{{{*/
        { 'c', MYSQL_TYPE_TINY,        0 }, { 'C', MYSQL_TYPE_TINY,        1 },
        { 'h', MYSQL_TYPE_SHORT,       0 }, { 'H', MYSQL_TYPE_SHORT,       1 },
        { 'i', MYSQL_TYPE_INT24,       0 }, { 'I', MYSQL_TYPE_INT24,       1 },
        { 'l', MYSQL_TYPE_LONG,        0 }, { 'u', MYSQL_TYPE_LONG,        1 },
        { 'L', MYSQL_TYPE_LONGLONG,    0 }, { 'U', MYSQL_TYPE_LONGLONG,    1 },
        { 'f', MYSQL_TYPE_FLOAT,       0 }, { 'd', MYSQL_TYPE_DOUBLE,      0 },
        { 't', MYSQL_TYPE_TIME,        0 }, { 'D', MYSQL_TYPE_DATE,        0 },
        { 'S', MYSQL_TYPE_TIMESTAMP,   0 }, { 'T', MYSQL_TYPE_DATETIME,    0 },
        { 'o', MYSQL_TYPE_TINY_BLOB,   0 }, { 'b', MYSQL_TYPE_BLOB,        0 },
        { 'O', MYSQL_TYPE_MEDIUM_BLOB, 0 }, { 'B', MYSQL_TYPE_LONG_BLOB,   0 },
        { 'v', MYSQL_TYPE_VAR_STRING,  0 }, { 's', MYSQL_TYPE_STRING,      0 },
        { '.', MYSQL_TYPE_NEWDECIMAL,  0 }, { '2', MYSQL_TYPE_BIT,         0 },
        { 0, 0, 0 }
    };/*}}}*/

    mydb_query *self;
    unsigned long i;

    /* allocate new object and set all values except param / result stuff*/
    MYDB_MALLOC(self, 1, db, e0);
    self->next = db->queries;
    self->db = db;
    self->stmt = NULL;
    self->query.length = strlen(query);
    self->query.size = self->query.length + 1;
    self->query.buf = malloc(self->query.size);
    if(!self->query.buf) {
        MYDB_ERR(db, CR_OUT_OF_MEMORY, "Could not malloc %lu bytes: Out of memory", self->query.size);
        goto e1;
    }
    strcpy(self->query.buf, query);

    /* set param related object variables */
    self->param_count = param_fmt ? strlen(param_fmt) : 0;
    if(self->param_count) {
        MYDB_MALLOC(self->param_bind, self->param_count, db, e2);
    } else {
        self->param_bind = NULL;
    }
    memset(self->param_bind, 0, self->param_count * sizeof *self->param_bind);
    for(i = 0; i < self->param_count; ++i) {
        const struct mydb_format *p = mydb_getformat(param_types, param_fmt[i]);
        if(!p) {
            MYDB_ERR(db, CR_UNSUPPORTED_PARAM_TYPE, "Unsupported param type '%c'", param_fmt[i]);
            goto e3;
        }
        self->param_bind[i].buffer_type = p->ft;
        self->param_bind[i].is_unsigned = p->u;
    }

    /* set result related object variables */
    self->result_count = result_fmt ? strlen(result_fmt) : 0;
    if(self->result_count) {
        MYDB_MALLOC(self->result_bind, self->result_count, db, e3);
        MYDB_MALLOC(self->result_columns, self->result_count, db, e4);
    } else {
        self->result_bind = NULL;
        self->result_columns = NULL;
    }
    memset(self->result_bind, 0, self->result_count * sizeof *self->result_bind);
    for(i = 0; i < self->result_count; ++i) {
        const struct mydb_format *p = mydb_getformat(result_types, result_fmt[i]);
        if(!p) {
            MYDB_ERR(db, CR_UNSUPPORTED_PARAM_TYPE, "Unsupported result type '%c'", result_fmt[i]);
            goto e5;
        }
        self->result_columns[i].data.b.size = 0;
        self->result_columns[i].data.b.buf  = NULL;

        self->result_bind[i].buffer_type = p->ft;
        self->result_bind[i].is_unsigned = p->u;
        self->result_bind[i].buffer      = &self->result_columns[i].data;
        self->result_bind[i].length      = mydb_is_string(p->ft) ? &self->result_columns[i].data.b.length : NULL;
        self->result_bind[i].is_null     = &self->result_columns[i].is_null;
        self->result_bind[i].error       = &self->result_columns[i].error;
    }

    /* add new object to list of queries */
    self->db->queries = self;

    return self;

    /* in case of error, clean up */
e5: free(self->result_columns);
e4: free(self->result_bind);
e3: free(self->param_bind);
e2: free(self->query.buf);
e1: free(self);
e0: return NULL;
}/*}}}*/
/*{{{
 *  Delete a query object
 *
 *  Parameters
 *      self        pointer to query object
 */
void mydb_query_del(mydb_query *self)
{
    unsigned long i;
    mydb_query **p;

    /* first, remove query from list */
    for(p = &self->db->queries; *p; p = &(*p)->next) {
        if(*p == self) break;
    }
    if(!*p) abort();
    *p = self->next;

    /* close statement */
    if(self->stmt)
        (void) mysql_stmt_close(self->stmt);

    /* free query string */
    free(self->query.buf);

    /* free result columns */
    for(i = 0; i < self->result_count; ++i) {
        if(mydb_is_string(self->result_bind[i].buffer_type) &&
                               self->result_columns[i].data.b.buf &&
                               self->result_columns[i].data.b.size) free(self->result_columns[i].data.b.buf);
    }
    free(self->result_columns);

    /* free bind structs */
    free(self->param_bind);
    free(self->result_bind);

    /* free self */
    free(self);
}/*}}}*/
/*{{{
 *  Free all resources occupied by mydb object, including queries
 *
 *  Parameters
 *      self        pointer to query object
 */
void mydb_del(mydb *self)
{
    /* free all queries */
    while(self->queries) mydb_query_del(self->queries);

    /* disconnect from database */
    if(self->sql) (void) mysql_close(self->sql);

    /* free self */
    free(self);
}/*}}}*/
/*{{{
 *  Variadic interface for mydb_query_vexecute
 */
int mydb_query_execute(mydb_query *self, my_ulonglong *id, my_ulonglong *affected, void(*result_cb)(void *arg, mydb_query_col col[]), void *arg, ...)
{
    int ret;
    va_list ap;
    va_start(ap, arg);
    ret = mydb_query_vexecute(self, id, affected, result_cb, arg, ap);
    va_end(ap);
    return ret;
}/*}}}*/
/*{{{
 *  Execute a query
 *
 *  Parameters
 *      self        pointer to query object
 *      id          if not null, the automatic column id is stored here
 *      affected    if not null, number of affected rows is stored here
 *      result_cb   callback to call for each result row
 *                  arg     user supplied argument
 *                  col     array of result columns
 *      arg         user supplied argument
 *      ap          data for parameter markers
 *
 *  Returns
 *      -1          error
 *      0           success
 */
int mydb_query_vexecute(mydb_query *self, my_ulonglong *id, my_ulonglong *affected, void(*result_cb)(void *arg, mydb_query_col col[]), void *arg, va_list ap)
{
    /* get the prepared statement handle */
    MYSQL_STMT *stmt = mydb_query_init(self);
    if(!stmt) return -1;

    /* bind parameters */
    if(self->param_count) {
        mydb_bind_param(self, ap);
        if(mysql_stmt_bind_param(stmt, self->param_bind)) {
            mydb_stmterr(self);
            return -1;
        }
    }

    /* execute query */
    if(mysql_stmt_execute(stmt)) {
        mydb_stmterr(self);
        return -1;
    }

    /* retrieve LAST_INSERT_ID and number of affected rows */
    if(id) *id = mysql_stmt_insert_id(stmt);
    if(affected) *affected = mysql_stmt_affected_rows(stmt);

    /* fetch results */
    if(self->result_count && mydb_get_result(self, result_cb, arg)) return -1;

    /* FIXME do we need mysql_stmt_reset? */

    return 0;
}/*}}}*/
