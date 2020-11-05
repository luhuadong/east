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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "jsonrpc-c.h"


#define PORT 6574  // the port users will be connecting to

struct jrpc_server my_server;

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

cJSON * exit_server(jrpc_context * ctx, cJSON * params, cJSON *id)
{
    jrpc_server_stop(&my_server);
    return cJSON_CreateString("Bye!");
}

int main(void)
{
    printf("east tool ~('_')=====b\n");
    
    jrpc_server_init(&my_server, PORT);
    jrpc_register_procedure(&my_server, say_hello, "sayHello", NULL );
    jrpc_register_procedure(&my_server, add, "add", NULL );
    jrpc_register_procedure(&my_server, exit_server, "exit", NULL );
    jrpc_server_run(&my_server);
    jrpc_server_destroy(&my_server);
    return 0;
}
