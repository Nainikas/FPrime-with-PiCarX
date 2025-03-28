## Prime-with-PiCarX
Integrating PiCar-X capabilities with Object Detection and Autonomous Movement using FPrime

### Meeting the System Requirements using Raspberry Pi 5 for FPrime
1. Downloading the bootstraping tool for F'
   ```pip install fprime-bootstrap -- installs fprime bootstrap ```
2. Creating a project
   ```fprime-bootstrap project -- naming it as PiCar-UDP-FPrime ```
3. Downloading VS Plugins to help write the F' code
   -FPP
   -FPPTools

### Project Setup
Activate virtual environment and generate a build cache.
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
| SYSTEM-001      | Integrate camera and motor control within the FPrime framework.                           |
| SYSTEM-002      | Provide real-time telemetry and status logging.            |
| SYSTEM-003      | Enable remote triggering via UDP for object detection.            |

---

### Software Requirements

| **Requirement** | **Description**                                                                                                                        |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------|
| CAM-001         | The software shall capture images using the camera when requested and report the capture status via telemetry.                           |
| MOT-001         | Control motor movement based on detection results.                                              |

### PiCar-X Requirements

Here we list a number of requirements for the PiCar-X software to implement.

| **Requirement**      | **Description**                                                                                               | **Derived From**                         | **Verification**  |
|----------------------|---------------------------------------------------------------------------------------------------------------|------------------------------------------|-------------------|
| PICAR-CAM-001       | The software shall capture an image upon receiving a capture command.                                        | SYSTEM & SW Requirements                 | Unit Test         |
| PICAR-MOT-001       | The software shall execute drive commands to control motor speed and direction.                                | SYSTEM & SW Requirements                 | Unit Test         |
| PICAR-US-001        | Continuously monitor camera readings to detect obstacles in the vehicle's path.                           | SYSTEM & SW Requirements                 | Unit Test         |
| PICAR-US-002        | Generate obstacle avoidance commands when obstacles are detected within a predefined threshold.            | SYSTEM & SW Requirements                 | Unit Test         |
| PICAR-INT-001       | Integrate camera and motor modules within FPrime for seamless communication and telemetry. | SYSTEM & SW Requirements                 | Integration Test  |

[!NOTE] Notice how the software also includes a requirement derived from the Electrical Interface Control Document. This captures the details of the software/hardware interface and is captured here as a requirement.

#### PiCar-X: Component Design and Initial Implementation
This section discussses the design of the component, the implementaion of a command to start/stop the PiCar-X to move and detect objects. 

### Component Design

For autonomous operation, PiCar-X uses FPrime components to manage motor movements, camera operations, and object detection via the Robot HAT library (no direct GPIO/I2C interactions).

The PiCar-X system will contain the component:

- ObjectDetector:
  - Custom UDP-based component.
  - Sends triggers via UDP (1 to start, 0 to stop) to PiCar-X (IP: 192.168.1.99, Port: 6000).
  - Receives UDP detection logs and records them as FPrime events.

The component will send Robot HAT commands via Python scripts rather than interacting with GPIO and I2C.

Communication within the system leverages FPrime ports:

- Commands: Initiate actions (move, stop).

- Telemetry: Report statuses.

- Events: Log system events.

- Parameters: Adjust operational settings dynamically.

# Supported Commands
| **Command**       | **Description**                                 | **UDP Message** | **PiCar-X IP:Port** |
|-------------------|-------------------------------------------------|-----------------|---------------------|
| `StartDetection`  | Initiate object detection and PiCar-X movement. | `"1"`           | `192.168.1.99:6000` |
| `StopDetection`   | Stop object detection and halt PiCar-X movement.| `"0"`           | `192.168.1.99:6000` |


This component design is captured in the block diagram below.

![image](https://github.com/user-attachments/assets/5783c9f5-3e97-4edc-b1ba-ac30e16e05ef)

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
# ObjectDetector Component
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

## Define Component Commands, Events, Telemetry in '.fpp' Files

The component now has a directory in 'Components/'. Open the '.fpp' file inside its directory.

### 1. To generate the implemention files after defining the '.fpp' files, run:
```
fprime-util impl
```

This will generate:
- ObjectDetector.template.cpp and ObjectDetector.template.hpp

Rename them by removing '.template' from them:
```
mv ObjectDetector.template.cpp ObjectDetector.cpp
mv ObjectDetector.template.hpp ObjectDetector.hpp

```

### 2. Build and Validate the files

Run:
```
fprime-util build
```
If everything compiles correctly, your FPrime component is now ready to integrate into the system topology.

## Create a deployment
Go to the terminal, navigate to the project's root directory and run 
```
# In PiCar-X
cd 
fprime-util new --deployment
```

You will be prompted for the information regarding your component. Fill out the prompts as shown below:
# ObjectDetector Deployment
```
[INFO] Cookiecutter source: using builtin
  [1/2] Deployment name (MyDeployment): MyDeployment
  [2/2] Select communication driver type
    1 - TcpClient
    2 - TcpServer
    3 - UART
    Choose from [1/2/3] (1): 1
[INFO] Found CMake file at 'PiCarUDP/project.cmake'
Add MyDeployment to PiCarUDP/project.cmake at end of file? (yes/no) [yes]:yes
```

Edit the instances.fpp and topology.fpp files to include the objectDetector instance and connections for MyDeployment.

Once saved, use
```
fprime-util build
```

If built, with no errors, proceed to use Ground Data System using
```
fprime-util gds --ip-port 6000
```

# Run the PiCar standalone Python Script

1. In another terminal, run the Python Script used to detect objects while moving. It will wait and listen to the port 6000.
2. In the F' UI, navigate to StartDetection and enter the trigger '1' for the PiCar-X to move and detect objects.
3. Observe the detections that uses YOLO11. Refer to https://docs.ultralytics.com/models/yolo11/#overview for more details on the model.
4. In the F' UI, navigate to StopDetection and enter the trigger '0' for the PiCar-X to stop movement and detecting objects.
