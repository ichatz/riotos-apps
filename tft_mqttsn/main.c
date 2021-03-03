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
  * @brief       Display messages received from MQTT channel to TFT display.
  *
  * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
  *
  * @}
  */

#include <stdio.h>
#include "xtimer.h"
#include "board.h"

#include "periph/gpio.h"
#include "periph/spi.h"

#include "ucg.h"
#include "ucg_riotos.h"
#include "logo.h"

ucg_t ucg;
int ucg_pos_y;
int ucg_font_height;
int ucg_max_height;
int ucg_max_width_chars;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#ifndef EMCUTE_ID
#define EMCUTE_ID           ("gertrud")
#endif
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)

static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];

static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(CONFIG_EMCUTE_DEFAULT_PORT, EMCUTE_ID);
    return NULL;    /* should never be reached */
}

static void on_pub(const emcute_topic_t *topic, void *data, size_t len)
{
    char *in = (char *)data;

    printf("### got publication for topic '%s' [%i] ###\n",
           topic->name, (int)topic->id);
    for (size_t i = 0; i < len; i++) {
        printf("%c", in[i]);
    }
    puts("");

    // Might need multiple lines to draw on TFT screen
    int last_pos = 0;
    int len_remaining = len;
    while(len_remaining % ucg_max_width_chars > 0) {
      // Clear screen if we reached maximum height
      if (ucg_pos_y > ucg_max_height) {
        ucg_ClearScreen(&ucg);
        ucg_pos_y = ucg_font_height;
      }

      // Get substring based on maximum chars that fit on TFT screen
      char subbuff[ucg_max_width_chars + 1];
      memcpy( subbuff, &in[last_pos], ucg_max_width_chars);
      subbuff[ucg_max_width_chars] = '\0';
      len_remaining -= ucg_max_width_chars;
      last_pos += ucg_max_width_chars;

      ucg_DrawString(&ucg, 0, ucg_pos_y, 0, subbuff);
      ucg_pos_y += ucg_font_height;
    }
  }


static unsigned get_qos(const char *str)
{
    int qos = atoi(str);
    switch (qos) {
        case 1:     return EMCUTE_QOS_1;
        case 2:     return EMCUTE_QOS_2;
        default:    return EMCUTE_QOS_0;
    }
}

static int cmd_con(int argc, char **argv)
{
    sock_udp_ep_t gw = { .family = AF_INET6, .port = CONFIG_EMCUTE_DEFAULT_PORT };
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;

    if (argc < 2) {
        printf("usage: %s <ipv6 addr> [port] [<will topic> <will message>]\n",
                argv[0]);
        return 1;
    }

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, argv[1]) == NULL) {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    if (argc >= 3) {
        gw.port = atoi(argv[2]);
    }
    if (argc >= 5) {
        topic = argv[3];
        message = argv[4];
        len = strlen(message);
    }

    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", argv[1], (int)gw.port);
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n",
           argv[1], (int)gw.port);

    return 0;
}

static int cmd_discon(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    int res = emcute_discon();
    if (res == EMCUTE_NOGW) {
        puts("error: not connected to any broker");
        return 1;
    }
    else if (res != EMCUTE_OK) {
        puts("error: unable to disconnect");
        return 1;
    }
    puts("Disconnect successful");
    return 0;
}

static int cmd_pub(int argc, char **argv)
{
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;

    if (argc < 3) {
        printf("usage: %s <topic name> <data> [QoS level]\n", argv[0]);
        return 1;
    }

    /* parse QoS level */
    if (argc >= 4) {
        flags |= get_qos(argv[3]);
    }

    printf("pub with topic: %s and name %s and flags 0x%02x\n", argv[1], argv[2], (int)flags);

    /* step 1: get topic id */
    t.name = argv[1];
    if (emcute_reg(&t) != EMCUTE_OK) {
        puts("error: unable to obtain topic ID");
        return 1;
    }

    /* step 2: publish data */
    if (emcute_pub(&t, argv[2], strlen(argv[2]), flags) != EMCUTE_OK) {
        printf("error: unable to publish data to topic '%s [%i]'\n",
                t.name, (int)t.id);
        return 1;
    }

    printf("Published %i bytes to topic '%s [%i]'\n",
            (int)strlen(argv[2]), t.name, t.id);

    return 0;
}

static int cmd_sub(int argc, char **argv)
{
    unsigned flags = EMCUTE_QOS_0;

    if (argc < 2) {
        printf("usage: %s <topic name> [QoS level]\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) > TOPIC_MAXLEN) {
        puts("error: topic name exceeds maximum possible size");
        return 1;
    }
    if (argc >= 3) {
        flags |= get_qos(argv[2]);
    }

    /* find empty subscription slot */
    unsigned i = 0;
    for (; (i < NUMOFSUBS) && (subscriptions[i].topic.id != 0); i++) {}
    if (i == NUMOFSUBS) {
        puts("error: no memory to store new subscriptions");
        return 1;
    }

    subscriptions[i].cb = on_pub;
    strcpy(topics[i], argv[1]);
    subscriptions[i].topic.name = topics[i];
    if (emcute_sub(&subscriptions[i], flags) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", argv[1]);
        return 1;
    }

    printf("Now subscribed to %s\n", argv[1]);
    return 0;
}

static int cmd_unsub(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage %s <topic name>\n", argv[0]);
        return 1;
    }

    /* find subscriptions entry */
    for (unsigned i = 0; i < NUMOFSUBS; i++) {
        if (subscriptions[i].topic.name &&
            (strcmp(subscriptions[i].topic.name, argv[1]) == 0)) {
            if (emcute_unsub(&subscriptions[i]) == EMCUTE_OK) {
                memset(&subscriptions[i], 0, sizeof(emcute_sub_t));
                printf("Unsubscribed from '%s'\n", argv[1]);
            }
            else {
                printf("Unsubscription form '%s' failed\n", argv[1]);
            }
            return 0;
        }
    }

    printf("error: no subscription for topic '%s' found\n", argv[1]);
    return 1;
}

static int cmd_will(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage %s <will topic name> <will message content>\n", argv[0]);
        return 1;
    }

    if (emcute_willupd_topic(argv[1], 0) != EMCUTE_OK) {
        puts("error: unable to update the last will topic");
        return 1;
    }
    if (emcute_willupd_msg(argv[2], strlen(argv[2])) != EMCUTE_OK) {
        puts("error: unable to update the last will message");
        return 1;
    }

    puts("Successfully updated last will topic and message");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "con", "connect to MQTT broker", cmd_con },
    { "discon", "disconnect from the current broker", cmd_discon },
    { "pub", "publish something", cmd_pub },
    { "sub", "subscribe topic", cmd_sub },
    { "unsub", "unsubscribe from topic", cmd_unsub },
    { "will", "register a last will", cmd_will },
    { NULL, NULL, NULL }
};

int main(void)
{
    // initialize to SPI
    puts("Initializing to TFT SPI peripheral.");

    ucg_riotos_t user_data =
    {
        .device_index = SPI_DEV(0),
        .pin_cs = GPIO_PIN(PORT_B, 5),
        .pin_cd = GPIO_PIN(PORT_C, 7),
        .pin_reset = GPIO_PIN(PORT_A, 9),
    };

    ucg_SetUserPtr(&ucg, &user_data);

    ucg_Init(&ucg, ucg_dev_ili9341_18x240x320, ucg_ext_ili9341_18, ucg_com_hw_spi_riotos);

    // initialize the display
    puts("Initializing display.");

    ucg_SetRotate270(&ucg);
    ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT);
    ucg_SetFont(&ucg, ucg_font_helvR18_tf);
    ucg_font_height = 20;
    ucg_max_width_chars = 20;
    ucg_max_height = 12 * ucg_font_height;

    ucg_ClearScreen(&ucg);

    // start drawing
    puts("Drawing on screen.");

    for (int y = 0; y < 48; y++) {
        for (int x = 0; x < 96; x++) {
            uint32_t offset = (x + (y * 96)) * 3;

            ucg_SetColor(&ucg, 0, logo[offset + 0], logo[offset + 1], logo[offset + 2]);
            ucg_DrawPixel(&ucg, x, y);
        }
    }

    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 10, 80, 0, "MQTT-SN example app");

    // Keep vertical position of last string drawn on TFT
    ucg_pos_y = 5 * ucg_font_height;

    puts("MQTT-SN example application\n");
    puts("Type 'help' to get started. Have a look at the README.md for more"
         "information.");

    /* the main thread needs a msg queue to be able to run `ping6`*/
    msg_init_queue(queue, ARRAY_SIZE(queue));

    /* initialize our subscription buffers */
    memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));

    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;

}
