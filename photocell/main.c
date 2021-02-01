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
 * @brief       Measure the intensity of ambient light using a photocell.
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
#include "analog_util.h"

#define ADC_IN_USE                  ADC_LINE(0)
#define ADC_RES                     ADC_RES_12BIT

#define DELAY                       (100LU * US_PER_MS) /* 100 ms */


int main(void)
{
    printf("\nRIOT Photocell application\n");
    printf("Measure the intensity of ambient light using a photocell\n");
    printf("connected to the Analog input of the STM32 Nucleo-64 board.\n");
    printf("The sensor is sampled through the ADC lines once every 100ms\n");
    printf("with a 12-bit resolution and print the sampled results to STDIO\n");

    /* initialize the ADC line */
    if (adc_init(ADC_IN_USE) < 0) {
        printf("Initialization of ADC_LINE(%u) failed\n", ADC_IN_USE);
        return 1;
    }
    else {
        printf("Successfully initialized ADC_LINE(%u)\n", ADC_IN_USE);

    }

    xtimer_ticks32_t last = xtimer_now();
    int sample = 0;
    int lux = 0;

    /* Sample continously the ADC line */
    while (1) {
        sample = adc_sample(ADC_IN_USE, ADC_RES);
        lux = adc_util_map(sample, ADC_RES, 10, 100);

        if (sample < 0) {
            printf("ADC_LINE(%u): selected resolution not applicable\n",
                   ADC_IN_USE);
        }
        else {
            printf("ADC_LINE(%u): raw value: %i, lux: %i\n", ADC_IN_USE, sample,
                   lux);
        }

        xtimer_periodic_wakeup(&last, DELAY);
    }

    return 0;
}
