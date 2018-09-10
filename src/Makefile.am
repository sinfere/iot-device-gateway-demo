AM_CFLAGS = -g -O2 -Wall -Werror -Wno-deprecated-declarations -fno-strict-aliasing -std=gnu99 -D_GNU_SOURCE
AM_CFLAGS += -I$(top_srcdir)/libmqtt/src

bin_PROGRAMS = gateway
gateway_SOURCES = main.c common.c bkv.c mqtt_client.c iot_client.c json.c
gateway_CFLAGS = $(AM_CFLAGS)
gateway_LDADD = $(top_builddir)/libmqtt/libmqtt.la

all: gateway