__Note: The partitions.csv must have identical partitions as any other apps that will be flashed (i.e. Factory, OTA, etc...).__

What we're primarily interested in with this project is:
```/components/bootloader/subproject/main/bootloader_start.c```
That is the bootloader code and is the only source file in this project that has been modified. It originates from the ~/esp/esp-idf/... folder.

## Building and Flashing the Bootloader

Before this will work, you must execute `idf.py menuconfig` in this projects folder, idf-bootloader. You must have esp-idf installed on your system to do this. In the menu system that appears, go into "Bootloader config" and check "GPIO triggers factory reset". The rest of the defaults should be left alone unless you know what you're doing and want differnt values.

This project is only interested in building and flashing
the bootloader by performing the following on the command line:
Note: If the idf.py command is not found, ensure esp-idf is installed on your system and run the following at the command line in the root of this project: ```$ . ~/esp/esp-idf/export.sh```

  * Clean the build as required.
    ```$ idf.py fullclean```

  * Compile the bootloader.
    ```$ idf.py bootloader```

  * Flash the bootloader to the ESP32 chip.  
    ```$ idf.py -p /dev/cu.SLAB_USBtoUART bootloader-flash```

Using "idf.py menuconfig" in the terminal at the root of this project and enter the "Bootloader config" menu. Now ensure "GPIO triggers factory reset" is checked and set the "Number of the GPIO input for factory reset" to whichever pin you would like to control the reset (default '4'). Also review the "Hold time of GPIO for reset/test mode (seconds)" menu item to ensure it's a value you would like. Save and exit.


## Helpful Terminal Shortcuts I Use

```
# Prep environment to use idf.py
alias export32=". ~/esp/esp-idf/export.sh"

alias x32=export32
alias lsports="ls /dev/cu.*"
alias menu32="idf.py menuconfig"
alias build32="idf.py build"
alias flash32="idf.py -p /dev/cu.SLAB_USBtoUART flash"
alias f32="idf.py -p /dev/cu.SLAB_USBtoUART flash"
alias fboot32="idf.py -p /dev/cu.SLAB_USBtoUART bootloader-flash"
alias fpart32="idf.py -p /dev/cu.SLAB_USBtoUART partition_table-flash"
alias fport32="idf.py -p /dev/cu.SLAB_USBtoUART "
alias fp32="idf.py -p /dev/cu.SLAB_USBtoUART "
alias mon32="idf.py -p /dev/cu.SLAB_USBtoUART monitor"
alias clearota32="idf.py -p /dev/cu.SLAB_USBtoUART erase_otadata"
alias buildboot32="idf.py bootloader"
alias clean32="idf.py fullclean"

alias espt="esptool.py -p /dev/cu.SLAB_USBtoUART "
alias cleanbuild32="idf.py fullclean; idf.py build"
```