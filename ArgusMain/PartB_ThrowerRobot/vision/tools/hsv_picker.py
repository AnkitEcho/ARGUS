"""
hsv_picker.py

Utility to dynamically find lower and upper HSV threshold values for color segmentation.
Press 'q' to exit and output the configuration tuples.

Run: python tools/hsv_picker.py [--camera 0]
"""

import argparse
import cv2
import numpy as np


def nothing(_):
    pass


def main():
    parser = argparse.ArgumentParser(description="HSV Threshold Calibration Utility")
    parser.add_argument("--camera", type=int, default=0, help="Camera device index")
    args = parser.parse_args()

    cap = cv2.VideoCapture(args.camera)
    if not cap.isOpened():
        print(f"Error: Could not open camera at index {args.camera}.")
        return

    cv2.namedWindow("Controls", cv2.WINDOW_AUTOSIZE)

    # OpenCV Hue max is 179; Saturation and Value max are 255
    trackbars = [
        ("H Min", 5, 179),
        ("H Max", 15, 179),
        ("S Min", 150, 255),
        ("S Max", 255, 255),
        ("V Min", 150, 255),
        ("V Max", 255, 255),
    ]

    for name, default, max_val in trackbars:
        cv2.createTrackbar(name, "Controls", default, max_val, nothing)

    print("--- HSV Picker Started ---")
    print("Adjust sliders in the 'Controls' window.")
    print("Press 'q' to quit and export array parameters.\n")

    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                print("Failed to grab frame from camera.")
                break

            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

            lower = np.array([
                cv2.getTrackbarPos("H Min", "Controls"),
                cv2.getTrackbarPos("S Min", "Controls"),
                cv2.getTrackbarPos("V Min", "Controls"),
            ])

            upper = np.array([
                cv2.getTrackbarPos("H Max", "Controls"),
                cv2.getTrackbarPos("S Max", "Controls"),
                cv2.getTrackbarPos("V Max", "Controls"),
            ])

            mask = cv2.inRange(hsv, lower, upper)
            result = cv2.bitwise_and(frame, frame, mask=mask)

            cv2.imshow("Original Frame", frame)
            cv2.imshow("Mask", mask)
            cv2.imshow("Filtered Result", result)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                print("--- Final Parameters ---")
                print(f"HSV_LOWER = np.array({lower.tolist()})")
                print(f"HSV_UPPER = np.array({upper.tolist()})")
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()