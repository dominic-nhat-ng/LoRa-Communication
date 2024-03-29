# _Sample project_

**Can read description in note iot-garden below**

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
.
├── build
├── build.log
├── CMakeLists.txt
├── debug.log
├── lib
│   ├── CMakeLists.txt
│   ├── DHT11.c
│   ├── include
│   │   ├── DHT11.h
│   │   ├── lcd.h
│   │   ├── lib.h
│   │   └── lora.h
│   ├── lcd.c
│   ├── lib.c
│   └── lora.c
├── main
│   ├── CMakeLists.txt
│   ├── main.c
│   └── test.html
├── process.txt
├── pySX127x
│   ├── lora_util.py
│   ├── mqtt_client.py
│   ├── README.md
│   ├── rx_cont.py
│   ├── socket_client.py
│   ├── socket_transceiver.py
│   ├── SX127x
│   │   ├── board_config.py
│   │   ├── constants.py
│   │   ├── __init__.py
│   │   ├── LoRaArgumentParser.py
│   │   └── LoRa.py
│   ├── test_lora.py
│   ├── test_mqtt.py
│   ├── tx_beacon.py
│   └── VERSION
├── README.md
├── sdkconfig
├── sdkconfig.old
└── workflow.txt                
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

# LoRa-Communication

This application uses an ESP32 to read data and publish it to the ThingsBoard platform.

This project is currently in progress.

I've completed the task of pushing data from the ESP32 to the ThingsBoard platform.

I have also completed the transmission and reception of data from the ESP32 to a Raspberry Pi through LoRa. Currently, I am pushing data from the Raspberry Pi to ThingsBoard using Python. The final step will be to build a system that can be controlled by either the ESP32 or the Raspberry Pi with a sensor.

This project was successfully built in one month.

Through this project, I've learned about the ESP32, LoRa communication, and using SSH to control a Raspberry Pi. This was my first time coding with a Raspberry Pi and building a full embedded system, perhaps an IoT garden, or something similar.

This project has been very beneficial for me. It has improved my programming skills, my ability to build an application, and my mindset about building a system in practice.


## Description of each folder

**lib**: This folder contains the libraries used in this project.
**main**: This folder contains the main files of the project.
**pySX127x**: This folder is used in the Raspberry Pi to receive data from the ESP and push data to ThingsBoard.

## Description of functionanility in this project

The ESP32 will collect data from the DHT11 sensor, push this data to ThingsBoard using the MQTT protocol, and send data to the Raspberry Pi using LoRa. The Raspberry Pi will then push this data to ThingsBoard using the MQTT protocol.

We can also control the ESP32 from ThingsBoard to switch a connected device on or off.

Dominic Ng.