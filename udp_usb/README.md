# How to provide IPv6 networking via the USB with RIOT

Use the USB to provide IPv6 networking to an [STM32 Nucleo-64 F401RE development board](https://www.st.com/en/evaluation-tools/nucleo-f401re.html) using the [RIOT operating system](https://github.com/RIOT-OS/RIOT).

RIOT provides two tools: [ethos (Ethernet Over Serial)](https://api.riot-os.org/group__drivers__ethos.html#details) and [UHCP (micro Host Configuration Protocol)](https://riot-os.org/api/group__net__uhcp.html) so that the serial link (USB) can provide network connectivity and also debugging messages. _Ethos_ multiplexes serial data to separate ethernet packets from debugging messages. _UHCP_ is a RIOT-proprietary protocol in charge of configuring the network interface prefix and routes of the device. In the case of the [STM32 Nucleo-64 F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html) this is very useful as the development board does not provide a wireless network interface.

## For this application we will use
- STM32 Nucleo-64 F401RE

## STM32 Nucleo-64 F401RE development board

The [STM32 Nucleo-64 F401RE](https://www.st.com/en/evaluation-tools/nucleo-f401re.html) is a low-cost development board that utilizes a 32-bit ARM Cortex-M4 processor to power various combinations of performance and power consumption. The CPU frequency can go as high as 84 MHz while the power consumption can go as low as 2.4uA at standby without RTC. The STM32 Nucleo board supports the ARDUINO® Uno V3 connectivity headers and the ST morpho headers allowing the easy expansion of the functionality with a wide choice of specialized shields.

## The RIOT operating system

The [RIOT](https://github.com/RIOT-OS/RIOT) is an open-source microkernel-based operating system designed for very low memory and energy footprint suitable for embedded devices that depend on real-time capabilities. RIOT provides out-of-the-box support for a very wide low-power wireless and communication stacks, making it an ideal choice to build Internet of Things (IoT) platforms.

## Setting-up a RIOT application

To create a new application you need to create a directory containing two files: (1) the [Makefile](Makefile) and (2) the [main.c](main.c) file.

At minimum the [Makefile](Makefile) should define the following basic aspects of the application:
- The name of the application (_APPLICATION_)
- The location of the RIOT base directory (_RIOTBASE_)
- Additionally, it is required to include the _Makefile.include_ from the _RIOTBASE_

```
# A simple name of the application  
APPLICATION = udp_usb

# This has to be the absolute path to the RIOT base directory:  
RIOTBASE ?= $(CURDIR)/../../RIOT    

include $(RIOTBASE)/Makefile.include
```

Within the [Makefile](Makefile) we can also specify the hardware platform to be used as the target board when compiling the code.

```
BOARD ?= nucleo-f401re
```

## Using the IPv6 network stack of RIOT

The implementation of the IPv6 stack builds upon the [Generic (GNRC) network stack](https://riot-os.org/api/group__net__gnrc.html), a modular IP network stack. The design and implementation details of the _netdev_ module are presented in details in [Martine Lenders' master thesis](http://doc.riot-os.org/mlenders_msc.pdf) and the [slide set of its defense](http://doc.riot-os.org/mlenders_msc_def.pdf).

Each layer of the network stack runs in its own thread and each lower layer thread has a higher priority than any upper layer thread. In this regard, the thread of the MAC layer implementation has the highest priority and threads on the application layer have the lowest priority. The communication between threads is handled by the kernel's Messaging / IPC functionality and by the GNRC communication interface. Most of the times IPC will take place between threads of neighboring layers for packets that traverse the network stack up or down.

We also need to declare the generic networking layer, in combination with the IPv6 module and the RIOT modules to pull up and auto-init the link layer.

All these modules are declared in the [Makefile](Makefile) as follows:

```
# Include packages that pull up and auto-init the link layer.
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif

# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
```

We will also use the _xtimer_ module that provides a high-level API to multiplex the available timers. We add the following line in the [Makefile](Makefile):

```
USEMODULE += xtimer
```

## Providing network connectivity via the USB

The _ethos_uhcpd_ tool builds on top of the serial interface, [ethos (Ethernet Over Serial)](https://api.riot-os.org/group__drivers__ethos.html#details) and [UHCP (micro Host Configuration Protocol)](https://riot-os.org/api/group__net__uhcp.html).

To use the _ethos_uhcpd_ tool we need to add the _stdio_ethos_ and _gnrc_uhcpc_ modules in the [Makefile](Makefile).

```
USEMODULE += stdio_ethos gnrc_uhcpc

```

The UHCP will be used to provide the default IPv6 prefix and configure the network routes. The network prefix can be replaced by another local prefix, or even by a global prefix. This is configured as follow in the [Makefile](Makefile):

```
IPV6_PREFIX ?= fe80:2::/64
STATIC_ROUTES ?= 1
```

The _ethos_uhcpd_ tool will provide network connectivity through the TAP interface.
This is configured as follow in the [Makefile](Makefile):

```
UPLINK ?= ethos

ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

TAP ?= tap0
```

Finally, The _ethos_uhcpd_ tool will use the script _setup_network.sh_ located in the $(RIOTTOOLS)/ethos directory using superuser privileges. This script sets up a tap device, configures a prefix and starts a uhcpd server serving that prefix towards the tap device. The script builds upon the _host-tools_. In the [Makefile](Makefile) we need to include the following elements:

```
host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

# Configure terminal parameters
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)
```

Notice that since the _ethos_uhcpd_ tool is using _sudo_, thus when we issue the command _make flash term_ we will be prompted for the password to execute the _sudo_ command.

We are can now test the connectivity using the ICMPv6 protocol. A minimal [main.c](main.c) file should contain the following:

```
#include "xtimer.h"

int main(void)
{
    while (1) {
        xtimer_sleep(5);
    }

    return 0;
}
```

To compile the application, flash the MCU and initiate the _ethos_uhcpd_ tool, we issue the following command:

```
make flash term
```

When compiling and flashing is complete and everything is in place, we should get the following output:

```
make -C ethos
make[2]: Nothing to be done for 'all'.
make -C uhcpd
make[2]: Nothing to be done for 'all'.
make -C sliptty
make[2]: Nothing to be done for 'all'.
make -C zep_dispatch
make[2]: Nothing to be done for 'all'.
sudo sh ../../RIOT/dist/tools/ethos/start_network.sh  /dev/ttyACM0 tap0 fe80:2::/64 115200
net.ipv6.conf.tap0.forwarding = 1
net.ipv6.conf.tap0.accept_ra = 0
----> ethos: sending hello.
----> ethos: activating serial pass through.
----> ethos: hello received
NETOPT_RX_END_IRQ not implemented by driver
gnrc_uhcpc: Using 4 as border interface and 0 as wireless interface.
gnrc_uhcpc: only one interface found, skipping setup.
main(): This is RIOT! (Version: 2021.04-devel-1142-g8f93e)
----> ethos: hello reply received
```

The _start_network.sh_ script will setup the virtual network TAP interface named _tap0_ through which we can communicate with the [STM32 Nucleo-64 F401RE development board](https://www.st.com/en/evaluation-tools/nucleo-f401re.html). More information on how to utilize TAP interface are available under the [RIOT Virtual Networking tutorial](https://github.com/RIOT-OS/RIOT/wiki/Virtual-riot-network).

We can now use the _ping_ command as follows:

```
$ ping6 -c 3 fe80::2%tap0
PING fe80::2%tap0(fe80::2%tap0) 56 data bytes
64 bytes from fe80::2%tap0: icmp_seq=1 ttl=64 time=40.2 ms
64 bytes from fe80::2%tap0: icmp_seq=2 ttl=64 time=22.9 ms
64 bytes from fe80::2%tap0: icmp_seq=3 ttl=64 time=23.8 ms

--- fe80::2%tap0 ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2003ms
rtt min/avg/max/mdev = 22.858/28.962/40.207/7.961 ms
```

## Creating a UDP server

We will create a simple UDP server based on the [IPv6 networking libraries](https://doc.riot-os.org/group__net__ipv6.html) and the [sock API](https://doc.riot-os.org/group__net__sock.html) of [RIOT](https://github.com/RIOT-OS/RIOT).

The code is based on [Task 06](https://github.com/RIOT-OS/Tutorials/tree/master/task-06) of the [RIOT Tutorials](https://github.com/RIOT-OS/Tutorials).

First we need to include the following header files in the [main.c](main.c) as follows:

```
#include <stdio.h>

#include "net/sock/udp.h"
#include "net/ipv6/addr.h"
#include "thread.h"

#include "msg.h"
```

In [RIOT](https://github.com/RIOT-OS/RIOT) threads have their own memory stack. For this, in [main.c](main.c) we need to define the following:

```
static char server_stack[THREAD_STACKSIZE_DEFAULT];
```

We also need to define a message queue so that the thread running the UDP server can receive potentially fast incoming networking packets. This is done as follows:

```
#define SERVER_MSG_QUEUE_SIZE   (8)
static msg_t server_msg_queue[SERVER_MSG_QUEUE_SIZE];
```

The next step is to initialize the UDP socket server:

```
sock_udp_ep_t server = { .port = atoi(args), .family = AF_INET6 };
if (sock_udp_create(&sock, &server, NULL, 0) < 0) {
    return NULL;
}
```

Here we use a classic implementation of socket server using a continuous loop listening for new connections:

```
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
```

Within the _main()_ function we setup the new thread as follows:

```
if (thread_create(server_stack,
                  sizeof(server_stack),
                  THREAD_PRIORITY_MAIN - 1,
                  THREAD_CREATE_STACKTEST,
                  udp_server,
                  "8888",
                  "UDP Server") <= KERNEL_PID_UNDEF) {
    return -1;
}
```

The port of the UDP server are specified within the [Makefile](Makefile):

```
SERVER_PORT = 8888
```

## Testing the system

To compile the application, flash the MCU and initiate the _ethos_uhcpd_ tool, we issue the following command:

```
make flash term
```

When compiling and flashing is complete and everything is in place, we should get the following output:

```
make -C ethos
make[2]: Nothing to be done for 'all'.
make -C uhcpd
make[2]: Nothing to be done for 'all'.
make -C sliptty
make[2]: Nothing to be done for 'all'.
make -C zep_dispatch
make[2]: Nothing to be done for 'all'.
sudo sh ../../RIOT/dist/tools/ethos/start_network.sh  /dev/ttyACM0 tap0 fe80:2::/64 115200
net.ipv6.conf.tap0.forwarding = 1
net.ipv6.conf.tap0.accept_ra = 0
----> ethos: sending hello.
----> ethos: activating serial pass through.
----> ethos: hello received
NETOPT_RX_END_IRQ not implemented by driver
gnrc_uhcpc: Using 4 as border interface and 0 as wireless interface.
gnrc_uhcpc: only one interface found, skipping setup.
main(): This is RIOT! (Version: 2021.04-devel-1142-g8f93e)
RIOT UDP server application
Success: started UDP server on port 8888
```

We can now communicate with the UDP server as follows:

```
echo "hello" | nc -6u fe80::2%tap0 8888
```
