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
├── LoRa-Mqtt
│   └── mqtt_client.py
├── main
│   ├── CMakeLists.txt
│   ├── main.c
│   └── test.html
├── process.txt
├── README.md
├── sdkconfig
├── sdkconfig.old
└── workflow.txt                   This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

# iot-garden

This application uses an ESP32 to read data and publish it to the ThingsBoard platform.

This project is currently in progress.

I've completed the task of pushing data from the ESP32 to the ThingsBoard platform.

I have also completed the transmission and reception of data from the ESP32 to a Raspberry Pi through LoRa. Currently, I am pushing data from the Raspberry Pi to ThingsBoard using Python. The final step will be to build a system that can be controlled by either the ESP32 or the Raspberry Pi with a sensor.

This project was successfully built in one month.

Through this project, I've learned about the ESP32, LoRa communication, and using SSH to control a Raspberry Pi. This was my first time coding with a Raspberry Pi and building a full embedded system, perhaps an IoT garden, or something similar.

This project has been very beneficial for me. It has improved my programming skills, my ability to build an application, and my mindset about building a system in practice.

Dominic Ng.