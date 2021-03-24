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
 * @brief       Control an external LED via the GPIO peripheral.
 *
 * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "periph/gpio.h"
#include "xtimer.h"

int main(void)
{
    printf("RIOT led_ext application\n"
           "Control an external LED using RIOT GPIO module.\n");

    gpio_t pin_out = GPIO_PIN(PORT_B, 5);
    if (gpio_init(pin_out, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 5);
        return -1;
    }

    while (1) {
        printf("Set pin to HIGH\n");
        gpio_set(pin_out);

        xtimer_sleep(2);

        printf("Set pin to LOW\n");
        gpio_clear(pin_out);

        xtimer_sleep(2);
    }

    return 0;
}
