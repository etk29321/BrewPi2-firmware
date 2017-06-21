# BrewPi2-firmware

## Building

The BrewPi2 firmware is currently built against particle photon firmware version 0.6.2 ('latest' branch).  Particle
 firmware can be found [here](https://github.com/spark/firmware/releases/tag/v0.6.2).  Make sure to upgrade your base firmware before
 installing this application firmware as per the "Updating System Firmware" instructions on the particle github page.
 
 1. Download particle firmware and untar
 2. Configure gcc-arm as per the instructions on the particle github
 3. Clone this repo into firmware-0.6.2/user/application/brewpi2
 4. cd to firmware-0.6.2/main
 5. Issue make command 
 
     make APP=brewpi2 PLATFORM=photon
     
 6. Place BrewPi Photon in DFU mode
 7. Issue program dfu command
 
    make APP=brewpi2 PLATFORM=photon program-dfu
    
    
 ## Manual Installation
 
 **Install System Firmware**
 
    dfu-util -d 2b04:d006 -a 0 -s 0x8020000 -D system-part1-0.4.6-photon.bin
    dfu-util -d 2b04:d006 -a 0 -s 0x8060000:leave -D system-part2-0.4.6-photon.bin

**Install BrewPi2**

    dfu-util -d 0x2B04:0xD006 -a 0 -s 0x80A0000:leave -D brewpi2.dfu