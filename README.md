# Scalable KVM Over IP (SKVMOIP)
This repository contains source code for a SKVMOIP for Windows and Linux platforms.

There are many options available in the market for KVKM over IP, however most of them either for a corporate or very expensive for individuals - and still not scalable enough.

Therefore, I started this project to invent a Scalable KVM Over IP software suite we can be installed on cheap hardware modules and on a client computer. This KVMK Over IP is scalable to any size because it uses ethernet wiring to communicate with the computers/servers.

### Features of SKVMOIP
1. Keyboard Over IP, works even to boot systems into their BIOS and without any software installed on the target system.
2. Mouse Over IP, works even in the BIOS of the systems and without any software installed on the target system.
3. Video Over IP, works even before the system boots and without any software installed on the target system.
4. Power/Reset/HDDLED/PowerLED Over IP, turn On or Off the systems remotely just like you do onsite by pressing the power button.
5. Easy to use Client software for Windows and Linux platforms.

### What's in the Software Suite
1. Firmware Software for STM32401CCU6 and W5500 WizChip
2. Client Software for a Client Computer.

### Hardware Requirements
1. STM32F401CCU6 Microcontroller
2. W5500 Wizchip Ethernet Module
4. ST Link Programmer
5. HDMI capture device
6. A client computer either running Windows or Linux.

### How to setup the SKVMOIP?
Installing and setting up the software suite is easy to follow and requires no special technical knowledge.

Clone this repository:
`$ git clone https://github.com/ravi688/SKVMOIP`

#### Setting up the STM32F401CCU6 and W5500 via SPI interface
Connect GND and 3.3 Vcc pins of the W5500 to the GND and 3.3 Vcc pins of STM32 MCU. Now check if the W5500 and STM32 MCU both are powering on (the red lights should light up on both).

Connect the MOSI, MISO, and CLOCK pins of the W5500 to the SPI1 pins of the STM32 MCU.

Connect the RST pin of W5500 to the one of the GPIO pins of STM32 MCU to enable reset function trigger from the STM32 MCU. 

#### Setting up the STM32F401CCU6 Firmware
Install STM32Cube IDE in any Windows Computer, and load the project under the directory `KVMOIP/Firmware/STM32F401CCU6`

Press the key combination `Windows Logo Key + B` to build the project and make sure there are no build errors.

Now, plug the ST link programmer to one of the USB ports of the computer (on which STM32Cube IDE has been installed). <br>
NOTE: Before plugging the ST link programmer, make appropriate connections to the microcontroller's pins from some online guides.

Click on the `Run` tab on the menu bar on the top, and select `run` to build and upload the executable on the Microcontroller.

#### Setting up client For Windows
It requires buildilng the installer first and then executing the installer to install the client in Windows.
1. `cd KVMOIP`
2. `./build.sh PLATFORM=Windows INSTALLER=1`
3. `cd build/Windows`
4. Now double click on `KVMOIP_Installer.exe` to execute the installer as usually you do to install any other Windows software package.

#### Setting up client For Linux
It is rather identical to Windows case above.
1. `cd KVMOIP`
2. `./build.sh PLATFORM=Linux INSTALLER=1`
3. `cd build/Linux`
4. `sudo dpkg -i ./KVMOIP_Installer.deb`

### TODO
1. Add HDMI Routing over IP support to scale Video Over IP across local networks (as LANs are very fast, can handle the RAW HDMI data).
