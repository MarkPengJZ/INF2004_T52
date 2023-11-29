# INF2004-Embedded-Systems-Programming

## Table of contents
* [General Info](#general-info)
* [Hardware](#hardware)
* [Pin Layout](#pin-layout)
* [Software](#software)
* [System Diagrams](#system-diagrams)
* [Members](#members)
* [Project Reflection](#project-reflection)

## General Info
### Introduction
INF2004 Team 52 Robot Car Project
###Directory Structure
A CMake Project containing our source code. Divided into sub-folders each containing one sub-module, depicted below.
```bash
driver
├── encoder
├── irline_barcode
├── line_detector
├── magnetometer
├── motor
├── ultrasonic
├── wifi
└── controller.c (program entry point)
└── CMakeLists.txt (main CMake file)
```

## Hardware
- Raspberry Pico W
- 1x L298N Motor Driver
- 2x DC Motor
- 2x Encoder
- 3x IR Sensor
- 1x Magnetometer
- 1x HC-SR04 Ultrasonic Sensor

## Pin Layout
| Hardware | GPIO # |
|---|---|
| Encoder | 2 - Left Motor <br/> 3 - Right Motor |
| Motor Control | 6 - IN1 <br/> 7 - IN2 <br/> 8 - IN3 <br/> 9 - IN4|
| Motor PWM | 14 - Left Motor <br/> 15 - Right Motor |
| Infrared | 11 - Right Wall Sensor <br/> 26 - Left Wall Sensor <br/> 27 - Barcode Sensor |
| Ultrasonic| 16 - trigPin <br/> 17 - echoPin |

## System Diagrams
### Block Diagram
![System_Diagram](https://github.com/MarkPengJZ/INF2004-Embedded-Systems-Programming/assets/54793197/4858f5d6-7340-4589-ac80-599052760d2f)
### Flow Diagram
![Flowchart](https://github.com/MarkPengJZ/INF2004_T52/assets/54793197/6cec5ece-30dd-4906-b776-99564465e420)


## Members

<table>
  <tr>
    <td><strong>Module</strong></td>
    <td><strong>Members</strong></td>
  </tr>
  <tr>
    <td>Motor</td>
    <td>
    Mark
    </td>
  </tr>
  <tr>
    <td>Ultrasonic &<br/>Accelerometer</td>
    <td>
      Hafizh
    </td>
  </tr>
  <tr>
    <td>Infrared &<br/>Encoder</td>
    <td>
      Koko
    </td>
  </tr>
  <tr>
    <td>Wifi</td>
    <td>
    Elias
    </td>
  </tr>
</table>

## Project Reflections
