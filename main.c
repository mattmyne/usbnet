#include <hardware/uart.h>
#include <lwip/ip.h>
#include <pico/stdlib.h>
#include <stdio.h>

#include "dhcpserver/dhcpserver.h"
#include "usb_network.h"

// usb network addresses
static const ip4_addr_t ownip = IPADDR4_INIT_BYTES(192, 168, 7, 1);
static const ip4_addr_t netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);
static const ip4_addr_t gateway = IPADDR4_INIT_BYTES(0, 0, 0, 0);

int main() {
  stdio_uart_init();

  // setup USB network
  if (!usb_network_init(&ownip, &netmask, &gateway)) {
    printf("failed to start usb network\n");
    return -1;
  }

  // setup DHCP server
  dhcp_server_t dhcp_server;
  dhcp_server_init(&dhcp_server, (ip_addr_t *)&ownip, (ip_addr_t *)&netmask, false);

  // enter main loop
  printf("setup complete, entering main loop\n");
  int key = 0;
  while ((key != 's') && (key != 'S')) {
    usb_network_update();
    key = getchar_timeout_us(0); // get any pending key press but don't wait
  }

  printf("shutting down\n");
  dhcp_server_deinit(&dhcp_server);
  usb_network_deinit();

  return 0;
}
