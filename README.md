# FPrime-with-PiCarX
Integrating PiCar-X compabilities with Object Detection and Movement to launch with FPrime

## Meeting the System Requirements using Raspberry Pi 5 for FPrime
1. Downloading the bootstraping tool for F'
   ```pip install fprime-bootstrap -- installs fprime bootstrap ```
2. Creating a project
   ```fprime-bootstrap project -- naming it as PiCarX ```
3. Downloading VS Plugins to help write the F' code
   -FPP
   -FPPTools

## Project Setup
Generate a build cache
```
cd led-blinker
. fprime-venv/bin/activate
fprime-util generate
```
## Specifying Requirements
# System Requirements
We need high level system requirements. These requirements would be defined by requirements sepcified by the electronic subsystem which are themselves derived by requirements defined at the full system level.

| **Requirement** | **Description** |
|-----------------|-----------------|
| SYSTEM-001      | The system shall integrate the camera and motor components using the F Prime framework. |
| SYSTEM-002      | The system shall report telemetry data for both camera and motor statuses in real time. |

# Software Requirements

| **Requirement** | **Description** |
|-----------------|-----------------|
| CAM-001         | The system shall capture images using the camera when requested and report the capture status via telemetry. |
| IMG-002         | The system shall process captured images to detect objects and provide detection results. |
| MOT-003         | The system shall execute drive and stop commands for motor control based on system inputs. |
| INT-004         | The system shall integrate the camera and motor control modules within the F Prime framework for seamless communication and telemetry logging. |
| ERR-005         | The system shall implement error handling and logging for both the camera and motor modules to facilitate debugging. |

# PiCar-X Requirements

Here we list a number of requirements for our PiCar-X software to implement.

| **Requirement**      | **Description**                                                                                               | **Derived From**                         | **Verification**  |
|----------------------|---------------------------------------------------------------------------------------------------------------|------------------------------------------|-------------------|
| PICARX-CAM-001       | The software shall capture an image upon receiving a capture command.                                        | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-CAM-002       | The software shall telemeter the status of camera capture operations.                                          | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-001       | The software shall execute drive commands to control motor speed and direction.                                | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-002       | The software shall stop the motors immediately upon receiving a stop command.                                | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-003       | The software shall control the motor using GPIO pin 13 as specified in the Electrical Interface Control Document. | SYSTEM, SW Requirements & Electrical ICD | Unit Test         |
| PICARX-INT-001       | The software shall integrate the camera and motor modules within the F Prime framework for seamless communication and telemetry logging. | SYSTEM & SW Requirements                 | Integration Test  |

[!NOTE] Notice how the software also includes a requirement derived from the Electrical Interface Control Document (PICARX-MOT-003). This captures the details of the software/hardware interface and is captured here as a requirement.

