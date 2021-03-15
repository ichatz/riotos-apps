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
#include <string.h>
#include <stdlib.h>

#include "xtimer.h"
#include "board.h"

#include "periph/gpio.h"
#include "periph/spi.h"

#include "ucg.h"
#include "ucg_riotos.h"
#include "logo.h"

#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#ifndef EMCUTE_ID
#define EMCUTE_ID           ("gertrud")
#endif
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS           (1U)
#define TOPIC_MAXLEN        (64U)

static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];

static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

static ucg_t ucg;
static int ucg_pos_y;
static int ucg_font_height;
static int ucg_max_height;
static int ucg_max_width_chars;

static ucg_riotos_t user_data =
{
    .device_index = SPI_DEV(0),
    .pin_cs = GPIO_PIN(PORT_B, 6),
    .pin_cd = GPIO_PIN(PORT_C, 7),
    .pin_reset = GPIO_PIN(PORT_A, 9),
};


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
    while (len_remaining % ucg_max_width_chars > 0) {
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

        ucg_DrawString(&ucg, 10, ucg_pos_y, 0, subbuff);
        ucg_pos_y += ucg_font_height;
    }
}

/**
 * Setup the SPI pins, initialize the UCG module for the given TFT drivers
 * and print the welcome message.
 */
int setup_tft(void)
{
    ucg_SetUserPtr(&ucg, &user_data);

    // ILI9341 TFT
    ucg_Init(&ucg, ucg_dev_ili9341_18x240x320, ucg_ext_ili9341_18,
             ucg_com_hw_spi_riotos);

    ucg_SetRotate270(&ucg);
    ucg_SetFontMode(&ucg, UCG_FONT_MODE_TRANSPARENT);
    ucg_SetFont(&ucg, ucg_font_helvR18_tf);
    ucg_font_height = 20;
    ucg_max_width_chars = 20;
    ucg_max_height = 12 * ucg_font_height;

    ucg_ClearScreen(&ucg);

    // start drawing logo
    printf("Printing welcome message and drawing RIOT logo on screen.\n");

    for (int y = 0; y < 48; y++) {
        for (int x = 0; x < 96; x++) {
            uint32_t offset = (x + (y * 96)) * 3;

            ucg_SetColor(&ucg, 0, logo[offset + 0], logo[offset + 1],
                         logo[offset + 2]);
            ucg_DrawPixel(&ucg, x, y);
        }
    }

    ucg_SetColor(&ucg, 0, 255, 255, 255);
    ucg_DrawString(&ucg, 10, 80, 0, "MQTT-SN example app");

    // Keep vertical position of last string drawn on TFT
    ucg_pos_y = 5 * ucg_font_height;

    return 0;
}

/**
 * Setup the EMCUTE, open a connection to the MQTT-S broker,
 * subscribe to the default topic and publish a message.
 */
int setup_mqtt(void)
{
    /* the main thread needs a msg queue to be able to run `ping6`*/
    msg_init_queue(queue, ARRAY_SIZE(queue));

    /* initialize our subscription buffers */
    memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));

    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                  emcute_thread, NULL, "emcute");

    // connect to MQTT-SN broker
    printf("Connecting to MQTT-SN broker %s port %d.\n",
           SERVER_ADDR, SERVER_PORT);

    sock_udp_ep_t gw = { .family = AF_INET6, .port = SERVER_PORT };
    char *topic = MQTT_TOPIC;
    char *message = "connected";
    size_t len = strlen(message);

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, SERVER_ADDR) == NULL) {
        printf("error parsing IPv6 address\n");
        return 1;
    }

    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", SERVER_ADDR,
               (int)gw.port);
        return 1;
    }

    printf("Successfully connected to gateway at [%s]:%i\n",
           SERVER_ADDR, (int)gw.port);

    /* setup subscription to topic*/
    unsigned flags = EMCUTE_QOS_0;
    subscriptions[0].cb = on_pub;
    strcpy(topics[0], MQTT_TOPIC);
    subscriptions[0].topic.name = MQTT_TOPIC;

    if (emcute_sub(&subscriptions[0], flags) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", MQTT_TOPIC);
        return 1;
    }

    printf("Now subscribed to %s\n", MQTT_TOPIC);

    return 1;
}

int main(void)
{
    printf("RIOT TFT/MQTT-SN application\n"
           "Use the TFT ILI9341 LED to display messages received to a\n"
           "MQTT-SN topic\n");

    // initialize to SPI
    printf("Initializing TFT SPI peripheral.\n");
    setup_tft();

    // setup MQTT-SN connection
    printf("Setting up MQTT-SN.\n");
    setup_mqtt();

    while (1) {
        xtimer_sleep(5);
    }

    /* should be never reached */
    return 0;
}
