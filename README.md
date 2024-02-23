# Scalable KVM Over IP (SKVMOIP)
This repository contains source code for a SKVMOIP for Windows and Linux platforms written in C++.<br>

### Introduction
There are many options available in the market for KVKM over IP, however most of them are either made for a corporate or very expensive for individuals - and still not scalable enough.
Many people have built cheap solutions such as PiKVM and TinyPilot - but I still find them expensive given that they can only support one machine and one would need to spend more money to replicate over multiple machines.

Therefore, I started this project to invent a Scalable KVM Over IP software suite we can be installed on cheap hardware modules and on a client computer. This KVMK Over IP is scalable to any size because it uses ethernet wiring to communicate with the computers/servers with only one Encoder Server; hence no need to replicate the "pricy" hardware for each machine to be managed.

### Features of SKVMOIP
1. Keyboard Over IP, works even to boot systems into their BIOS and without any software installed on the target system.
2. Mouse Over IP, works even in the BIOS of the systems and without any software installed on the target system.
3. Video Over IP, works even before the system boots and without any software installed on the target system.
4. Power/Reset/HDDLED/PowerLED Over IP, turn On or Off the systems remotely just like you do onsite by pressing the power button.
5. Easy to use Client software for Windows and Linux platforms.

### What's in the Software Suite
1. Firmware Software for STM32401CCU6 and W5500 WizChip
2. Client Software for a Client Computer, called "SKVMOIP Client".
3. Server Software for HDMI stream Encoder Server.

### Hardware Requirements
1. STM32F401CCU6 Microcontroller
2. W5500 Wizchip Ethernet Module
4. ST Link Programmer
5. HDMI capture device
6. A Video Encoder server computer (usually a mini PC with enough processing power, more quantitative data will be added here in future).
7. A client computer either running Windows or Linux.

### How to setup the SKVMOIP?
Installing and setting up the software suite is easy to follow and requires no special technical knowledge. However, currently only Windows in msys2 environment is supported.

- Install Msys2 development environment from [here](https://www.msys2.org/)
- Launch the `MSYS2 MINGW64` from start menu
- Install gcc using `$pacman -S mingw-w64-x86_64-gcc`
- Install make using `$pacman -S mingw-w64-x86_64-make`
- Install git using `$pacman -S git`
- Install glslang using `$pacman -S mingw-w64-x86_64-glslang`, this is only required for Client Build
- Install gtk3 using `$pacman -S mingw-w64-x86_64-gtk3`, this is only required for Client Build
- Clone this repository using `$git clone https://github.com/ravi688/SKVMOIP`

#### Setting up the STM32F401CCU6 and W5500 via SPI interface
Connect GND and 3.3 Vcc pins of the W5500 to the GND and 3.3 Vcc pins of STM32 MCU. Now check if the W5500 and STM32 MCU both are powering on (the red lights should light up on both).

Connect the MOSI, MISO, and CLOCK pins of the W5500 to the SPI1 pins of the STM32 MCU.

Connect the RST pin of W5500 to the one of the GPIO pins of STM32 MCU to enable reset function trigger from the STM32 MCU. 

#### Setting up the STM32F401CCU6 Firmware
Install STM32Cube IDE in any Windows Computer, and load the project under the directory `SKVMOIP/Firmware/STM32F401CCU6`

Press the key combination `Windows Logo Key + B` to build the project and make sure there are no build errors.

Now, plug the ST link programmer to one of the USB ports of the computer (on which STM32Cube IDE has been installed). <br>
NOTE: Before plugging the ST link programmer, make appropriate connections to the microcontroller's pins from some online guides.

Click on the `Run` tab on the menu bar on the top, and select `run` to build and upload the executable on the Microcontroller.

#### Setting up server for Windows
It requires building the server executable first and then deploying it to the encoder server computer (a windows computer).
- cd into the git repo directory (on the server computer) using `$cd SKVMOIP`
- Make sure to run `$make -s clean` for previous builds if any
- Run `$make -s build BUILD=server OUT=server`, this will start the server build process
- And outputs a `server.exe` executable file
- Run the application using `./server.exe`
- Now, note down the IP address and Port number at which the server is listening for connections
- Done!

#### Setting up client For Windows
It requires buildilng the installer first and then deploying it to a Windows Client Computer.
- cd into the git repo directory (on the client computer) using `$cd SKVMOIP`
- Make sure to run `$make -s clean` for previous builds if any
- Run `$make -s build BUILD=client OUT=client`, this will start the client build process
- And outputs a `client.exe` executable file
- Run the application using `./client.exe`
- Now you must have to first select a machine and click on `Connect` button
- Once, the connection has been established, you would need to click on `Video` button to start using Remote Desktop.
- Done!

#### Setting up client For Linux
In-Progress

### TODO
1. Add HDMI Routing over IP support to scale Video Over IP across local networks (as LANs are very fast, can handle the RAW HDMI data).'
2. Add support for USB Mass Storage over IP to enable the host computer boot from a remote USB flash drive or Computer acting as a USB flash drive. However, PXE boot already exists.
3. Add support for Audio Over IP

### License
                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/
                        (See: LICENSE file, for more info)
