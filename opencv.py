import cv2
import serial
import time
from picamera2 import Picamera2

picam2 = Picamera2()
picam2.start()

qr_scanned = False
qr = cv2.QRCodeDetector()

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser.reset_input_buffer()

prev_state = []

# actions:
# STOP - stops the vehicle
# F5 - go straight with 5 more
# L10 - go to left for 10 more
# R20 - go to right for 20 more
# forward is 0 or 1, STOP and FORWORD, respectively
# direction is -1, 0, 1, LEFT, STRAIGHT, RIGHT, respectively
# amount is literally the amount
def send_serial(forward, direction, amount):
    global prev_state

    string = ''
    prev_state = [direction, amount]

    if forward == 0:
        string = 'S0'
    else:
        if direction == -1:
            string += 'L'
        elif direction == 0:
            string += 'F'
        elif direction == 1:
            string += 'R'
        else:
            return
        string += str(amount)
    string += '\n'
    # debug
    print(string)

    ser.write(string.encode())

def send_last():
    send_serial(1, prev_state[0], prev_state[1])

# returns true when qr is scanned
def scan_qr(gray):
    global qr_scanned
    
    if qr_scanned:
        return False

    try:
        ret, _ = qr.detect(gray)
    except:
        pass

    if ret:
        qr_scanned = True
        return True
    return False

def process_frame(frame):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, binary = cv2.threshold(gray, 60, 255, cv2.THRESH_BINARY_INV)

    if scan_qr(gray):
        send_serial(0, 0, 0)
        time.sleep(3)
        return (None, None)
    
    h, w = binary.shape
    cell_h = h // 3

    centers = []
    for row in range(3):
        y1 = row * cell_h
        y2 = (row + 1) * cell_h
        x1 = 0
        x2 = w

        cell = binary[y1:y2, x1:x2]
        M = cv2.moments(cell, binaryImage=True)

        if (M['m00'] > 100):
            cx = int(M['m10'] / M['m00'])
            cy = int(M['m01'] / M['m00'])

            abs_x = x1 + cx
            abs_y = y1 + cy

            centers.append((abs_x, abs_y))
    return binary, centers

def get_total(cen):
    c = 640 // 2
    h = 480
    total = 0
    weight = [[4.5], [2.0, 1.25], [1.2, 0.9, 0.8]]

    for i in range(len(cen)):
        total += (cen[i][0] - c) * weight[len(cen) - 1][i]

    return total

# cen is ordered from top to bottom
def drive(shape, cen):
    divisor = 5.0
    send = 0
    send_straight = 40

    total = get_total(cen)
    send = int(total / divisor)

    if send > 0:
        send_serial(1, 1, send)
    elif send == 0:
        send_serial(1, 0, send_straight)
    else:
        send_serial(1, -1, -send)

if __name__ == '__main__':
    i = 0
    while True:
        frame = picam2.capture_array()
        binary, cen = process_frame(frame)
        if binary is None:
            continue
        if cen is None:
            send_serial(0, 0, 0)
            continue
        drive(binary.shape, cen)

        # debug
        for c in cen:
            cv2.circle(frame, c, 5, (200, 200, 200, 255), -1)
        cv2.imwrite(f'out/out{i}.png', frame)
        i += 1
        print(f'Image{i} saved!')
