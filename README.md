# RIOT-OS Applications

This repository provides a collection of applications based on the [RIOT operating system](https://github.com/RIOT-OS/RIOT). The applications showcase specific functionalities of the RIOT operating system and how to build an IoT platform using an [STM32 Nucleo-64 development board](https://www.st.com/en/evaluation-tools/nucleo-f401re.html).

The applications are used as part of the hands-on activities of the [Internet of Things course](http://ichatz.me/Site/InternetOfThings2021) for the [MSc in Computer Engineering](http://ingegneriainformatica.diag.uniroma1.it/en/corsidistudio.htm), of the [Sapienza University of Rome](http://www.uniroma1.it) (in English)

## List of Applications

### Basic Networking
- [IPv6 Networking via USB](udp_usb) - The application demonstrates how to network using the IPv6 stack over a serial USB device through the ETHOS and UHCP tools of RIOT OS.
  - The application provides step by step instructions on how to setup a new RIOT application.
  - The application use multi-threading to implement a UDP socket server.

### Reading Sensors
- [Measuring Ambient Light Intensity](photocell) - The application demonstrates how to use an analog sensor through the ADC module.
  - The application provides some insights on the inner functionality of the XTIMER module of RIOT OS.
  - Some details are also provided on how IoT boards are defined in RIOT OS as part of the platform-independent layers.
  - A [video tutorial](https://youtu.be/Xw8a_NSvvvI) is available on YouTube.  
- [Measuring Ambient Temperature and Humidity](temperature_humidity) - The application demonstrates how to use a digital sensor through the provided RIOT driver.
  - The application provides some insights on the inner functionality of the PM module of RIOT OS.
  - Some details are also provided on how to measure the power consumption of an IoT board.
  - A [video tutorial on using the DHT22 sensor](https://youtu.be/0Z0uEQ21pL4) is available on YouTube.
  - A [project on hackster.io](https://www.hackster.io/ichatz/using-a-digital-temperature-sensor-with-riot-os-a4c213) is also available.   

### Controlling Devices
- [External LED Example](ledext) - The application demonstrates how to control an external LED through the GPIO device peripheral of RIOT OS.
  - The application provides some insights on identifying the GPIO port and pin number through the MCU's developer manual.
  - A [project on hackster.io](https://www.hackster.io/ichatz/control-external-led-using-riot-os-b626da) is also available.   
- [Control High-voltage Devices using an SSD Relay Module](relay_coap) - The application demonstrates how to control an 220V Lighbulb by driving an SSD relay module using the GPIO device peripheral  of RIOT OS.
  - The application provides some insights on identifying the GPIO port and pin number through the MCU's developer manual.   
  - The application requires to provide [IPv6 Networking via USB](udp_usb).
  - A [project on hackster.io](https://www.hackster.io/ichatz/drive-an-ssd-relay-module-using-riot-os-a59665) is also available.
- [Displaying Messages on a TFT LCD received via MQTT-S](tft_mqtts) - The application demonstrates how to use a TFT LCD through the provided RIOT package to display messages received from an MQTT topic.
  - The application provides some insights on how to connect devices via SPI.
  - The application demonstrates how RIOT can provide network connectivity via USB to connect to connect to a MQTT-S broker.
  - The application requires to provide [IPv6 Networking via USB](udp_usb).   


## STM32 Nucleo-64 F401RE development board

The [STM32 Nucleo-64 F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html) is a low-cost development board that utilizes an ARM Cortex-M processor to power various combinations of performance and power consumption. The STM32 Nucleo board supports the ARDUINO® Uno V3 connectivity headers and the ST morpho headers allowing the easy expansion of the functionality with a wide choice of specialized shields.

## How to use the RIOT operating system

The [RIOT](https://github.com/RIOT-OS/RIOT) is an open-source microkernel-based operating system designed for a very low memory and energy footprint suitable for embedded devices that depend on real-time capabilities. RIOT provides out-of-the-box support for a very wide low-power wireless and communication stacks, making it an ideal choice to build Internet of Things (IoT) platforms.

To build and use the applications you need to make sure that you have a local copy of the RIOT main code. For detailed instructions on how to clone and build the RIOT OS follow [the instructions in the RIOT repository](https://github.com/RIOT-OS/RIOT/blob/master/README.md#getting-started) and the READMEs within the respective application directory.
