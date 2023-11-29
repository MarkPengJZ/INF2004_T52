# INF2004-Embedded-Systems-Programming

## Table of contents

- [INF2004-Embedded-Systems-Programming](#inf2004-embedded-systems-programming)
  - [Table of contents](#table-of-contents)
  - [General Info](#general-info)
    - [Introduction](#introduction)
    - [Hardware](#hardware)
    - [Pin Layout](#pin-layout)
  - [System Diagrams](#system-diagrams)
    - [Block Diagram](#block-diagram)
    - [Flow Diagram](#flow-diagram)
  - [Members](#members)
  - [Project Reflections](#project-reflections)

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

| Hardware      | GPIO #                                                                       |
| ------------- | ---------------------------------------------------------------------------- |
| Encoder       | 2 - Left Motor <br/> 3 - Right Motor                                         |
| Motor Control | 6 - IN1 <br/> 7 - IN2 <br/> 8 - IN3 <br/> 9 - IN4                            |
| Motor PWM     | 14 - Left Motor <br/> 15 - Right Motor                                       |
| Infrared      | 11 - Right Wall Sensor <br/> 26 - Left Wall Sensor <br/> 27 - Barcode Sensor |
| Ultrasonic    | 16 - trigPin <br/> 17 - echoPin                                              |

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
    <td>Ultrasonic &<br/>Magnetometer</td>
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

Upon the project's conclusion, we found ourselves with a partially integrated car, lacking essential mapping and navigation capabilities. While this outcome was disheartening, it served as a catalyst for our team to enhance our understanding of embedded systems and undertake further improvements.

Throughout the implementation phase, our team encountered various challenges. Although suspected, confirmation of potential hardware faults remained elusive. In hindsight, a key revision for a hypothetical project redo would involve prioritizing the initial implementation of a Real-Time Operating System (RTOS). This strategic approach would significantly enhance our ability to integrate more complex functionalities, particularly those related to mapping and navigation.

Despite having a well-conceived theoretical plan, challenges arose during motor calibration, leading to a shortage of time due to frequent troubleshooting. Despite the implementation of PID calibration for the motors, disparities in speed became evident, resulting in visible steering oscillations during forward motion. These variations were attributed to slight differences in friction between the two motors, even under the control of a closed feedback loop.

To address the motor synchronization issue, our proposed solution involves the incorporation of an additional 'C' term in our controller. This fourth term aims to minimize the disparity in encoder counts between the two motors, providing a more refined approach to mitigating speed differentials.

Another challenge emerged with the magnetometer, where fluctuating values were observed even when the sensor was stationary, leading to inaccurate readings and subsequently unreliable directional information. Consequently, the decision was made to forgo the magnetometer due to its unreliable sensor input.

The barcode presented another challenge, particularly in achieving straight driving for proper IR sensor reading. Initially attributing the issue to sensor reading code, we later identified that the car's speed was the determining factor. Adjusting the speed to match the threshold for line detection enabled successful barcode reading.

In summary, our project experience underscores the importance of thorough hardware testing and calibration from the project's inception. The incorporation of an RTOS and the refinement of control algorithms will be pivotal considerations for future projects of similar complexity, ensuring a robust foundation for the implementation of intricate functionalities.