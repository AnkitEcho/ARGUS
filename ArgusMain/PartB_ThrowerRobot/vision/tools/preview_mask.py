"""
preview_mask.py

Quick utility to preview the HSV mask using your saved HSV_LOWER and HSV_UPPER
thresholds from track_and_predict.py.
"""

import argparse
import cv2
import numpy as np

# Paste your calibrated values from track_and_predict.py here:
HSV_LOWER = np.array([108, 62, 61])
HSV_UPPER = np.array([147, 255, 255])


def main():
    parser = argparse.ArgumentParser(description="Preview Saved HSV Mask")
    parser.add_argument("--camera", type=int, default=0, help="Camera device index")
    args = parser.parse_args()

    cap = cv2.VideoCapture(args.camera)
    if not cap.isOpened():
        print(f"Error: Could not open camera at index {args.camera}.")
        return

    print("--- Displaying Saved HSV Threshold Preview ---")
    print("Showing: Original, Binary Mask, and Filtered Color Result.")
    print("Press 'q' in any window to exit.\n")

    try:
        while True:
            ok, frame = cap.read()
            if not ok:
                print("Failed to capture video frame.")
                break

            # 1. Convert frame to HSV space
            hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

            # 2. Create the binary mask using saved thresholds
            mask = cv2.inRange(hsv, HSV_LOWER, HSV_UPPER)

            # 3. Apply optional noise reduction (same as track_and_predict.py)
            mask_clean = cv2.erode(mask, None, iterations=2)
            mask_clean = cv2.dilate(mask_clean, None, iterations=2)

            # 4. Extract full color object using bitwise operation
            filtered_result = cv2.bitwise_and(frame, frame, mask=mask_clean)

            # 5. Render windows
            cv2.imshow("1. Original Camera Feed", frame)
            cv2.imshow("2. Binary Mask (Saved HSV)", mask_clean)
            cv2.imshow("3. Isolated Object View", filtered_result)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()