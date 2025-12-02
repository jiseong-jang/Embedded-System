# 🏎️ Vision-based Autonomous Line Tracer

**Raspberry Pi(Vision)**와 **Arduino(Motor Control)**를 연동하여 자율 주행 및 QR 코드 인식 정지 기능을 수행하는 라인트레이서 프로젝트입니다.

## 📋 프로젝트 개요
이 프로젝트는 컴퓨터 비전(Computer Vision) 기술을 활용하여 주행 경로를 실시간으로 인식하고, 인식된 정보를 바탕으로 DC 모터를 제어하여 라인을 따라 자율 주행하는 시스템을 구현합니다. [cite_start]또한, 주행 중 QR 코드를 인식하면 비상 정지(Emergency Stop)하는 기능을 포함합니다. [cite: 5, 6]

---

## 🛠️ 기술 스택 (Tech Stack)

### H/W
* [cite_start]**Main Controller:** Raspberry Pi (with Picamera2) [cite: 9]
* [cite_start]**Sub Controller:** Arduino Uno [cite: 10]
* [cite_start]**Actuator:** DC Motors (x2), L298N Motor Driver [cite: 10]

### S/W
* **Language:** Python (RPi), C++ (Arduino)
* [cite_start]**Library:** OpenCV (Image Processing), PySerial (Communication) [cite: 9]

---

## ⚙️ 시스템 아키텍처 (System Architecture)

[cite_start]시스템은 크게 **영상 처리부(Eye/Brain)**와 **모터 제어부(Hand/Foot)**로 나뉩니다. [cite: 8]

1.  **Raspberry Pi (Master):**
    * Picamera를 통해 실시간 도로 영상을 획득합니다.
    * OpenCV를 이용해 라인의 위치(무게 중심)를 계산합니다.
    * [cite_start]상황에 맞는 조향(Steering) 및 속도 명령을 생성하여 UART 시리얼로 전송합니다. [cite: 9]

2.  **Arduino (Slave):**
    * 라즈베리파이로부터 문자열 형태의 명령어를 수신 및 파싱합니다.
    * [cite_start]L298N 드라이버를 통해 양쪽 모터의 PWM을 제어하여 차동 조향(Differential Drive)을 수행합니다. [cite: 10]

---

## 💡 핵심 알고리즘 (Core Algorithms)

### 1. 영상 처리 파이프라인 (Image Processing)
[cite_start]`process_frame` 함수를 통해 다음과 같은 단계를 거칩니다. [cite: 12]
* [cite_start]**전처리 (Pre-processing):** 영상을 흑백(Grayscale)으로 변환 후, `cv2.threshold`를 통해 이진화(Binary)합니다. [cite: 13]
* [cite_start]**ROI 분할:** 이미지를 상(Top), 중(Mid), 하(Bottom) 3개의 영역으로 분할합니다. [cite: 14]
* [cite_start]**무게 중심 산출:** `cv2.moments`를 사용하여 각 영역별 라인의 중심 좌표를 계산합니다. [cite: 15]

### 2. 적응형 가중치 제어 (Adaptive Weighting Control) [cite_start]단순히 라인의 위치만 따라가는 것이 아니라, **검출된 라인의 개수(시야 확보량)**에 따라 가중치를 동적으로 변경하여 주행 안정성을 높였습니다. [cite: 17, 21]

| 검출된 포인트 | 가중치 배열 (`weight`) | 주행 상황 및 제어 로직 |
| :---: | :--- | :--- |
| **1개 (하단)** | `[4.5]` | **위급 상황 (급커브):** 상단/중단 라인을 놓친 상태입니다. [cite_start]`4.5`라는 높은 가중치를 부여하여 급격하게 방향을 틀어 라인에 복귀합니다. [cite: 22, 23] |
| **2개 (중/하단)** | `[2.0, 1.25]` | **커브 진입:** 상단 라인이 시야에서 사라졌습니다. [cite_start]평소보다 높은 가중치로 부족한 조향각을 보정합니다. [cite: 24, 25] |
| **3개 (전체)** | `[1.2, 0.9, 0.8]` | **직선/안정 주행:** 모든 라인이 잘 보입니다. [cite_start]낮은 가중치를 사용하여 차체의 흔들림(Oscillation)을 방지하고 부드럽게 주행합니다. [cite: 26, 27] |

---

## 📡 통신 프로토콜 (Communication Protocol)

[cite_start]라즈베리파이에서 아두이노로 전송되는 시리얼 명령어 포맷입니다. [cite: 30]

| 명령 (Command) | 포맷 | 설명 |
| :---: | :--- | :--- |
| **STOP** | `S0` | [cite_start]QR 코드 인식 시 즉시 정지 및 모터 차단 [cite: 31] |
| **FORWARD** | `F{val}` | [cite_start]직진 주행 (속도 가속) [cite: 32] |
| **LEFT** | `L{val}` | [cite_start]좌회전 (왼쪽 모터 감속, 조향값 적용) [cite: 33] |
| **RIGHT** | `R{val}` | [cite_start]우회전 (오른쪽 모터 감속, 조향값 적용) [cite: 33] |

* **Example:** `L10\n` (왼쪽으로 조향 강도 10만큼 회전)

---

## 🚀 개선 사항 및 트러블슈팅
* **PWM 제한 로직 수정:** 모터 속도가 하드웨어 한계(0~255)를 넘지 않도록 Clamping 로직을 적용하여 안정성을 확보했습니다.
* [cite_start]**QR 인식 지연 해결:** QR 코드 인식 시 통신 버퍼 오버플로우를 방지하기 위해 시리얼 버퍼 초기화(`reset_input_buffer`) 로직을 최적화했습니다. [cite: 43]
