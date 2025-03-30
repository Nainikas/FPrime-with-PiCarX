#!/usr/bin/env python3
import socket
import time
import random
from datetime import datetime
import threading
import cv2
from picamera2 import Picamera2
from ultralytics import YOLO
from picarx import Picarx

# UDP server configuration
CLIENT_IP = "0.0.0.0"
CLIENT_PORT = 6000
BUFFER_SIZE = 1024

# Movement/detection parameters
POWER = 30
NO_DETECTION_THRESHOLD = 3
PAN_MAX = 30
PAN_STEP = 5
PAN_DELAY = 0.2
BACKUP_THRESHOLD = 3
TOO_CLOSE_AREA_THRESHOLD = 0.25

# Global control variables
detection_running = False
detection_thread = None
detection_results = []

movement_modes = ["forward", "backward", "right", "left"]
movement_index = 0

def compute_normalized_area(bbox, frame_width, frame_height):
    width = bbox[2] - bbox[0]
    height = bbox[3] - bbox[1]
    return (width * height) / (frame_width * frame_height)

def check_obstacles_from_vision(frame_width, frame_height):
    for obj in detection_results:
        bbox = obj.get("bounding_box")
        if bbox is not None:
            norm_area = compute_normalized_area(bbox, frame_width, frame_height)
            print(f"Normalized Area: {norm_area:.2f}")
            if norm_area >= TOO_CLOSE_AREA_THRESHOLD:
                return True
    return False

def perform_action(px, mode):
    print(f"Attempting to move: {mode}")
    if mode == "forward":
        px.set_dir_servo_angle(0)
        px.forward(POWER)
    elif mode == "backward":
        px.set_dir_servo_angle(random.choice([-30, 30]))
        px.backward(POWER)
    elif mode == "right":
        px.set_dir_servo_angle(30)
        px.forward(POWER)
    elif mode == "left":
        px.set_dir_servo_angle(-30)
        px.forward(POWER)

def perform_movement_and_detection():
    global detection_running, detection_results, movement_index

    picam2 = Picamera2()
    picam2.preview_configuration.main.size = (1280, 720)
    picam2.preview_configuration.main.format = "RGB888"
    picam2.preview_configuration.align()
    picam2.configure("preview")
    picam2.start()

    model = YOLO("yolo11n.pt")
    px = Picarx()

    no_detection_count = 0
    failed_scan_cycles = 0
    cam_pan_angle = 0
    pan_direction = 1

    try:
        while detection_running:
            px.set_cam_pan_angle(cam_pan_angle)
            px.set_cam_tilt_angle(0)

            frame = picam2.capture_array()
            if frame is None:
                print("Frame capture failed.")
                continue

            frame_height, frame_width, _ = frame.shape
            # Do not convert the frame before detection; keep original RGB888 format
            results = model(frame)
            detection_results = []

            for result in results:
                for box in result.boxes:
                    coords = box.xyxy[0].tolist()
                    cls_id = int(box.cls[0])
                    label = model.names[cls_id] if hasattr(model, 'names') else str(cls_id)
                    detection_results.append({
                        "bounding_box": coords,
                        "class_name": label
                    })

            try:
                annotated_frame = results[0].plot()
                cv2.imshow("Camera", annotated_frame)
                cv2.waitKey(1)
            except:
                pass

            obstacle_close = check_obstacles_from_vision(frame_width, frame_height)

            if detection_results and not obstacle_close:
                no_detection_count = 0
                failed_scan_cycles = 0
                cam_pan_angle = 0
                pan_direction = 1
                px.set_cam_pan_angle(0)
                perform_action(px, "forward")
                time.sleep(0.5)
            else:
                px.stop()
                time.sleep(0.2)
                no_detection_count += 1

                if obstacle_close:
                    print("Obstacle too close. Trying alternative path.")
                else:
                    print("No detections. Waiting for scene to clear.")

                time.sleep(0.5)
                failed_scan_cycles += 1

                if failed_scan_cycles >= BACKUP_THRESHOLD:
                    movement_index = (movement_index + 1) % len(movement_modes)
                    selected_mode = movement_modes[movement_index]
                    perform_action(px, selected_mode)
                    time.sleep(1.5 if selected_mode != "backward" else 3)
                    px.stop()
                    failed_scan_cycles = 0
                    no_detection_count = 0
                    cam_pan_angle = 0
                    pan_direction = 1

            time.sleep(0.1)

    finally:
        px.stop()
        try:
            picam2.stop()
        except Exception as e:
            print("Error stopping camera:", e)
        cv2.destroyAllWindows()
        print("Detection loop stopped.")

def start_udp_client():
    global detection_running, detection_thread
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client_socket:
        client_socket.bind((CLIENT_IP, CLIENT_PORT))
        print(f"UDP server listening on {CLIENT_IP}:{CLIENT_PORT}")

        while True:
            data, server_address = client_socket.recvfrom(BUFFER_SIZE)
            message = data.decode().strip()
            print(f"Received message from {server_address}: {message}")
            if message == "1":
                if not detection_running:
                    print("Trigger received. Starting detection loop...")
                    detection_running = True
                    detection_thread = threading.Thread(target=perform_movement_and_detection)
                    detection_thread.start()
                else:
                    print("Detection already running.")
            elif message == "0":
                if detection_running:
                    print("Stop trigger received. Stopping detection loop...")
                    detection_running = False
                    detection_thread.join()
                    detection_thread = None
                else:
                    print("Detection is not running.")
            else:
                print("Unknown trigger message received.")

if __name__ == "__main__":
    start_udp_client()
