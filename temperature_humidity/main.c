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
 * @brief       Measure the ambient temperature and humidity using a digital
 *              sensor.
 *
 * @author      Ioannis Chatzigiannakis <ichatz@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#define DHT_PARAM_TYPE              (DHT22)

#include "fmt.h"
#include "dht.h"
#include "dht_params.h"

#include "periph/pm.h"
#include "periph/rtc.h"

/**
 * Call-back function invoked when RTC alarm triggers wakeup.
 */
static void callback_rtc(void *arg)
{
    puts(arg);
}

int main(void)
{
    printf("RIOT temperature_humidity application\n"
           "DHT temperature and humidity sensor test application\n"
           "using RIOT DHT peripheral driver\n"
           "DHT sensor type %d\n", DHT_PARAM_TYPE);

    /* Fix port parameter for digital sensor */
    dht_params_t my_params;
    my_params.pin = GPIO_PIN(PORT_A, 10);
    my_params.type = DHT_PARAM_TYPE;
    my_params.in_mode = DHT_PARAM_PULL;

    /* Initialize digital sensor */
    dht_t dev;
    if (dht_init(&dev, &my_params) == DHT_OK) {
        printf("DHT sensor connected\n");
    }
    else {
        printf("Failed to connect to DHT sensor\n");
        return 1;
    }

    /* Retrieve sensor reading */
    int16_t temp, hum;
    if (dht_read(&dev, &temp, &hum) != DHT_OK) {
        printf("Error reading values\n");
    }

    /* Extract + format temperature from sensor reading */
    char temp_s[10];
    size_t n = fmt_s16_dfp(temp_s, temp, -1);
    temp_s[n] = '\0';

    /* Extract + format humidity from sensor reading */
    char hum_s[10];
    n = fmt_s16_dfp(hum_s, hum, -1);
    hum_s[n] = '\0';

    printf("DHT values - temp: %s°C - relative humidity: %s%%\n",
           temp_s, hum_s);

    /* Set an RTC-based alarm to trigger wakeup */
    const int mode = 0;
    const int delay = 5;

    printf("Setting wakeup from mode %d in %d seconds.\n", mode, delay);
    fflush(stdout);

    struct tm time;
    rtc_get_time(&time);
    time.tm_sec += delay;
    rtc_set_alarm(&time, callback_rtc, "Wakeup alarm");

    /* Enter deep sleep mode */
    pm_set(mode);

    return 0;
}
