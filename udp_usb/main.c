/*
 * Copyright (C) 2021 Ioannis Chatzigiannakis
 *
 * This file is subject to the terms and conditions of the MIT License.
 * See the file LICENSE in the top level directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief       Create a simple UDP server.
 *
 * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
 *
 * @}
 */
#include <stdio.h>

#include "net/sock/udp.h"
#include "net/ipv6/addr.h"
#include "thread.h"

#include "msg.h"

#include "xtimer.h"

#define SERVER_MSG_QUEUE_SIZE   (8)
#define SERVER_BUFFER_SIZE      (64)

static sock_udp_t sock;
static char server_buffer[SERVER_BUFFER_SIZE];
static char server_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];

/**
 * A simple UDP server based on the Tutoriasl/Task-06.
 * @see https://github.com/RIOT-OS/Tutorials/tree/master/task-06
 */
void *udp_server(void *args)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(server_msg_queue, SERVER_MSG_QUEUE_SIZE);

    sock_udp_ep_t server = { .port = atoi(args), .family = AF_INET6 };
    if (sock_udp_create(&sock, &server, NULL, 0) < 0) {
        return NULL;
    }

    printf("Success: started UDP server on port %u\n", server.port);

    while (1) {
        int res;

        if ((res = sock_udp_recv(&sock, server_buffer,
                                 sizeof(server_buffer) - 1, SOCK_NO_TIMEOUT,
                                 NULL)) < 0) {
            puts("Error while receiving");
        }
        else if (res == 0) {
            puts("No data received");
        }
        else {
            server_buffer[res] = '\0';
            printf("Recvd: %s\n", server_buffer);
        }
    }

    return NULL;
}

int main(void)
{
    printf("RIOT UDP server application\n");
    if (thread_create(server_stack,
                      sizeof(server_stack),
                      THREAD_PRIORITY_MAIN - 1,
                      THREAD_CREATE_STACKTEST,
                      udp_server,
                      "8888",
                      "UDP Server") <= KERNEL_PID_UNDEF) {
        return -1;
    }

    while (1) {
        xtimer_sleep(5);
    }

    return 0;
}
