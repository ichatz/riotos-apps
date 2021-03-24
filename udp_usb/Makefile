# name of your application
APPLICATION = udp_usb

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f401re

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Include packages that pull up and auto-init the link layer.
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif

# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo

USEMODULE += stdio_ethos
USEMODULE += gnrc_uhcpc

USEMODULE += xtimer

# Address of device
IPV6_PREFIX ?= fe80:2::/64
STATIC_ROUTES ?= 1

# Port number to setup UDP server
SERVER_PORT = 8888
CFLAGS += -DSERVER_PORT='"$(SERVER_PORT)"'

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Default to using ethos for providing the uplink when not on native
UPLINK ?= ethos

# Configure ethos parameters
ETHOS_BAUDRATE ?= 115200
CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)

# Ethos/native TAP interface and UHCP prefix can be configured from make command
TAP ?= tap0

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)

# Configure terminal parameters
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

include $(RIOTBASE)/Makefile.include
