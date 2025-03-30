#!/usr/bin/env python3
import socket
import time
import random
from datetime import datetime
from vilib import Vilib
from picarx import Picarx
import threading

# Configuration for trigger reception on the Pi:
CLIENT_IP = "0.0.0.0"         # Listen on all interfaces for trigger
CLIENT_PORT = 6000            # Listening port for trigger messages
BUFFER_SIZE = 1024

# Movement/detection parameters:
POWER = 30
OBSTACLE_AREA_THRESHOLD = 0.4
NO_DETECTION_THRESHOLD = 3

# Global flag and thread for detection loop control:
detection_running = False
detection_thread = None

def compute_object_area(bbox):
    width = bbox[3] - bbox[1]
    height = bbox[2] - bbox[0]
    return width * height

def check_obstacles_from_vision():
    for obj in Vilib.object_detection_list_parameter:
        bbox = obj.get("bounding_box")
        if bbox is not None:
            area = compute_object_area(bbox)
            if area >= OBSTACLE_AREA_THRESHOLD:
                return True
    return False

def send_detection_results():
    # Gather detection information and print it locally.
    detections = Vilib.object_detection_list_parameter
    lines = []
    timestamp = datetime.now().strftime("%H:%M:%S")
    for obj in detections:
        cls = obj.get("class_name", "Unknown")
        bbox = obj.get("bounding_box", [0, 0, 0, 0])
        area = compute_object_area(bbox)
        log_line = f"[{timestamp}] Object Detected: {cls} | Area: {area:.2f}"
        print(log_line)
        lines.append(f"Object Detected: {cls}")
    message = "\n".join(lines) if lines else "Object Detected: None"
    # (You could also send this message to a fixed destination if needed.)
    # For now, we simply print it.
    print("Detection results:", message)

def perform_movement_and_detection():
    global detection_running
    # Start the camera and detection.
    Vilib.camera_start(vflip=False, hflip=False)
    Vilib.show_fps()
    Vilib.display(local=False, web=True)
    time.sleep(1)
    Vilib.object_detect_switch(True)
    px = Picarx()
    no_detection_count = 0

    try:
        while detection_running:
            px.forward(POWER)
            time.sleep(0.5)

            init_angle = random.randint(-10, 10)
            px.set_dir_servo_angle(init_angle)
            time.sleep(0.1)

            for angle in range(init_angle, 35, 1 if init_angle < 35 else -1):
                px.set_dir_servo_angle(angle)
                time.sleep(0.01)
            for angle in range(35, -35, -1):
                px.set_dir_servo_angle(angle)
                time.sleep(0.01)
            for angle in range(-35, 1):
                px.set_dir_servo_angle(angle)
                time.sleep(0.01)

            px.stop()
            time.sleep(1)

            send_detection_results()

            if Vilib.object_detection_list_parameter:
                no_detection_count = 0
            else:
                no_detection_count += 1

            if check_obstacles_from_vision() or no_detection_count >= NO_DETECTION_THRESHOLD:
                px.set_dir_servo_angle(-30)
                px.backward(POWER)
                time.sleep(1.5)
                no_detection_count = 0

            time.sleep(0.1)
    finally:
        px.stop()
        Vilib.camera_close()
        print("Detection loop stopped.")

def start_udp_client():
    global detection_running, detection_thread
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client_socket:
        # Bind to CLIENT_IP and CLIENT_PORT to receive trigger messages.
        client_socket.bind((CLIENT_IP, CLIENT_PORT))
        print(f"UDP client listening on {CLIENT_IP}:{CLIENT_PORT}")

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
