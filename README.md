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
pico
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
- Raspberry Pico
- 1x L298N Motor Driver
- 2x DC Motor
- 3x IR Encoder
- 1x Magnetometer
- 1x HC-SR04 Ultrasonic Sensor

## Pin Layout
| Hardware | GPIO # |
|---|---|
| Encoder | 2 - Left Motor <br/> 3 - Right Motor |
| Accelerometer | 4 - I2C (SDA) <br/> 5 - I2C (SDL) |
| Motor Control | 6 - IN1 <br/> 7 - IN2 <br/> 8 - IN3 <br/> 9 - IN4|
| Motor PWM | 14 - Left Motor <br/> 15 - Right Motor |
| Infrared | 26 - Digital IN |

## Members

<table>
  <tr>
    <td><strong>Module</strong></td>
    <td><strong>Members</strong></td>
  </tr>
  <tr>
    <td>Motor</td>
    <td>
    Hafizh
    </td>
  </tr>
  <tr>
    <td>Ultrasonic &<br/>Accelerometer</td>
  </tr>
  <tr>
    <td>Infrared &<br/>Encoder</td>
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
