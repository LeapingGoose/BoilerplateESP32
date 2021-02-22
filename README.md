## General

This project is something I wish I had found when starting down the road of bootloader fun. I hope this is useful for you as well and if the documentation is lacking at the moment, feel free to contact me with any questions you may have and hopefully I can help you out.


## Objective

I was unable to get the bootloader to boot the factory app as defined in online documents using the default values so I adjusted the bootloader code to make it happen.

This project will create a boilerplate with a minimal bootloader that handles the option of loading a factory app when booting and the GPIO pin 4 is pulled down for 5 seconds. The pin and pull-down time can be set via `idf.py menuconfig' when executed in the idf-bootloader project directory.

In the root directory of this project is a PlatformIO project that defines and contains the Factory app. The Factory app uses the "Arduino" platform for development and will go into the Factory partition.

The bootloader is not a PlatformIO project and is not using the "Arduino" platform, it's an "esp-idf" project. I found building a native ESP32 app for managing the bootloader, factory app, OTA updates, and partitions was easier than using the Arduino platform with the Espressif docs.

See the idf-bootloader directory's README.md file for more details.


## Workflow

- Clean the flash on the destination ESP32 ($ ef32).
- Build and flash the Factory app to the ESP32, this also includes the partition table (PlatformIO build and flash).
- Build and flash the bootloader to the ESP32 (bb32, fb32)
- Start the ESP32 module
- Connect to the WiFi access point created by the ESP32 (WiFi: WROOM-FACTORY, Password: wroomwroom)
- Go to 192.168.4.1
- Upload primary app via OTA.


## Circuit

#### Components Required

  - 1 ESP32 (I used ESP-WROOM-32)
  - 1 Button
  - 1 10K resistor
  - 3 wires


#### Circuit

  - ESP32 Ground        -> Button pin 1
  - ESP32 GPIO 4        -> Button pin 2
  - ESP32 3.3v          -> 10K resistor, end 1
  - 10K resistor, end 2 -> Button Pin 2


## Todo

- Finish this README.md file.

## Local Aliases I Use
```
# Execute this if other commands below aren't working. This will
# create the pathing required to execute in the directory it's run in.
alias export32=". ~/esp/esp-idf/export.sh"
alias x32=export32

alias lsports="ls /dev/cu.*"
alias fport32="idf.py -p /dev/cu.SLAB_USBtoUART "
alias fp32=fport32

alias menu32="idf.py menuconfig"
alias build32="idf.py build"
alias flash32="idf.py -p /dev/cu.SLAB_USBtoUART flash"
alias f32=flash32
alias fboot32="idf.py -p /dev/cu.SLAB_USBtoUART bootloader-flash"
alias fb32=fboot32

alias buildpart32="idf.py partition_table"
alias bp32=buildpart32
alias fpart32="idf.py -p /dev/cu.SLAB_USBtoUART partition_table-flash"
alias fp32=fpart32

# alias mon32="idf.py -p /dev/cu.SLAB_USBtoUART monitor"
# Not sure why but the monitor isn't finding the proper build folder, so we're telling it where it is.
alias mon32="idf.py -p /dev/cu.SLAB_USBtoUART -B ./build/bootloader monitor"

alias eraseota32="idf.py -p /dev/cu.SLAB_USBtoUART erase_otadata"
alias buildboot32="idf.py bootloader"
alias bb32=buildboot32

alias eraseflash32="idf.py -p /dev/cu.SLAB_USBtoUART erase_flash"
alias ef32=eraseflash32

alias espt="esptool.py -p /dev/cu.SLAB_USBtoUART "
alias cleanbuild32="idf.py fullclean; idf.py build"
alias fc32="idf.py fullclean"
```