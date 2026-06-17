#include <Preferences.h>
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_HTU21DF.h"

RTC_DS3231 rtc;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

#define dataPin 5
#define latchPin 6
#define clockPin 7
#define secondLED 21
#define PWMPin 10
#define Potentio 4
#define Buzzer 0
#define Button 1
#define switch_1 2
#define buttonMode 3

volatile int currentSecond = 0;
volatile int currentMinute = 0;
volatile int currentHour = 0;

volatile int digit1 = 0;
volatile int digit2 = 0;
volatile int digit3 = 0;
volatile int digit4 = 0;

volatile int previousDigit1 = -1;
volatile int previousDigit2 = -1;
volatile int previousDigit3 = -1;
volatile int previousDigit4 = -1;

int previousSecond = -1;

bool secondState = 1;
int mode_logic = 0;

int button_v = 0;
bool buttonMode_logic = 0;
int alarmHour = 0;
int alarmMinute = 0;

const unsigned long ALARM_DURATION = 60000; // 1 menit
unsigned long alarmStartTime = 0;
bool alarmActive = false;

const int POT_MIN = 120;
const int POT_MAX = 4010;

int filterBuffer[8] = {0};
int filterIndex = 0;

Preferences preferences;

//Numbers code
const int patternsNum = 19;
const byte patterns[patternsNum] = {
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111,  // 9

  0b00000000, //10 - ""
  0b01000000, //11 - -
  0b01100011, //12 - UP circle
  0b01011100, //13 - Down Circle

  0b00111001, //14 - C
  0b01111001, //15 - E
  0b00110001 //16 - r
};

// === Display Functions ===
void displayFourDigits(int first, int second, int third, int fourth){
  first = constrain(first, 0, patternsNum);
  second = constrain(second, 0, patternsNum);
  third = constrain(third, 0, patternsNum);
  fourth = constrain(fourth, 0, patternsNum);

  uint32_t data = 0;

  data =  patterns[fourth] << 24 |
          patterns[third] << 16 |
          patterns[second] << 8 |
          patterns[first];

  digitalWrite(latchPin, LOW);

  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 24) & 0xFF);
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 16) & 0xFF);
  shiftOut(dataPin, clockPin, MSBFIRST, (data >> 8) & 0xFF);
  shiftOut(dataPin, clockPin, MSBFIRST, data & 0xFF);

  digitalWrite(latchPin, HIGH);
}

// ===== Dimming Control =====
int applyLowPassFilter(int value, int num) {
    filterBuffer[filterIndex] = value;
    filterIndex = (filterIndex + 1) % num;
    
    long sum = 0;
    for(int i = 0; i < num; i++) {
        sum += filterBuffer[i];
    }
    return sum / num;
}

int calculateLogBrightness(int potValue) {
    // Linear mapping dulu
    float normalized = (float)(potValue - POT_MIN) / (POT_MAX - POT_MIN);
    
    // Apply gamma correction (gamma ~2.2 untuk LED)
    float gammaCorrected = pow(normalized, 2.2);
    
    // Convert to 0-255 range
    return 255 - (int)(gammaCorrected * 255);
}

void dimming(){
  int rawValue = analogRead(Potentio);
  int filtered = applyLowPassFilter(rawValue, 6);
  
  // 2. Constrain ke range yang diinginkan
  filtered = constrain(filtered, POT_MIN, POT_MAX);
  
  // 3. Map ke brightness dengan curve logaritmik
  // int brightness = calculateLogBrightness(filtered);

  int brightness = map(filtered, POT_MIN, POT_MAX, 180, 0);
  
  ledcWrite(PWMPin, brightness);
  // Serial.print(filtered);
  // Serial.print("-");
  // Serial.println(brightness);
  
  secondState = (brightness > 30);
  if (!secondState){  //turn of the second's led if the Light is also off
    digitalWrite(secondLED, 0);
  }
  
  // Serial.printf("Pot: %d -> Brightness: %d\n", filtered, brightness);
}

// ===== Button =====
void buttonPressed(){
  if (analogRead(Button) > 700){
    if (analogRead(switch_1) > 2000){
      setAlarm();
    }
    else if (analogRead(switch_1) < 2000){
      setClock();
    }
  }
}

void modeSwitch(){
  if (digitalRead(buttonMode) == LOW){
    if (buttonMode_logic){
      Serial.println("BUTTON MODE PRESSEED");
      mode_logic += 1;
      if (mode_logic > 2){
        mode_logic = 0; //looping back to mode 0 (clock mode)
      }
      buttonMode_logic = 0;
    }
  }
  else{
    buttonMode_logic = 1; //button is released
  }
}

// ===== Alarm Functions =====
void saveAlarmSettings() {
  preferences.begin("my-app", false);
  preferences.putInt("alarm_hour", alarmHour);
  preferences.putInt("alarm_minute", alarmMinute);
  preferences.end();
  Serial.printf("Alarm saved: %02d:%02d\n", alarmHour, alarmMinute);
}

void loadAlarmSettings() {
  preferences.begin("my-app", true);  // Read-only mode
  alarmHour = preferences.getInt("alarm_hour", 0);
  alarmMinute = preferences.getInt("alarm_minute", 0);
  preferences.end();
}

void setAlarm(){
  int tempAlarmHour = alarmHour;
  int tempAlarmMinute = alarmMinute;

  digitalWrite(secondLED, 0);

  while (analogRead(Button) > 700){
    button_v = analogRead(Button);
    Serial.println(button_v);

    if (button_v > 700 && button_v < 2000){
      // Serial.println("BUTTON 1");
      alarmMinute = (alarmMinute + 1) % 60;
      
    }
    else if (button_v > 2000 && button_v < 3000){
      // Serial.println("BUTTON 2");
      alarmHour = (alarmHour + 1) % 24;   
    }

    displayFourDigits(alarmHour / 10, alarmHour % 10, alarmMinute / 10, alarmMinute % 10);

    delay(250);
  }

  if (tempAlarmHour != alarmHour || tempAlarmMinute != alarmMinute){
    saveAlarmSettings();
  }
  
  displayFourDigits(currentHour / 10, currentHour % 10, currentMinute / 10, currentMinute % 10);
}

void checkAlarm() {
    if (currentHour == alarmHour && currentMinute == alarmMinute && !alarmActive) {
        alarmActive = true;
        alarmStartTime = millis();
    }
    
    if (alarmActive && (analogRead(switch_1) > 2000)) {
        if (millis() - alarmStartTime < ALARM_DURATION) {
            ledcWrite(Buzzer, (millis() / 500) % 2 ? 255 : 0); // Buzzer beeping/second
        } else {
            alarmActive = false;
            ledcWrite(Buzzer, 0);
        }
    }
}

// ===== Clock =====
void setClock(){
  int tempHour = currentHour;
  int tempMinute = currentMinute;
  digitalWrite(secondLED, HIGH);

  while (analogRead(Button) > 700){
    button_v = analogRead(Button);
    // Serial.println(button_v);

    if (button_v > 700 && button_v < 2000){
      // Serial.println("BUTTON 1");
      tempMinute = (tempMinute + 1) % 60;   
    }
    else if (button_v > 2000 && button_v < 3000){
      // Serial.println("BUTTON 2");
      tempHour = (tempHour + 1) % 24;   
    }
    displayFourDigits(tempHour / 10, tempHour % 10, tempMinute / 10, tempMinute % 10);
    delay(250);
  }

  if (tempHour != currentHour || tempMinute != currentMinute){
    DateTime now = rtc.now();
    DateTime newTime(now.year(), now.month(), now.day(), tempHour, tempMinute, 0);
    rtc.adjust(newTime);
  }
  
  displayFourDigits(currentHour / 10, currentHour % 10, currentMinute / 10, currentMinute % 10);
}

// ===== MODE =====
void clockMode(){
  DateTime now = rtc.now();
  currentSecond = now.second();
  currentMinute = now.minute();
  currentHour = now.hour();

  if ((currentSecond != previousSecond) && secondState) {
    digitalWrite(secondLED, currentSecond % 2);
    previousSecond = currentSecond;
  }

  digit1 = currentHour / 10;
  digit2 = currentHour % 10;
  digit3 = currentMinute / 10;
  digit4 = currentMinute % 10;

  buttonPressed();
  checkAlarm();
}

void tempMode(){
  int temp = htu.readTemperature();
  digitalWrite(secondLED, LOW);

  digit1 = temp / 10;
  digit2 = temp % 10;
  digit3 = 12;
  digit4 = 14;
}

void humidMode(){
  int humid = htu.readHumidity();
  digitalWrite(secondLED, LOW);

  digit1 = humid / 10;
  digit2 = humid % 10;
  digit3 = 12;
  digit4 = 13;

}

// ===== MAIN CODE =====
void setup() {
  Serial.begin(9600);

  preferences.begin("my-app", false);
  bool firstRun = preferences.getBool("initialized", false);

  if (!firstRun) {   
      // Set nilai default hanya sekali
      preferences.putInt("alarm_hour", 0);
      preferences.putInt("alarm_minute", 0);
      
      // Tandai sudah diinisialisasi
      preferences.putBool("initialized", true);
  }
  preferences.end();

  loadAlarmSettings();

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(secondLED, OUTPUT);

  pinMode(PWMPin, OUTPUT);
  pinMode(Potentio, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Button, INPUT);
  pinMode(switch_1, INPUT_PULLUP);
  pinMode(buttonMode, INPUT_PULLUP);
  
  Wire.begin();

  ledcAttach(PWMPin, 1000, 8);
  ledcWrite(PWMPin, 0);

  ledcAttach(Buzzer, 500, 8);
  ledcWrite(Buzzer, 0);

  if (!htu.begin()) {
    while (1){
      displayFourDigits(15, 16, 11, 2);
    }
  }

  if (!rtc.begin()) {
    // RTC not found
    while(1){
      displayFourDigits(15, 16, 11, 1);
    }
  }

}

void loop() {
  modeSwitch();

  switch(mode_logic){
    case 0:
      clockMode();
      // Serial.println("CLOCK MODE");
      break;
    case 1:
      tempMode();
      // Serial.println("TEMP MODE");
      break;
    case 2:
      humidMode();
      // Serial.println("HUMID MODE");
      break;
  }

  if (digit1 != previousDigit1 || digit2 != previousDigit2 || digit3 != previousDigit3 || digit4 != previousDigit4){
    displayFourDigits(digit1, digit2, digit3, digit4);
    previousDigit1 = digit1;
    previousDigit2 = digit2;
    previousDigit3 = digit3;
    previousDigit4 = digit4;
  }

  dimming();
  delay(200);
}
