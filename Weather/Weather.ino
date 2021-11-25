#include <EEPROM.h>
#include <Wire.h>
#include "U8glib.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);

#define SEALEVELPRESSURE_HPA (1013.25)

#define PRESSURE_WAIT_TIME (long)240
#define PRESSURE_COUNT (long)45
#define MEASUREMENT_COUNT 10
#define MODES_COUNT 4

long pres[PRESSURE_COUNT];
int pres_sz = 0;
long long pres_time = -PRESSURE_WAIT_TIME * 1000;

#define LEFT_B 32

#define PRINT_LPS false
#define PRINT_PRESSURE false

#define INF_ (long)256 * (long)256 * (long)256 - 1

long double sumx, sumx2, sumy, sumxy;
long double A = 0, B = 0;

long long updateTime = -1000000000;

double alt0 = 0;
double curTemp, curHum, curAlt;
long curPres;
long counter___ = 0;
long long lastTimeCheck__ = 0;

int mode = 0;
long long timePressed = -10000;
long long timeReleased = -10000;
bool oldButton = false;

int t[36];
long pres2[36];
int tpres_sz = 0;
long long tpres_time = -600000;
byte presx[36], tx[36];
byte presy[36], ty[36];
long mnp, mxp, mnt, mxt;

long long initTime;

void setup() {
  Serial.begin(9600);
  if (!bme.begin()) {
    Serial.print("Failed\n");
    /*Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");*/
    while (1);
  }
  if (u8g.getMode() == U8G_MODE_R3G3B2) {
    u8g.setColorIndex(255);
  }
  else if (u8g.getMode() == U8G_MODE_GRAY2BIT) {
    u8g.setColorIndex(3);
  }
  else if (u8g.getMode() == U8G_MODE_BW) {
    u8g.setColorIndex(1);
  }
  else if (u8g.getMode() == U8G_MODE_HICOLOR) {
    u8g.setHiColorByRGB(255, 255, 255);
  }
  u8g.setFont(u8g_font_6x10);
  pinMode(12, INPUT_PULLUP);
  for (int i = 0; i < PRESSURE_COUNT; i++) {
    if (getnum(i) != INF_) {
      pres_sz++;
      pres[i] = getnum(i);
    }
  }
  if (pres_sz > 0) {
    getLLS();
  }
  for (int i = 0; i < 36; i++) {
    if (getnum(100 + 2 * i) != INF_) {
      tpres_sz++;
      pres2[i] = getnum(100 + 2 * i);
      t[i] = getnum(100 + 2 * i + 1) - 2000;
    }
  }
  if (getnum(340) != INF_) {
    pres_time = (long)millis() - getnum(340);
  }
  if (getnum(339) != INF_) {
    tpres_time = (long)millis() - getnum(339);
  }
  updateGraph();
  initTime = millis();
}

void draw(void) {
  String prediction;
  double temp = (double)A * 3600.0;
  if (temp <= -150.0) prediction = "storm";
  else if (temp <= -75.0) prediction = "rain";
  else if (temp < 75) prediction = "same";
  else prediction = "clear";
  switch (mode) {
    case (0):
      u8g.setPrintPos(0, 14);
      u8g.print(String((double)A * 3600.0) + " Pa/Hr" + " (" + prediction + ")");
      u8g.setPrintPos(0, 26);
      u8g.print("Temp: " + String(round(curTemp * 10.0) / 10.0));
      u8g.setPrintPos(0, 38);
      u8g.print("Hum: " + String((int)curHum) + "%");
      u8g.setPrintPos(0, 50);
      u8g.print("Pres: " + String(paToMercury(curPres)) + " mm");
      u8g.setPrintPos(0, 62);
      break;
    case (1):
      u8g.setPrintPos(0, 14);
      u8g.print(String((double)A * 3600.0) + " Pa/Hr" + " (" + prediction + ")");
      u8g.setPrintPos(0, 26);
      u8g.print("Temp: " + String(curTemp));
      u8g.setPrintPos(0, 38);
      u8g.print("Hum: " + String(curHum) + "%");
      u8g.setPrintPos(0, 50);
      u8g.print("Pres: " + String(paToMercury(curPres)) + " mm");
      u8g.setPrintPos(0, 62);
      u8g.print("Height: " + String(curAlt - alt0) + " m");
      break;
    case (2):
      u8g.setPrintPos(0, 14);
      u8g.print("Pressure");
      u8g.setPrintPos(0, 25);
      u8g.print(String(mxp / 10) + '.' + String(mxp % 10));
      u8g.setPrintPos(0, 63);
      u8g.print(String(mnp / 10) + '.' + String(mnp % 10));
      for (int i = 1; i < tpres_sz; i++) {
        u8g.drawLine(presx[i - 1], presy[i - 1], presx[i], presy[i]);
      }
      break;
    case (3):
      u8g.setPrintPos(0, 14);
      u8g.print("Temperature");
      u8g.setPrintPos(0, 25);
      u8g.print(String(mxt / 10) + '.' + String(mxt % 10));
      u8g.setPrintPos(0, 63);
      u8g.print(String(mnt / 10) + '.' + String(mnt % 10));
      for (int i = 1; i < tpres_sz; i++) {
        u8g.drawLine(tx[i - 1], ty[i - 1], tx[i], ty[i]);
      }
      break;
  }
  u8g.setPrintPos(122, 14);
  u8g.print(mode + 1);
}

void loop() {
  u8g.firstPage();
  do {
    Main();
    draw();
  } while (u8g.nextPage());
}

void Main() {
  if (button() == false && oldButton == true) {
    if (millis() - timeReleased >= 1000 && mode == 1) {
      alt0 = curAlt;
    }
    else {
      mode = (mode + 1) % MODES_COUNT;
    }
  }
  if (button()) {
    timePressed = millis();
  }
  else {
    timeReleased = millis();
  }
  oldButton = button();
  if (PRINT_LPS) {
    counter___++;
    Serial.println((double)counter___ / (double)millis() * 1000.0);
  }
  if (mode == 0 && millis() - updateTime >= 30000 || mode == 1 && millis() - updateTime >= 500) {
    updateTime = millis();
    curTemp = getTemp();
    curHum = getHum();
    curPres = getPres();
    curAlt = getAlt();
  }
  if ((long long)millis() - pres_time >= PRESSURE_WAIT_TIME * 1000) {
    pres_time = millis();
    long cur = 0;
    for (int i = 0; i < MEASUREMENT_COUNT; i++) {
      cur += getPres();
      delay(20 / MEASUREMENT_COUNT);
    }
    cur /= MEASUREMENT_COUNT;
    if (PRINT_PRESSURE) {
      Serial.println(cur);
    }
    if (pres_sz < PRESSURE_COUNT) {
      pres[pres_sz] = cur;
      pres_sz++;
    }
    else {
      for (int i = 0; i < pres_sz - 1; i++) {
        pres[i] = pres[i + 1];
      }
      pres[pres_sz - 1] = cur;
    }
    for (int i = 0; i < pres_sz; i++) {
      if (pres[i]) {
        savenum(i, pres[i]);
      }
    }
    getLLS();
  }
  if ((long long)millis() - tpres_time >= 600000) {
    tpres_time = millis();
    long cur = 0;
    double curt = 0;
    for (int i = 0; i < MEASUREMENT_COUNT; i++) {
      cur += getPres();
      curt += getTemp();
      delay(20 / MEASUREMENT_COUNT);
    }
    cur /= MEASUREMENT_COUNT;
    curt /= MEASUREMENT_COUNT;
    if (tpres_sz < 36) {
      pres2[tpres_sz] = cur;
      t[tpres_sz] = curt * 10;
      tpres_sz++;
    }
    else {
      for (int i = 0; i < tpres_sz - 1; i++) {
        pres2[i] = pres2[i + 1];
        t[i] = t[i + 1];
      }
      pres2[tpres_sz - 1] = cur;
      t[tpres_sz - 1] = curt * 10;
    }
    for (int i = 0; i < 36; i++) {
      if (pres2[i]) {
        savenum(100 + 2 * i, pres2[i]);
        savenum(100 + 2 * i + 1, t[i] + 2000);
      }
    }
    updateGraph();
  }
  if (millis() - lastTimeCheck__ >= 30000) {
    lastTimeCheck__ = millis();
    savenum(340, (long)millis() - pres_time);
    savenum(339, (long)millis() - tpres_time);
  }
}

void updateGraph() {
  mnp = 10000;
  mxp = -10000;
  mnt = 10000;
  mxt = -10000;
  for (int i = 0; i < tpres_sz; i++) {
    mnp = min(mnp, paToMercury(pres2[i]) * 10.0);
    mxp = max(mxp, paToMercury(pres2[i]) * 10.0);
    mnt = min(mnt, t[i]);
    mxt = max(mxt, t[i]);
  }
  for (int i = 0; i < tpres_sz; i++) {
    long x, y;
    x = map(i, 0, 35, 32, 127);
    y = map(paToMercury(pres2[i]) * 10.0, mnp, mxp, 63, 16);
    presx[i] = x;
    presy[i] = y;
    x = map(i, 0, 35, 26, 127);
    y = map(t[i], mnt, mxt, 63, 16);
    tx[i] = x;
    ty[i] = y;
  }
}

void getLLS() {
  sumx = 0;
  sumx2 = 0;
  sumy = 0;
  sumxy = 0;
  for (int i = 0; i < pres_sz; i++) {
    sumx += PRESSURE_WAIT_TIME * i;
    sumx2 += (PRESSURE_WAIT_TIME * i) * (PRESSURE_WAIT_TIME * i);
    sumy += pres[i];
    sumxy += (PRESSURE_WAIT_TIME * i) * pres[i];
  }
  long double temp = (pres_sz * sumx2) - sumx * sumx;
  if (fabs(temp) > 1e-3) {
    A = ((pres_sz * sumxy - sumx * sumy) / temp);
  }
  B = (sumy - A * sumx) / pres_sz;
}

double getTemp() {
  return bme.readTemperature() - 2.5;
}

double getHum() {
  return bme.readHumidity();
}

long getPres() {
  return bme.readPressure();
}

double getAlt() {
  return bme.readAltitude(SEALEVELPRESSURE_HPA);
}

bool button() {
  return !digitalRead(12);
}

void savenum(int i, long num) {
  EEPROM.write(3 * i, num / 256 / 256);
  EEPROM.write(3 * i + 1, num / 256 % 256);
  EEPROM.write(3 * i + 2, num % 256);
}

long getnum(int i) {
  return EEPROM.read(3 * i) * (long)256 * (long)256 + EEPROM.read(3 * i + 1) * (long)256 + EEPROM.read(3 * i + 2);
}

double paToMercury(long p) {
  return (double)p / ((double)SEALEVELPRESSURE_HPA * 100.0) * 760.0;
}
