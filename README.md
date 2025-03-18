##### FPrime-with-PiCarX
Integrating PiCar-X compabilities with Object Detection and Movement to launch with FPrime

### Meeting the System Requirements using Raspberry Pi 5 for FPrime
1. Downloading the bootstraping tool for F'
   ```pip install fprime-bootstrap -- installs fprime bootstrap ```
2. Creating a project
   ```fprime-bootstrap project -- naming it as PiCar-x-FPrime ```
3. Downloading VS Plugins to help write the F' code
   -FPP
   -FPPTools

### Project Setup
Generate a build cache
```
cd PiCar-x-FPrime
. fprime-venv/bin/activate
fprime-util generate
```
#### Specifying Requirements
### System Requirements
We need high level system requirements. These requirements would be defined by requirements sepcified by the electronic subsystem which are themselves derived by requirements defined at the full system level.

| **Requirement** | **Description**                                                                                                                       |
|-----------------|---------------------------------------------------------------------------------------------------------------------------------------|
| SYSTEM-001      | The system shall integrate the camera, motor, and ultrasonic sensor components using the F Prime framework.                           |
| SYSTEM-002      | The system shall report telemetry data for the statuses of the camera, motor, and ultrasonic sensor components in real time.            |

---

### Software Requirements

| **Requirement** | **Description**                                                                                                                        |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------|
| CAM-001         | The software shall capture images using the camera when requested and report the capture status via telemetry.                           |
| IMG-002         | The software shall process captured images to detect objects and provide detection results.                                             |
| MOT-003         | The software shall execute drive and stop commands for motor control based on system inputs.                                              |
| MOT-004         | The software shall control the motor using GPIO pin 13.                                                                                |
| US-001          | The software shall continuously monitor ultrasonic sensor readings to detect obstacles in the vehicle's path.                             |
| US-002          | The software shall generate obstacle avoidance commands when an obstacle is detected within a predefined threshold distance.              |
| INT-005         | The software shall integrate the camera, motor, and ultrasonic sensor modules within the F Prime framework for seamless communication and telemetry logging. |
| ERR-006         | The software shall implement error handling and logging for the camera, motor, and ultrasonic sensor modules to facilitate debugging.   |


### PiCar-X Requirements

Here we list a number of requirements for the PiCar-X software to implement.

| **Requirement**      | **Description**                                                                                               | **Derived From**                         | **Verification**  |
|----------------------|---------------------------------------------------------------------------------------------------------------|------------------------------------------|-------------------|
| PICARX-CAM-001       | The software shall capture an image upon receiving a capture command.                                        | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-CAM-002       | The software shall telemeter the status of camera capture operations.                                          | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-001       | The software shall execute drive commands to control motor speed and direction.                                | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-002       | The software shall stop the motors immediately upon receiving a stop command.                                | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-MOT-003       | The software shall control the motor using GPIO pin 13 as specified in the Electrical Interface Control Document. | SYSTEM, SW Requirements & Electrical ICD | Unit Test         |
| PICARX-US-001        | The software shall continuously monitor ultrasonic sensor readings to detect obstacles in the vehicle's path.                           | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-US-002        | The software shall generate obstacle avoidance commands when an obstacle is detected within a predefined threshold distance.            | SYSTEM & SW Requirements                 | Unit Test         |
| PICARX-INT-001       | The software shall integrate the camera and motor modules within the F Prime framework for seamless communication and telemetry logging. | SYSTEM & SW Requirements                 | Integration Test  |

[!NOTE] Notice how the software also includes a requirement derived from the Electrical Interface Control Document (PICARX-MOT-003). This captures the details of the software/hardware interface and is captured here as a requirement.

#### PiCar-X: Component Design and Initial Implementation
This section discussses the design of the component, the implementaion of a command to start/stop the PiCar-X to move and detect objects, and the sending of events. First, proceed to initial ground testing before finishing the implementation in the later sections.

### Component Design

In order for the PiCar-X to move and detect objects autonomously, it needs to accept commands for movement and obstacle avoidance, and control the motors and sensors using the Robot HAT library, which abstracts the lower-level hardware interfaces. The system leverages F Prime ports to communicate with the motor, camera, and ultrasonic sensor modules. A rate group input port is used to schedule periodic tasks—such as updating movement commands and polling sensor data.

The PiCar-X system will contain three key components:

- MotorController: Handles moving forward, stopping, and speed control.
- CameraHandler: Captures images and logs image capture status.
- UltrasonicSensor: Reads distance to obstacles and reports data to telemetry.
Each component will send Robot HAT commands via Python scripts rather than interacting with GPIO/I2C.

Additionally, standard F Prime ports are defined for:

- Commands: To trigger actions like moving the vehicle, stopping, capturing an image, or reading sensor data.
- Telemetry: To report the status of each component, including motor speed, obstacle detection, and camera capture status.
- Events: To log significant system actions, such as movement state changes, image capture events, and detected obstacles.
- Parameters: To allow runtime adjustments of settings like motor speed, obstacle detection threshold, and camera capture mode.
- ExecPython: A dedicated port for invoking Python scripts, which interact with Robot HAT APIs for motor, camera, and sensor control.

This component design is captured in the block diagram below, with input ports on the left and output ports on the right. Ports for standard F Prime functions (e.g., commands, events, telemetry, and parameters) are highlighted in green, while the ExecPython port (used for triggering external scripts) is specially integrated.

![image](https://github.com/user-attachments/assets/6e339eb3-4826-4b8b-9005-7ae682a8c159)

#### Design Summary

### Component Ports
- run: Invoked at a set rate from the rate group, used to control movement, camera panning, and obstacle detection.
- execPython: Triggers external Python scripts that interface with the Robot HAT library to execute movement, object detection, and obstacle avoidance.
- Standard component ports (circled in green) are not explicitly listed here.

---

### **Commands**
| **Component**        | **Command**                  | **Function** |
|---------------------|----------------------------|-------------|
| **MotorController** | `FORWARD`                   | Moves **forward** at a set speed. |
|                     | `BACKWARD`                  | Moves **backward** at a set speed. |
|                     | `STOP`                      | Stops the PiCar-X. |
|                     | `SET_DIR_SERVO_ANGLE`       | Adjusts **steering direction** angle. |
| **CameraHandler**   | `SET_CAM_PAN_ANGLE`         | Moves the **camera left/right**. |
|                     | `SET_CAM_TILT_ANGLE`        | Moves the **camera up/down**. |
| **UltrasonicSensor**| `READ_ULTRASONIC`           | Reads **distance to objects**. |

---

### **Events**
| **Component**        | **Event**                 | **Description** |
|---------------------|-------------------------|----------------|
| **MotorController** | `MotorStateChanged`      | Triggered when PiCar-X **starts, stops, or changes direction**. |
|                     | `SteeringAdjusted`       | Triggered when the **steering direction changes**. |
| **CameraHandler**   | `CameraPanned`           | Triggered when the **camera moves left or right**. |
|                     | `CameraTilted`           | Triggered when the **camera moves up or down**. |
| **UltrasonicSensor**| `ObstacleDetected`       | Triggered when an **obstacle is detected**. |

---

### **Telemetry Channels**
| **Component**        | **Telemetry Channel**    | **Description** |
|---------------------|------------------------|----------------|
| **MotorController** | `MotorSpeed`           | Reports **motor speed and direction**. |
|                     | `SteeringAngle`        | Reports **current steering angle**. |
| **CameraHandler**   | `CameraPanAngle`       | Reports **current camera pan angle**. |
| **CameraTiltAngle** | `CameraTiltAngle`      | Reports **current camera tilt angle**. |
| **UltrasonicSensor**| `ObstacleDistance`     | Reports **distance to the nearest obstacle**. |

---

### **Parameters**
| **Component**        | **Parameter**          | **Description** |
|---------------------|----------------------|----------------|
| **MotorController** | `MOTOR_SPEED`        | Adjustable **motor speed**. |
| **CameraHandler**   | `PAN_INTERVAL`       | Time **between camera panning movements** (default: 0.5s). |
| **UltrasonicSensor**| `OBSTACLE_THRESHOLD` | Defines **distance threshold** for obstacle avoidance. |

---

## **ExecPython: Running Individual Python Scripts for Each Component**
Instead of writing a single **integrated Python script**, we will **let FPrime execute three separate scripts simultaneously**:

| **Component**        | **ExecPython Script**   | **Function** |
|---------------------|----------------------|-------------|
| **MotorController** | `movement.py`        | Handles **movement, speed, and direction**. |
| **CameraHandler**   | `object_detection.py` | Controls **camera panning and tilting**. |
| **UltrasonicSensor**| `avoiding_obstacles.py` | Manages **obstacle detection and avoidance**. |

By calling all three scripts in parallel, FPrime will ensure that movement, camera tracking, and obstacle detection happen simultaneously without needing a combined script.

## Install Robot Hat and its necessary dependencies

1. Connect to WiFi using the wpa_supplicant.config file.
2. Enable I2C on Raspberry Pi.
3. Install all modules for Robot Hat

## Create the components

Go to the terminal, navigate to the project's root directory and run 
```
# In PiCar-X
cd Components
fprime-util new --component
```

You will be prompted for the information regarding your component. Fill out the prompts as shown below:
# 1. MotorController Component
```
[INFO] Cookiecutter source: using builtin
  [1/8] Component name (MyComponent): MotorController
  [2/8] Component short description (Component for F Prime FSW framework.): Controls movement using Robot Hat
  [3/8] Component namespace (Components): Components
  [4/8] Select component kind
    1 - active
    2 - passive
    3 - queued
    Choose from [1/2/3] (1): 1
  [5/8] Enable Commands?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [6/8] Enable Telemetry?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [7/8] Enable Events?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [8/8] Enable Parameters?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
[INFO] Found CMake file at 'PiCar-X/Components/CMakeLists.txt'
Add MotorController to PiCar-X/Components/CMakeLists.txt at end of file? (yes/no) [yes]: yes
Generate implementation files? (yes/no) [yes]: yes
Refreshing cache and generating implementation files...
[INFO] Created new component and generated initial implementations.
```
# 2. CameraHandler Component
```
[INFO] Cookiecutter source: using builtin
  [1/8] Component name (MyComponent): CameraHandler
  [2/8] Component short description (Component for F Prime FSW framework.): Handles camera panning and tilting
  [3/8] Component namespace (Components): Components
  [4/8] Select component kind
    1 - active
    2 - passive
    3 - queued
    Choose from [1/2/3] (1): 1
  [5/8] Enable Commands?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [6/8] Enable Telemetry?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [7/8] Enable Events?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [8/8] Enable Parameters?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
[INFO] Found CMake file at 'PiCar-X/Components/CMakeLists.txt'
Add CameraHandler to PiCar-X/Components/CMakeLists.txt at end of file? (yes/no) [yes]: yes
Generate implementation files? (yes/no) [yes]: yes
Refreshing cache and generating implementation files...
[INFO] Created new component and generated initial implementations.
```

# 3. UltrasonicSensor Component
```
[INFO] Cookiecutter source: using builtin
  [1/8] Component name (MyComponent): UltrasonicSensor
  [2/8] Component short description (Component for F Prime FSW framework.): Handles obstacle detection and avoidance using ultrasonic sensor
  [3/8] Component namespace (Components): Components
  [4/8] Select component kind
    1 - active
    2 - passive
    3 - queued
    Choose from [1/2/3] (1): 1
  [5/8] Enable Commands?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [6/8] Enable Telemetry?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [7/8] Enable Events?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
  [8/8] Enable Parameters?
    1 - yes
    2 - no
    Choose from [1/2] (1): 1
[INFO] Found CMake file at 'PiCar-X/Components/CMakeLists.txt'
Add UltrasonicSensor to PiCar-X/Components/CMakeLists.txt at end of file? (yes/no) [yes]: yes
Generate implementation files? (yes/no) [yes]: yes
Refreshing cache and generating implementation files...
[INFO] Created new component and generated initial implementations.
```

## Define Component Commands, Events, Telemetry in '.fpp' Files

Each component now has a directory in 'Components/'. Open each '.fpp' file inside its directory.

### 1. To generate the implemention files after defining the '.fpp' files, run:
```
fprime-util impl
```

This will generate:
- MotorController.template.cpp and MotorController.template.hpp
- CameraHandler.template.cpp and CameraHandler.template.hpp
- UltrasonicSensor.template.cpp and UltrasonicSensor.template.hpp

Rename them by removing '.template' from them:
```
mv MotorController.template.cpp MotorController.cpp
mv MotorController.template.hpp MotorController.hpp

mv CameraHandler.template.cpp CameraHandler.cpp
mv CameraHandler.template.hpp CameraHandler.hpp

mv UltrasonicSensor.template.cpp UltrasonicSensor.cpp
mv UltrasonicSensor.template.hpp UltrasonicSensor.hpp
```

### 2. Build and Validate the files

Run:
```
fprime-util build
```
If everything compiles correctly, your FPrime components are now ready to integrate into the system topology.

### 3. 
