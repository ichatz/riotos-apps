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
 * @brief       Control a solid state relay via COAP.
 *
 * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "xtimer.h"
#include "periph/gpio.h"

int main(void)
{
    printf("RIOT relay_coap application\n"
           "Control a Solid State Relay using RIOT GPIO module\n"
           "and issue commands via COAP\n");

    gpio_t pin_out = GPIO_PIN(PORT_B, 5);
    if (gpio_init(pin_out, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 5);
        return -1;
    }

    while (1) {
        printf("Set pin to HIGH and signal SSD to switch on lightbulb\n");
        gpio_set(pin_out);

        xtimer_sleep(5);

        printf("Set pin to LOW and signal SSD to switch off lightbulb\n");
        gpio_clear(pin_out);

        xtimer_sleep(5);
    }

    return 0;
}
