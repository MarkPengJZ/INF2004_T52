# INF2004-Embedded-Systems-Programming

## Table of contents
* [General Info](#general-info)
* [Hardware](#hardware)
* [Pin Layout](#pin-layout)
* [Software](#software)
* [System Diagrams](#system-diagrams)
* [Members](#members)

## General Info
### Introduction
INF2004 Team 52 Robot Car Project
###Directory Structure
A CMake Project containing our source code. Divided into sub-folders each containing one sub-module, depicted below.
```bash
driver 
├── magnetometer
├── wifi
├── encoder
├── infrared
├── motor (PWM + PID controller)
├── navigation (mapping + shortest route)
├── ultrasonic
└── main.c (program entry point)
```

A `README.md` can be found in each sub-module folder with explaination for each module. 

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
| Magnetometer | |
| Motor Control | 6 - IN1 <br/> 7 - IN2 <br/> 8 - IN3 <br/> 9 - IN4|
| Motor PWM | 14 - Left Motor <br/> 15 - Right Motor |
| Infrared | 26 - Digital IN |
| Ultrasonic| |

## System Diagrams
### System Diagram
![System_Diagram](https://github.com/MarkPengJZ/INF2004-Embedded-Systems-Programming/assets/54793197/4858f5d6-7340-4589-ac80-599052760d2f)
### Flowchart
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
  <tr>
    <td>Mapping &<br/>Navigation</td>
  </tr>
</table>
