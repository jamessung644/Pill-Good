#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

Servo servo;
ESP8266WebServer server(80);

const char* ssid = "PillGood2"; // AP의 SSID
const char* password = ""; // 안전한 암호 설정

const int buttonPin = D2;
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0; // 디바운싱을 위한 마지막 시간 추적
unsigned long debounceDelay = 30; // 디바운스 지연 시간

// RGB LED 핀 설정
const int redPin = D7;
const int greenPin = D6;
const int bluePin = D5;
bool timeMatched = false;

unsigned long currentTimeInSeconds = 0; // 현재 시간을 초로 저장
String selectedTime = ""; // 선택된 시간

void setRGBColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void handleRoot() {
  String html = "<html><head><title>Servo Control</title></head><body>"
                "<h1>Servo Control</h1>"
                "<p><a href=\"/servo?position=0\"><button>0 Degrees</button></a></p>"
                "<p><a href=\"/servo?position=180\"><button>180 Degrees</button></a></p>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void handleServo() {
  String position = server.arg("position");
  if (position != "") {
    int angle = position.toInt();
    servo.write(angle);
  }
  server.sendHeader("Location", "/", true); // 리다이렉트
  server.send(302, "text/plain", ""); // 메인 페이지로 리다이렉트
}

void handleInput() {
  String input = server.arg("input");
  if (input != "") {
    int angle = input.toInt();
    servo.write(angle);
    server.send(200, "text/plain", "Servo moved to " + input + " degrees");
  } else {
    server.send(400, "text/plain", "Invalid input");
  }
}

void handleTimeSetting() {
  if (server.hasArg("selectedTime")) {
    selectedTime = server.arg("selectedTime").substring(0, 5);
    Serial.println("Selected time: " + selectedTime);
  }

  if (server.hasArg("currentTime")) {
    String currentTimeString = server.arg("currentTime");
    int hour = currentTimeString.substring(0, 2).toInt();
    int minute = currentTimeString.substring(3, 5).toInt();
    int second = currentTimeString.substring(6, 8).toInt();
    currentTimeInSeconds = hour * 3600 + minute * 60 + second;
  }

  server.send(200, "text/plain", "Time received");
}

void setup() {
  Serial.begin(74880); // 시리얼 통신 속도 설정

  servo.attach(D1);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setRGBColor(0, 0, 0);

  pinMode(buttonPin, INPUT_PULLUP);
  lastButtonState = digitalRead(buttonPin); // 초기 버튼 상태 설정

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(myIP);

  server.on("/", handleRoot);
  server.on("/servo", handleServo);
  server.on("/input", handleInput);
  server.on("/setTime", HTTP_POST, handleTimeSetting);

  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();

  static unsigned long lastUpdateTime = millis();
  if (millis() - lastUpdateTime >= 1000) {
    lastUpdateTime = millis();
    currentTimeInSeconds++; // 시간 증가

    int hours = currentTimeInSeconds / 3600 % 24;
    int minutes = (currentTimeInSeconds % 3600) / 60;
    int seconds = currentTimeInSeconds % 60;
    char currentTimeString[9];
    sprintf(currentTimeString, "%02d:%02d:%02d", hours, minutes, seconds);

    String currentMinute = String(minutes);
    if (currentMinute.length() == 1) currentMinute = "0" + currentMinute;

    String currentTimeStr(currentTimeString);
    String checkTime = currentTimeStr.substring(0, 5);

    // 선택된 시간과 현재 시간이 일치하는지 확인
    if (selectedTime == checkTime) {
      if (!timeMatched) { // 시간이 일치하는 첫 순간에만
        Serial.println("Times match! Triggering the action.");
        timeMatched = true; // 플래그 설정

        // 모터 작동 코드 (버튼이 눌렸을 때와 동일)
        setRGBColor(255, 200, 200);
        servo.write(0);
        delay(1000);
        servo.write(180);
        delay(1000);
        setRGBColor(0, 0, 0);
      }
    } else {
      timeMatched = false; // 시간이 더 이상 일치하지 않으면 플래그 해제
    }

    Serial.println(checkTime);
  }
  // 버튼 처리 코드
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        setRGBColor(255, 200, 200);
        servo.write(0);
        delay(1000);
        servo.write(180);
        delay(1000);
        setRGBColor(0, 0, 0);
      }
    }
  }

  lastButtonState = reading;
}
