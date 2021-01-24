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
 * @brief       ADC test application
 *
 * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
 *
 * @}
 */

#include <stdio.h>
#include <math.h>

#include "cpu.h"
#include "board.h"
#include "xtimer.h"
#include "periph/adc.h"
#include "periph/gpio.h"

#define RES                         ADC_RES_12BIT
#define DELAY                       (100LU * US_PER_MS) /* 100 ms */
#define ADC_IN_USE                  0
#define ADC_CHANNEL_USE             0

int main(void)
{
    xtimer_ticks32_t last = xtimer_now();
    int sample = 0;

    puts("\nRIOT ADC peripheral driver test\n");
    puts("This test will sample all available ADC lines once every 100ms with\n"
         "a 12-bit resolution and print the sampled results to STDIO\n\n");

    /* initialize all available ADC lines */
    for (unsigned i = 0; i < ADC_NUMOF; i++) {
        if (adc_init(ADC_LINE(i)) < 0) {
            printf("Initialization of ADC_LINE(%u) failed\n", i);
            return 1;
        } else {
            printf("Successfully initialized ADC_LINE(%u)\n", i);
        }
    }

    while (1) {
        for (unsigned i = 0; i < ADC_NUMOF; i++) {
            sample = adc_sample(ADC_LINE(i), RES);
            if (sample < 0) {
                printf("ADC_LINE(%u): selected resolution not applicable\n", i);
            } else {
                printf("ADC_LINE(%u): %i\n", i, sample);
            }
        }
        xtimer_periodic_wakeup(&last, DELAY);
    }

    return 0;
}
