// USB ethernet and networking on the Pico using TinyUSB

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Peter Lawrence
 * Copyright (c) 2024 Matthew Bennett
 *
 * influenced by lrndis https://github.com/fetisov/lrndis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <lwip/etharp.h>
#include <lwip/ethip6.h>
#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/opt.h>
#include <lwip/timeouts.h>
#include <netif/ethernet.h>
#include <pico/stdlib.h>
#include <pico/unique_id.h>
#include <tusb.h>

#include "usb_network.h"

uint8_t tud_network_mac_address[6]; // MAC will be generated in usb_network_init()

// lwip network interface for usb
struct netif netif_usb;

// flag whether netif_usb interface has been added
static bool netif_added = false;

// shared between tud_network_recv_cb() and service_traffic()
static struct pbuf *received_frame;

// network interface functions:

static err_t tud_output(__unused struct netif *netif, struct pbuf *p) {
  for (;;) {
    // if TinyUSB isn't ready, signal back to lwip that there is nothing to do
    if (!tud_ready()) {
      return ERR_USE;
    }

    // check if the network driver can accept another packet
    if (tud_network_can_xmit(p->tot_len)) {
      tud_network_xmit(p, 0);
      return ERR_OK;
    }

    // can't send new packet yet
    // transfer execution to TinyUSB to hopefully finish transmitting the prior packet
    tud_task();
  }
}

static err_t netif_init_cb(struct netif *netif) {
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  netif->mtu = CFG_TUD_NET_MTU;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
  netif->state = NULL;
  netif->name[0] = 'E';
  netif->name[1] = 'X';
  netif->linkoutput = tud_output;
#if LWIP_IPV4
  netif->output = etharp_output;
#endif
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif
  return ERR_OK;
}

// driver callbacks:

void tud_network_init_cb() {
  // if the network is re-initialising and there is a leftover packet, perform a cleanup
  if (received_frame) {
    pbuf_free(received_frame);
    received_frame = NULL;
  }
}

bool tud_network_recv_cb(const uint8_t *src, uint16_t size) {
  // this shouldn't happen, but if receive another packet before
  // parsing the previous one, signal cannot accept
  if (received_frame) {
    return false;
  }

  if (size) {
    struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

    if (p) {
      // pbuf_alloc() has already initialised struct; just copy the data
      memcpy(p->payload, src, size);

      // store the pointer for service_traffic() to handle later
      received_frame = p;
    }
  }

  return true;
}

uint16_t tud_network_xmit_cb(uint8_t *dst, void *ref, uint16_t arg) {
  struct pbuf *p = (struct pbuf *)ref;

  (void)arg; // unused

  return pbuf_copy_partial(p, dst, p->tot_len, 0);
}

// usb network api:

static inline void service_traffic() {
  // handle any packet received by tud_network_recv_cb()
  if (received_frame) {
    if (netif_usb.input(received_frame, &netif_usb) != ERR_OK) {
      pbuf_free(received_frame); // only free on error
    }
    received_frame = NULL;
    tud_network_recv_renew();
  }

  sys_check_timeouts();
}

void usb_network_update() {
  tud_task();
  service_traffic();
}

bool usb_network_init(const ip4_addr_t *ownip, const ip4_addr_t *netmask, const ip4_addr_t *gateway) {
  if (!tud_init(PICO_TUD_RHPORT)) {
    printf("usb_network: tud_init fail\n");
    return false;
  }

  lwip_init();

  // generate new tud_network_mac_address from pico board_id, as in cyw43_hal_generate_laa_mac()
  pico_unique_board_id_t board_id;
  pico_get_unique_board_id(&board_id);
  memcpy(tud_network_mac_address, &board_id.id[2], 6);
  tud_network_mac_address[0] &= (uint8_t)~0x1; // unicast
  tud_network_mac_address[0] |= 0x2; // locally administered

  // the lwip virtual MAC address must be different from the host's; toggle the LSB
  netif_usb.hwaddr_len = sizeof(tud_network_mac_address);
  memcpy(netif_usb.hwaddr, tud_network_mac_address, sizeof(tud_network_mac_address));
  netif_usb.hwaddr[5] ^= 0x01;

  printf("usb_network: using MAC: ");
  int c = 0;
  char macString[20];
  for (int i = 0; i < sizeof(netif_usb.hwaddr); i++) {
    macString[c++] = "0123456789ABCDEF"[(netif_usb.hwaddr[i] >> 4) & 0xf];
    macString[c++] = "0123456789ABCDEF"[(netif_usb.hwaddr[i] >> 0) & 0xf];
  }
  macString[c] = 0;
  printf("%s\n", macString);

#if NO_SYS
  netif_input_fn input_func = ethernet_input;
#else
  netif_input_fn input_func = tcpip_input;
#endif

#if LWIP_IPV4
  if (netif_add(&netif_usb, ownip, netmask, gateway, NULL, netif_init_cb, input_func) == NULL) {
    printf("usb_network: error adding netif\n");
    return false;
  }
#endif
#if LWIP_IPV6
  if (netif_add(&netif_usb, NULL, netif_init_cb, input_func) == NULL) {
    printf("usb_network: error adding netif\n");
    return false;
  }
  netif_create_ip6_linklocal_address(&netif_usb, 1);
#endif
  netif_set_default(&netif_usb);
  netif_added = true;

  // wait for netif to be up
  while (!netif_is_up(&netif_usb)) {
    tight_loop_contents();
  }

  return true;
}

void usb_network_deinit() {
  if (netif_added) {
    netif_remove(&netif_usb);
    netif_added = false;
  }
  netif_usb.flags = 0;
  tud_deinit(PICO_TUD_RHPORT);
}
