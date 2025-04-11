#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DEBUG
//#define LED_BUILTIN_PIN 13
#define CMD_SIZE 60
#define ARR_CNT 5

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial BTSerial(10, 11); // RX ==>BT:TXD, TX ==> BT:RXD

char lcdLine1[17] = "      STATE";
char lcdLine2[17] = "";
char recvId[10] = "KJW_BT";  
char sendBuf[CMD_SIZE];

bool windowstate = false;
unsigned long starttime = 0;
int cnt = 0;

void lcdDisplay(int x, int y, char *str) {
  lcd.setCursor(x, y);
  lcd.print("                ");  // 기존 문자열 지우기
  lcd.setCursor(x, y);
  lcd.print(str);
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("setup() start!");
#endif
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);

  //pinMode(LED_BUILTIN_PIN, OUTPUT);
  BTSerial.begin(9600);
}

void loop() {
  if (BTSerial.available()) {
    Serial.println("📡 블루투스 데이터 수신 중...");
    bluetoothEvent();
  }

  if (windowstate) {
    unsigned long endtime = (millis() - starttime) / 1000;
    unsigned long remainingtime = cnt - endtime;
    unsigned int min = remainingtime / 60;
    unsigned int sec = remainingtime % 60;

    sprintf(lcdLine1, "  TIMER %02d:%02d", min, sec);
    strcpy(lcdLine2, "  WINDOW OPEN");

    lcdDisplay(0, 0, lcdLine1);
    lcdDisplay(0, 1, lcdLine2);

    if (endtime >= cnt) {
      windowstate = false;
      strcpy(lcdLine1, "      STATE");
      strcpy(lcdLine2, "  WINDOW CLOSE");
      lcdDisplay(0, 0, lcdLine1);
      lcdDisplay(0, 1, lcdLine2);
    }
  }

#ifdef DEBUG
  if (Serial.available()) {
    char c = Serial.read();
    BTSerial.write(c);
    Serial.print("📤 PC -> 블루투스: ");
    Serial.println(c);
  }
#endif
}



  

// 문자열 앞뒤 공백 & 개행 문자 제거
void trim(char* str) {
  int i, start = 0, end = strlen(str) - 1;
  while (str[start] == ' ' || str[start] == '\r' || str[start] == '\n') start++;  // 앞 공백 & 개행 문자 제거
  while (end > start && (str[end] == ' ' || str[end] == '\r' || str[end] == '\n')) end--;  // 뒤 공백 & 개행 문자 제거
  for (i = start; i <= end; i++) str[i - start] = str[i];
  str[i - start] = '\0';  // 새로운 문자열로 저장
}



void bluetoothEvent() {
  int i = 0;
  char *pToken;
  char *pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};

  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);
  recvBuf[len] = '\0';
  if (len <= 0) return;

#ifdef DEBUG
  Serial.print("✅ 블루투스 메시지 수신 (길이: ");
  Serial.print(len);
  Serial.print("): ");
  Serial.println(recvBuf);
#endif

  if (strlen(recvBuf) < 5) {  // 최소 길이 체크 (ex. "A@B")
#ifdef DEBUG
    Serial.println("⚠️ 데이터 수신 오류: 메시지가 너무 짧음!");
#endif
    return;
  }

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL) {
    pArray[i] = pToken;
    if (++i >= ARR_CNT) break;
    pToken = strtok(NULL, "[@]");
  }

  if (pArray[1] == NULL || pArray[2] == NULL) {
#ifdef DEBUG
    Serial.println("⚠️ 잘못된 메시지 형식! 무시합니다.");
#endif
    return;
  }

  // 문자열 정리 (공백 & 개행 문자 제거)
  trim(pArray[1]);  
  trim(pArray[2]);  
  if (pArray[3] != NULL) trim(pArray[3]);

  // WINDOW 명령 처리
  if (!strcmp(pArray[1], "WINDOW")) {
    if (!strcmp(pArray[2], "OPEN")) { 
      windowstate = true;
      starttime = millis();
      
      if (pArray[3] != NULL) {
        if (!strcmp(pArray[3], "DAY")) {
          cnt = 5;
          Serial.println("DAY WINDOW OPEN");
        } else if (!strcmp(pArray[3], "NIGHT")) {
          cnt = 5;
          Serial.println("NIGHT WINDOW OPEN");
        } else {
          Serial.println("(DAY/NIGHT 없음)");
        }
      } else {
        Serial.println("WINDOW OPEN");
      }

    } 
    else if (!strcmp(pArray[2], "CLOSE")) {
      windowstate = false;
      strcpy(lcdLine1, "      STATE");
      strcpy(lcdLine2, "  WINDOW CLOSE");
      lcdDisplay(0, 0, lcdLine1);
      lcdDisplay(0, 1, lcdLine2);

#ifdef DEBUG
      Serial.println("WINDOW CLOSE");
#endif
    }
  }
}







