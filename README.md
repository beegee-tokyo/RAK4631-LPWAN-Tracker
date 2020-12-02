
| | |
| :-: | :-: |
| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center> | <center><img src="./assets/WisBlock.svg" alt="WisBlock" width=75%></center> |    

RAK4631 LoRaWan® tracker node solution![RAKwireless](./assets/RAK-Whirls.png)
===    
This code acts as a LoRaWan® tracker node. It gets location information from an attached uBlox GPS module. In addition an acceleration sensor is used to detect if the tracker is moving.
If moving of the tracker is detected, location information is sent immediately. If the tracker is stationary, the location data is sent every 1 minute 

Solution
---
This solution shows
- how to initiate a LoRaWan connection with OTAA network join on the WisCore RAK4631
- how to initiate BLE with  on the WisCore RAK4631
  - BLE UART characteristic for debug output. Requires a BLE-UART app like the [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal) for Android
  - BLE OTA DFU for firmware updates
- how to use the WisSensor RAK1910 GPS module to aquire location information
- how to use the WisSensor RAK1904 acceleration sensor to detect movements
- how to read the battery level from the WisCore RAK5005-O board
- how to convert the sensor values into a byte array to create the smallest possible LoRa package size

Hardware required
---
To build this solution the following hardware is required
- WisBase RAK5005-O
- WisCore RAK4631
- WisSensor RAK1904
- WisSensor RAK1910
- WisIO RAK1921
- LiPo battery

Software required
---
To build this solution the following is required
- ArduinoIDE
- [RAK4630 BSP](https://github.com/RAKWireless/RAK-nRF52-Arduino)
- [SX126x-Arduino](https://github.com/beegee-tokyo/SX126x-Arduino)
- [nRF52_OLED](https://github.com/beegee-tokyo/nRF52_OLED)
- [Sparkfun LIS3DH](https://github.com/sparkfun/SparkFun_LIS3DH_Arduino_Library)


LoRaWan server required
---
In order to get this code working you need access to a LoRaWan® gateway. This could be either of    
- a public LoRaWan® gateway, like [TheThingsNetwork](https://thethingsnetwork.org/) provides in several big cities all over the world
- a private LoRaWan® gateway with multi-channel capability
- a simple and cheap single channel LoRaWan® gateway

In addition you need an account at TheThingsNetwork. You need to create an application there and register your device before the data you send over LoRa can be forwarded by the gateway. It is quite simple and there is a good tutorial at [RAKwireless RAK4270 Quick Start Guide](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK4270-Module/Quickstart/#connecting-to-the-things-network-ttn). It is for another product of RAK, but the steps how to setup an application and how to register your device are the same. 

The region you live in defines the frequency your LoRaWan® gateways will use. So you need to setup your device to work on the correct frequency. The region is setup by adding a build flag into your projects platformio.ini file.
```
build_flags = -DREGION_US915
```
Valid regions are
```
REGION_AS923 -> Asia 923 MHz
REGION_AU915 -> Australia 915 MHz
REGION_CN470 -> China 470 MHz
REGION_CN779 -> China 779 MHz
REGION_EU433 -> Europe 433 MHz
REGION_EU868 -> Europe 868 MHz
REGION_IN865 -> India 865 MHz
REGION_KR920 -> Korea 920 MHz
REGION_US915 -> US 915 MHz
```

Some explanation for the code
---

Due to the complexity of the code, it is split into functional parts.
- main.h 
   - All the includes, global definitions and forward declarations for the app
- main.cpp
   - Setup function where we initialize all peripherals
   - Main loop
   - Timers callback functions for periodic and delayed sending of packages
- acc.cpp
   - Accelerometer initialization, interrupt callback function and interrupt clearing functions
- bat.cpp
   - Battery level functions
- ble.cpp
   - BLE initialization and BLE UART callback functions
- display.cpp
   - Display initialization and handling functions
- gps.cpp
   - GPS initialization and and data poll functions
- loraHandler.cpp
   - LoRaWan initialization function, LoRaWan handling task and LoRaWan event callbacks

How to achieve power saving with nRF52 cores on Arduino IDE
----
Within the nRF52 Arduino framework is no specific function to send the MCU into sleep mode. The MCU will go into sleep mode when 
- all running tasks are calling delay()
- the running tasks are waiting for a semaphore

In this example you use the semaphore method.    

Two tasks are running independently, the loop() task and the loraTask() task.
Two semaphores are used to control the activity of these two tasks.    
```cpp
/** Semaphore to wake up the main loop */
SemaphoreHandle_t loopEnable;
```
and
```cpp
/** Semaphore to wake up the main loop */
SemaphoreHandle_t loraEnable;
```

After setup and starting the LoRaWan join process, the semaphore _**loraEnable**_ stays available. The semaphore _**loopEnable**_ is taken.  
This means the loop task will go into waiting mode while the loraTask is enabled to handle LoRaWan events until the join process was successful. Once the join process is done, the _**loraEnable**_ semaphore is taken as well, forcing the loraTask to go into waiting mode as well.    
At this point no more task is active and the device will go into sleep mode.  
  
**Events to wake up the MCU**
1. Accelerometer detect movement
If the ACC sensor detects movement, the MCU receives an interrupt. In the interrupt callback the semaphore _**loopEnable**_ is given again, which allows the loop task to run.
2. Timer event
If the periodic sending timer is triggered, the MCU will wake up and the callback function of the timer will give the semaphore _**loopEnable**_, which allows the loop task to run.

Once the loop task is enabled, it will poll the position from the GPS module and requests sending a data package by calling sendLoRaFrame(). Then it takes the semaphore _**loopEnable**_, which puts herself back into waiting mode until the next event.

LoRa® is a registered trademark or service mark of Semtech Corporation or its affiliates. LoRaWAN® is a licensed mark. 