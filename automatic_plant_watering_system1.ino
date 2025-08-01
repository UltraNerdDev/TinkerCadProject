#include <LiquidCrystal.h>
#include <Servo.h>

// Дефиниране на пинове за LCD
LiquidCrystal lcd(13, 12, 6, 5, 3, 2);

// Дефиниране на пинове за други компоненти
const int soilSensorPin = A0;   // Аналогов пин за датчика за влажност
const int ldrPin = A1;          // Аналогов пин за фоторезистор (светлина)
const int alertLedPin = 7;      // Цифров пин за червения LED (светване при проблем)
const int normalLedPin = 4;    // Цифров пин за зеления LED (светване когато всичко е ОК)
const int buzzerPin = 8;        // Цифров пин за зумера
const int servoPin = 10;         // Цифров пин за серво мотора (PWM)

Servo wateringServo; // Създаване на обект "wateringServo"

// Прагови стойности 
const int dryThreshold = 400;   // Под тази стойност се счита за "сухо"
const int wetThreshold = 800;   // Над тази стойност се счита за "много мокро"
const int minLight = 300;       // Минимална оптимална светлина (аналогова стойност от LDR)

// Дефиниране на ъгли за сервото
const int servoClosedAngle = 0;   // Сервото е "затворено" (имитация на затворена клапа)
const int servoOpenAngle = 90;    // Сервото е "отворено" (имитация на отворена клапа)

// Променливи за ротация на LCD екрана
int screenMode = 0; // 0: Soil Moisture, 1: Light
unsigned long lastScreenChangeTime = 0;
const long screenChangeInterval = 5000; // Промяна на екрана на всеки 5 секунди

void setup() {
  lcd.begin(16, 2); // Инициализация на LCD 16x2
  Serial.begin(9600); // Стартиране на серийна комуникация за дебъгване

  wateringServo.attach(servoPin); // Прикрепяне на серво обекта към избрания пин
  wateringServo.write(servoClosedAngle); // Инициализиране на сервото в "затворена" позиция

  pinMode(alertLedPin, OUTPUT);   //
  pinMode(normalLedPin, OUTPUT);  // Нагласяне на компонентите
  pinMode(buzzerPin, OUTPUT);     //

  lcd.setCursor(0, 0); 
  lcd.print("Plant Guardian"); 
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  // ^Извеждане на съобщения на екрана 
  lcd.clear(); // Изчистване преди главния цикъл
}

void loop() { 
  // Четене на данни от сензори
  int soilMoistureValue = analogRead(soilSensorPin);
  int moisturePercentage = map(soilMoistureValue, 0, 1023, 0, 100); // Преобразуване в проценти

  int lightValue = analogRead(ldrPin); // Стойност от фоторезистора

  // Извеждане на данни към Serial Monitor
  Serial.print("Soil: "); Serial.print(moisturePercentage); 
  Serial.print("% | ");
  Serial.print("Light: "); Serial.println(lightValue);

  // Логика за известяване и действия
  bool isAlert = false;
  String alertMessage = "All Good!       "; // По подразбиране, когато праговете са в нормата

  // Проверка за влажност на почвата
  if (soilMoistureValue < dryThreshold) {
    isAlert = true;
    alertMessage = "Water Me!       ";
    // Активиране на поливането
    wateringServo.write(servoOpenAngle); // Включва симулираното поливане
    Serial.println("Activating Watering (Servo Open)!");
    delay(2000); // Сервото остава "отворено" за 2 секунди
    wateringServo.write(servoClosedAngle); // Изключва симулираното поливане
    Serial.println("Watering Off (Servo Closed).");
    delay(5000); // Изчакване, симулиращо попиването на водата в почвата преди следващо поливане
  } else if (soilMoistureValue > wetThreshold) {
    isAlert = true;
    alertMessage = "Overwatered!    ";
    wateringServo.write(servoClosedAngle); 
  }

  // Проверка за светлина (ако вече няма "Water Me!" или "Overwatered!")
  if (lightValue < minLight && !isAlert) { // Ако вече има проблем, не презаписваме съобщението
    isAlert = true;
    alertMessage = "Light LOW!      ";
  }

  // Известяване с LED и зумер
  if (isAlert) {
    digitalWrite(alertLedPin, HIGH);   // Червен LED светва
    digitalWrite(normalLedPin, LOW);    // Зелен LED гасне
    tone(buzzerPin, 500, 100);          // Кратък звуков сигнал
    delay(100);
    noTone(buzzerPin);
  } else {
    digitalWrite(alertLedPin, LOW);     // Червен LED гасне
    digitalWrite(normalLedPin, HIGH);   // Зелен LED светва
    noTone(buzzerPin);                  // Изключва зумера
  }
  lcd.setCursor(0, 1);
  lcd.print(alertMessage); // Винаги показва най-релевантното съобщение

  // Автоматична ротация на LCD екрана на всеки 5 секунди
  if (millis() - lastScreenChangeTime >= screenChangeInterval) { // Проверка дали изминалото време от началото на програма минус последното време 
    															 // когато екрана се е сменил дали е по-голямо от интервала за смяна на екрани (5 секунди)
    
    lastScreenChangeTime = millis(); // Обновява времето на последна промяна
    screenMode++; 
    if (screenMode > 1) { // Имаме 2 режима: 0 (влажност), 1 (светлина)
      screenMode = 0;
    }
  }

  // Показване на данни на LCD (горен ред) според текущия режим 
  lcd.setCursor(0, 0); // Показване на параметрите на горния ред
  if (screenMode == 0) {
    lcd.print("Soil: ");
    lcd.print(moisturePercentage);
    lcd.print("%   "); // Добавяне на интервали за изчистване
  } else if (screenMode == 1) {
    lcd.print("Light: ");
    lcd.print(lightValue);
    lcd.print("    "); // За да изчисти остатъци
  }

  delay(50); // Кратко закъснение за стабилност
}