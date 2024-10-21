# USBNet

## Demonstrates a USB ethernet network on the Raspberry Pi Pico (or Pico W)
By default uses CDC-NCM, but can use ECM/RNDIS by changing the `USE_ECM` define from `0` to `1` in `tusb_config.h`

The Pico assigns itself an IP of 192.168.7.1 and a DHCP server is started that assigns the host computer 192.168.7.16. You should be able to ping the Pico from the computer to confirm the connection.

No additional libraries other than those included in the Pico SDK are needed. The built-in TinyUSB and LwIP are used.

This has only been tested briefly on Windows 11, but I would hope will work for MacOS and Linux and potentially Android and iOS too (the latter with the CDC-NCM driver).
I've only run it on a Pico W (RP2040) so far (though works when compiled as a Pico). I've not tested it on a Pico 2 / RP2350 yet, but since only the SDK is used it should be ok?

If you access stdio over the default UART connection, sending the 's' character will demonstrate a clean shutdown.

This is only meant as a starting point for your own projects!

The code is mostly based on the latest version of the TinyUSB webserver example:
https://github.com/hathach/tinyusb/tree/master/examples/device/net_lwip_webserver
with the hathach/tinyusb#2829 PR included (thanks!)
