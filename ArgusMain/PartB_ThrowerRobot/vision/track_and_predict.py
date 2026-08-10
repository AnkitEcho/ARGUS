"""
track_and_predict.py

Vision system for Part B (the thrower robot).

Watches Part A's plate (marked with a bright, uniquely-coloured sticker),
tracks it in real time via a webcam, predicts where it will be a short
moment from now, and sends AIM/THROW commands to the Arduino running
PartB_Thrower.ino over Serial.

Run: python track_and_predict.py --port /dev/ttyUSB0
"""

import argparse
import collections
import time

import cv2
import numpy as np
import serial

# ------------------------------------------------------------------
# 1. USER CONFIGURATION -- calibrate these for your physical setup
# ------------------------------------------------------------------

# HSV range for the marker colour on the plate.
# Use tools/hsv_picker.py to tune this for your lighting/marker.
HSV_LOWER = np.array([108, 62, 61])
HSV_UPPER = np.array([147, 255, 255])
MIN_MARKER_RADIUS_PX = 4

# Four board-plane reference points: pixels vs. real-world mm coordinates
IMAGE_POINTS = np.array([
    [80, 60],     # top-left
    [560, 60],    # top-right
    [560, 420],   # bottom-right
    [80, 420],    # bottom-left
], dtype=np.float32)

WORLD_POINTS = np.array([
    [-400, 300],
    [400, 300],
    [400, -300],
    [-400, -300],
], dtype=np.float32)

THROWER_ORIGIN = np.array([0.0, -800.0])
THROWER_HEADING_OFFSET_DEG = 90.0

PREDICT_AHEAD_S = 0.6
HISTORY_LEN = 8


def height_and_power_for_target(world_xy):
    """Maps a target point to launcher height (mm) and flywheel PWM (0-255)."""
    dist = np.linalg.norm(world_xy - THROWER_ORIGIN)
    launcher_height_mm = float(np.clip(150 + 0.05 * dist, 0, 300))
    flywheel_pwm = int(np.clip(120 + 0.15 * dist, 0, 255))
    return launcher_height_mm, flywheel_pwm


# ------------------------------------------------------------------
# 2. TRACKING & PREDICTION
# ------------------------------------------------------------------

def detect_marker(frame):
    """Returns (x_px, y_px) of the marker centroid, or None if not found."""
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, HSV_LOWER, HSV_UPPER)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    if not contours:
        return None
    c = max(contours, key=cv2.contourArea)
    (x, y), radius = cv2.minEnclosingCircle(c)
    if radius < MIN_MARKER_RADIUS_PX:
        return None
    return (x, y)


def pixel_to_world(pt_px, homography):
    pt = np.array([[pt_px]], dtype=np.float32)
    world = cv2.perspectiveTransform(pt, homography)
    return world[0][0]


def world_to_pixel(pt_world, inv_homography):
    pt = np.array([[pt_world]], dtype=np.float32)
    px = cv2.perspectiveTransform(pt, inv_homography)
    return px[0][0]


def predict_future_position(history, ahead_s):
    """Linear velocity extrapolation over history samples."""
    if len(history) < 2:
        return None
    t0, x0, y0 = history[0]
    t1, x1, y1 = history[-1]
    dt = t1 - t0
    if dt <= 1e-3:
        return None
    vx = (x1 - x0) / dt
    vy = (y1 - y0) / dt
    return np.array([x1 + vx * ahead_s, y1 + vy * ahead_s])


def compute_aim(world_xy):
    """Predicted world (x, y) -> (azimuth_deg, height_mm, flywheel_pwm)."""
    vec = world_xy - THROWER_ORIGIN
    angle_deg = (np.degrees(np.arctan2(vec[0], vec[1])) + THROWER_HEADING_OFFSET_DEG) % 360
    height_mm, pwm = height_and_power_for_target(world_xy)
    return angle_deg, height_mm, pwm


# ------------------------------------------------------------------
# 3. CALIBRATION HELPER
# ------------------------------------------------------------------

def run_calibration(camera_index):
    points = []
    cap = cv2.VideoCapture(camera_index)

    def on_click(event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN and len(points) < 4:
            points.append((x, y))
            print(f"Point {len(points)}: ({x}, {y})")

    win_name = "Calibrate - click TL, TR, BR, BL, then press q"
    cv2.namedWindow(win_name)
    cv2.setMouseCallback(win_name, on_click)

    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                break
            for p in points:
                cv2.circle(frame, p, 6, (0, 0, 255), -1)
            cv2.imshow(win_name, frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        cap.release()
        cv2.destroyAllWindows()

    if len(points) == 4:
        print("\nIMAGE_POINTS = np.array(", points, ", dtype=np.float32)")
    else:
        print("Calibration incomplete -- need exactly 4 points.")


# ------------------------------------------------------------------
# 4. MAIN LOOP
# ------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--camera", type=int, default=0)
    parser.add_argument("--port", type=str, help="Arduino serial port, e.g. /dev/ttyUSB0 or COM5")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--throw-cooldown", type=float, default=2.0, help="Minimum seconds between throws")
    parser.add_argument("--calibrate", action="store_true", help="Run the corner-click calibration helper")
    args = parser.parse_args()

    if args.calibrate:
        run_calibration(args.camera)
        return

    if not args.port:
        parser.error("--port is required unless --calibrate is used")

    cap = cv2.VideoCapture(args.camera)
    if not cap.isOpened():
        print(f"Error: Could not open camera {args.camera}")
        return

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.05)  # Non-blocking fast timeout
        time.sleep(2)  # Allow Arduino reset cycle
    except serial.SerialException as e:
        print(f"Failed to open serial port: {e}")
        cap.release()
        return

    # Compute forward and inverse homography
    homography, _ = cv2.findHomography(IMAGE_POINTS, WORLD_POINTS)
    inv_homography = np.linalg.inv(homography)

    history = collections.deque(maxlen=HISTORY_LEN)
    last_throw_time = 0.0

    print("Tracking started. Press 'q' in the video window to quit.")
    
    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                print("Camera frame drop detected.")
                break

            marker_px = detect_marker(frame)
            if marker_px is not None:
                world = pixel_to_world(marker_px, homography)
                history.append((time.time(), world[0], world[1]))
                cv2.circle(frame, (int(marker_px[0]), int(marker_px[1])), 8, (0, 255, 0), 2)

            predicted_world = predict_future_position(history, PREDICT_AHEAD_S)
            now = time.time()

            if predicted_world is not None:
                # Project predicted world point back into camera view for visual debugging
                pred_px = world_to_pixel(predicted_world, inv_homography)
                pred_pt = (int(pred_px[0]), int(pred_px[1]))
                cv2.circle(frame, pred_pt, 6, (0, 0, 255), -1)  # Red dot = Future Prediction
                
                if marker_px is not None:
                    cv2.line(frame, (int(marker_px[0]), int(marker_px[1])), pred_pt, (255, 0, 0), 2)

                # Execute Throw Command if Cooldown is passed
                if (now - last_throw_time) > args.throw_cooldown:
                    angle_deg, height_mm, pwm = compute_aim(predicted_world)
                    
                    # Flush buffers and issue aim command
                    ser.reset_input_buffer()
                    ser.write(f"AIM {angle_deg:.1f} {height_mm:.1f} {pwm}\n".encode())
                    
                    # Non-blocking wait loop for Arduino acknowledgment
                    start_wait = time.time()
                    aim_ok = False
                    while (time.time() - start_wait) < 0.5:
                        if ser.in_waiting:
                            resp = ser.readline().decode(errors="ignore").strip()
                            if resp == "AIMED":
                                aim_ok = True
                                break

                    if aim_ok:
                        ser.write(b"THROW\n")
                        last_throw_time = now
                        print(f"[THROW] Aimed: {angle_deg:.1f} deg | Height: {height_mm:.1f} mm | PWM: {pwm}")

            cv2.imshow("Plate Tracker & Predictor", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    finally:
        # Safe shutdown procedure
        try:
            ser.write(b"STOP\n")
            ser.close()
        except Exception:
            pass
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()