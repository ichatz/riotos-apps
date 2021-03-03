# name of your application
APPLICATION = tft_mqttsn

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f401re

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Default to using ethos for providing the uplink when not on native
UPLINK ?= ethos

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
# For testing we also include the ping6 command and some stats
USEMODULE += gnrc_icmpv6_echo
# Include MQTT-SN
USEMODULE += emcute
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# Optimize network stack to for use with a single network interface
USEMODULE += gnrc_netif_single

USEMODULE += stdio_ethos

USEMODULE += ili9341
USEMODULE += xtimer

USEPKG += ucglib

FEATURES_REQUIRED += periph_gpio periph_spi

# Allow for env-var-based override of the nodes name (EMCUTE_ID)
ifneq (,$(EMCUTE_ID))
  CFLAGS += -DEMCUTE_ID=\"$(EMCUTE_ID)\"
endif

# include UHCP client
USE_DHCPV6 ?= 0
USEMODULE += gnrc_uhcpc

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Ethos/native TAP interface and UHCP prefix can be configured from make command
TAP ?= tap0
IPV6_PREFIX ?= fe80:2::/64

ETHOS_BAUDRATE ?= 115200
include $(CURDIR)/Makefile.ethos.conf

include $(RIOTBASE)/Makefile.include

.PHONY: host-tools

host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)
