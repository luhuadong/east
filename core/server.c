/*
 * Copyright (c) 2020, Rudy Lo <luhuadong@163.com>
 *
 * Licensed under GPLv2 or later.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-05     luhuadong    first version
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <openssl/md5.h>
#include "jsonrpc-c.h"

static int debug = 1; /* enable this to printf */
#define DEBUG_PRINT(fmt, args...) \
    do { if(debug) \
    printf(fmt, ## args); \
    } while(0)

#define VERSION               "0.0.1"
#define PORT                  6574  // the port users will be connecting to

#define PROC_BUFF             8192
#define CMD_BUFF              16384

#define LOOKUP_TABLE_COUNT    32
#define COMMAND(name)         name##_main
#define LOCK(name)            name##_lock

enum {
    CMD_TYPE_NONE,
    CMD_TYPE_SYS,
    CMD_TYPE_PROC,
    CMD_TYPE_BUILTIN,
    CMD_TYPE_PERF,
    CMD_TYPE_MAX,
};

struct jrpc_server my_server;
unsigned char *endstring = "eastendstring";

typedef int (*builtin_func)(int argc, char **argv, int fd);

typedef struct
{
    char* name;
    int type;
    pthread_mutex_t* lock;
    builtin_func func;

} builtin_func_info;

static pthread_mutex_t proc_lock;

static builtin_func_info lookup_table[LOOKUP_TABLE_COUNT] = {
    {
        .name = "proc",
        .type = CMD_TYPE_PROC,
        .func = NULL,
        .lock = &LOCK(proc),
    },
    {
        .name = NULL,
        .func = NULL,
    },
};


cJSON * say_hello(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    return cJSON_CreateString("Hello!");
}

cJSON * add(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    cJSON * a = cJSON_GetArrayItem(params,0);
    cJSON * b = cJSON_GetArrayItem(params,1);
    return cJSON_CreateNumber(a->valueint + b->valueint);
}

/*
 * params:
 *  - url
 *  - md5sum
 *  - save_path
 */
cJSON * download(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    cJSON * url = cJSON_GetArrayItem(params,0);
    cJSON * md5 = cJSON_GetArrayItem(params,1);
    cJSON * path = cJSON_GetArrayItem(params,2);

    DEBUG_PRINT("URL: %s\n", url->valuestring);
    DEBUG_PRINT("md5sum: %s\n", md5->valuestring);
    DEBUG_PRINT("save path: %s\n", path->valuestring);

    size_t len = strlen(url->valuestring);
    char cmd[len + 10];
    snprintf(cmd, sizeof(cmd), "wget %s", url->valuestring);
    system(cmd);

    return cJSON_CreateString("Ok");
}

builtin_func_info* lookup_func(char* name)
{
    /*int i = 0;
    for( ; i < LOOKUP_TABLE_COUNT; i++){
        if(lookup_table[i].name == NULL)
            return NULL;
        if(!strcmp(name, lookup_table[i].name))
            return &lookup_table[i];
    }*/
    builtin_func_info* p = lookup_table;
    while(p->name != NULL){
        if(!strcmp(name, p->name))
            return p;
        p++;
    }
    return NULL;
}

cJSON * read_proc(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    int fd;
    int size;
    cJSON *result;
    unsigned char proc_buff[PROC_BUFF];
    unsigned char proc_path[50];

    if (!ctx->data)
        return NULL;

    builtin_func_info* info = lookup_func("proc");
    snprintf(proc_path, 50, "/proc/%s", (char *)ctx->data);

    pthread_mutex_lock(info->lock);
    DEBUG_PRINT("read_proc: path: %s\n", proc_path);
    fd = open(proc_path, O_RDONLY);
    if (fd < 0){
        DEBUG_PRINT("Open file:%s error.\n", proc_path);
        pthread_mutex_unlock(info->lock);
        return NULL;
    }

    memset(proc_buff, 0, PROC_BUFF);
    size = read(fd, proc_buff, PROC_BUFF);
    close(fd);
    DEBUG_PRINT("read %d bytes from %s\n", size, proc_path);
    strcat(proc_buff, endstring);
    pthread_mutex_unlock(info->lock);
    return cJSON_CreateString(proc_buff);
}

cJSON * list_all(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    int i;
    unsigned char proc_buff[CMD_BUFF];
    memset(proc_buff, 0, PROC_BUFF);

    for (i = 0; i < my_server.procedure_count; i++)
    {
        strcat(proc_buff, my_server.procedures[i].name);
        strcat(proc_buff, " ");
    }
    strcat(proc_buff, endstring);

    return cJSON_CreateString(proc_buff);
}

cJSON * exit_server(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    jrpc_server_stop(&my_server);
    return cJSON_CreateString("Bye!");
}

int main(void)
{
    printf("east tool v%s  ~('_')=====b\n", VERSION);
    printf("port [%d]\n", PORT);
    
    jrpc_server_init(&my_server, PORT);

    jrpc_register_procedure(&my_server, say_hello, "sayHello", NULL );
    jrpc_register_procedure(&my_server, list_all,  "listAllMethod", NULL);
    jrpc_register_procedure(&my_server, read_proc, "getProcVersion", "version");
    jrpc_register_procedure(&my_server, add, "add", NULL );
    jrpc_register_procedure(&my_server, download, "download", NULL );
    jrpc_register_procedure(&my_server, exit_server, "exit", NULL );

    jrpc_server_run(&my_server);
    jrpc_server_destroy(&my_server);

    return 0;
}
