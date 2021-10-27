#!/bin/sh

# outputs #define commands for unknown mysql options

options="MYSQL_DEFAULT_AUTH
         MYSQL_INIT_COMMAND
         MYSQL_OPT_BIND
         MYSQL_OPT_COMPRESS
         MYSQL_OPT_CONNECT_TIMEOUT
         MYSQL_OPT_LOCAL_INFILE
         MYSQL_OPT_READ_TIMEOUT
         MYSQL_OPT_SSL_VERIFY_SERVER_CERT
         MYSQL_OPT_WRITE_TIMEOUT
         MYSQL_PLUGIN_DIR
         MYSQL_READ_DEFAULT_FILE
         MYSQL_READ_DEFAULT_GROUP
         MYSQL_SECURE_AUTH
         MYSQL_SET_CHARSET_DIR
         MYSQL_SET_CHARSET_NAME
"

for opt in $options; do
    if ! gcc -x c -S -o /dev/null - 2>/dev/null <<EOF
#include <mysql/mysql.h>
int opt = $opt;
EOF
    then
        echo \#define $opt -1
    fi
done
