// 1. 핀 번호 설정 (그대로 유지)
int ENA = 5;
int IN1 = 6;
int IN2 = 7;

int IN3 = 8;
int IN4 = 9;
int ENB = 10;

const int lowerLimit = 10;
const int upperLimit = 255;

// 2. 상태 변수 (현재 속도와 조향각을 기억함)
const int baseSpeedL = 150;
const int baseSpeedR = 150;
int addSpeed = 0;
int isStopped = false;
int currentSteering = 0; // 현재 조향값 (음수:좌회전, 0:직진, 양수:우회전)

void setup() {
  Serial.begin(9600); // 파이썬과 통신 속도 맞춤

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    // 1. 명령어를 문자열로 통째로 받음 (예: "F5\n", "STOP\n")
    String input = Serial.readStringUntil('\n');
    input.trim(); // 공백 제거

    // 2. 명령어 파싱 (첫 글자와 나머지 숫자 분리)
    char cmd = input.charAt(0);              // 첫 글자 (F, L, R, S)
    int value = input.substring(1).toInt();  // 나머지 숫자 부분

    Serial.print(cmd);

    // 3. 명령어에 따른 값 누적 (더하기/빼기)
    if (cmd == 'S') {
      // STOP: 모든 값 0으로 초기화 및 정지
      Serial.println('WHAT THE FUCK!!');
      addSpeed = 0;
      currentSteering = 1000;
      isStopped = true;
    }
    else if (cmd == 'F') {
      // F5 -> 현재 속도에 5 더하기
      addSpeed = value;
      currentSteering = 0;
      isStopped = false;
    }
    else if (cmd == 'L') {
      // L10 -> 왼쪽으로 10만큼 더 꺾기 (조향값 감소)
      addSpeed = value;
      currentSteering = -1;
      isStopped = false;
    }
    else if (cmd == 'R') {
      // R20 -> 오른쪽으로 20만큼 더 꺾기 (조향값 증가)
      addSpeed = value;
      currentSteering = 1;
      isStopped = false;
    }

    // 5. 모터 구동
    applyMotor();
  }
}

void applyMotor() {
  // 기본 속도에 조향값을 섞어서 양쪽 바퀴 속도 결정
  int speedLeft = baseSpeedL;
  int speedRight = baseSpeedR;

  if (currentSteering == 1000)
  {
    speedLeft = 0;
    speedRight = 0;
  }
  else
  {
    if (currentSteering > 0)
      speedRight -= addSpeed;
    else if (currentSteering < 0)
      speedLeft -= addSpeed;
    else
    {
      speedLeft += addSpeed / 2;
      speedRight += addSpeed / 2;
    }

    // 최종 PWM 범위 제한 (0~255)
    if (speedLeft < lowerLimit)
      speedLeft = lowerLimit;
    else if (speedLeft > upperLimit)
      speedLeft = upperLimit;
    if (speedRight < lowerLimit)
      speedRight = lowerLimit;
    else if (speedLeft > upperLimit)
      speedRight = upperLimit;
  }

  Serial.print(speedLeft);

  // --- 왼쪽 모터 (전진 방향 고정) ---
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  if (isStopped)
  {
    delay(3000);
    isStopped = false;
  }
}