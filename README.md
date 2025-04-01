# USBNet

## Demonstrates a USB ethernet network on the Raspberry Pi Pico (or Pico W)
By default uses CDC-NCM, but can use ECM/RNDIS by changing the `USE_ECM` define from `0` to `1` in `tusb_config.h`

The Pico assigns itself an IP of `192.168.7.1` and a DHCP server is started that assigns the host computer `192.168.7.16`. You should be able to ping the Pico from the computer to confirm the connection.

I've also added mDNS, allowing the Pico to also be reached at `demo.local`

I've used the DHCP server code from the Pico examples rather than the TinyUSB one, but feel free to switch it out as needed.

No additional libraries other than those included in the Pico SDK are needed. Only the built-in TinyUSB and lwIP are used.

I have tested this on Windows 11 and iOS (CDC-NCM only), and I am aware of it working at least in Linux. I would hope it will work for MacOS and potentially Android too.
I've only run it on a Pico W (RP2040) so far (though works when compiled as a Pico). I've not tested it on a Pico 2 / RP2350 yet, but since only the SDK is used it should be ok?

For Pico W, if you need to use CYW43-specific functionality, after adding `cyw43_arch_init()` call `usb_network_init(...)` with the last parameter (init_lwip) as `false` to avoid re-initialising lwIP again.
If you use the e.g. `pico_cyw43_arch_lwip_poll` library in CMakeLists.txt then you can remove the pico_lwip* libraries in this example (as they'll be linked anyway).

If you access stdio over the default UART connection, sending the 's' character will demonstrate a clean shutdown.

This is only meant as a starting point for your own projects!

The code was mostly based on the latest version of the TinyUSB webserver example:
https://github.com/hathach/tinyusb/tree/master/examples/device/net_lwip_webserver
with the hathach/tinyusb#2829 PR included (thanks for that!)

- I've removed the actual web server (`httpd_init()` call) for simplicity, but this and anything else that can run under lwIP should work the same though the USB interface.
