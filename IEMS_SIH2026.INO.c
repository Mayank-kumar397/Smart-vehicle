#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- PINS ----------

// L298N
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

// Red analog RGB strip through IRFZ44N
const int RED = 10;

// Charging detection button
const int CHARGE_BUTTON = 7;


// ---------- MOTOR ----------

const int MOTOR_SPEED = 160;


// ---------- RED STRIP ----------

const int RED_BRIGHTNESS = 180;


// ---------- ESTIMATED EV VALUES ----------

// These are demonstration/estimated values
float estimatedSpeed = 25.0;       // km/h
float distanceKm = 0.0;            // km
float batteryPercent = 80.0;       // %

const float MAX_BATTERY = 100.0;
const float MIN_BATTERY = 0.0;


// Driving battery consumption per hour.
// Demo value only.
const float BATTERY_USE_PER_HOUR = 8.0;

// Charging increase per hour.
// Demo value only.
const float BATTERY_CHARGE_PER_HOUR = 15.0;


// ---------- TIMERS ----------

unsigned long lastCalculation = 0;
unsigned long lastDisplay = 0;

const unsigned long CALCULATION_INTERVAL = 1000;
const unsigned long DISPLAY_INTERVAL = 3000;


// ---------- LCD CUSTOM CHARACTERS ----------

byte fullBlock[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte emptyBlock[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};


// ---------- DISPLAY SCREEN ----------

int screen = 0;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // L298N
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // RGB red
  pinMode(RED, OUTPUT);

  // Charging button
  pinMode(CHARGE_BUTTON, INPUT_PULLUP);


  // LCD
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, fullBlock);
  lcd.createChar(1, emptyBlock);


  // ---------- STARTUP ----------

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("EV SIMULATION");

  lcd.setCursor(0, 1);
  lcd.print("STARTING...");

  delay(1500);


  // ---------- START MOTOR ----------

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENA, MOTOR_SPEED);


  // ---------- RED STRIP ON ----------

  analogWrite(RED, RED_BRIGHTNESS);


  lastCalculation = millis();
  lastDisplay = millis();

  lcd.clear();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  unsigned long currentTime = millis();


  // ===================================================
  // CHECK CHARGING BUTTON
  // ===================================================

  bool charging = (digitalRead(CHARGE_BUTTON) == LOW);


  // ===================================================
  // MOTOR ALWAYS RUNS
  // ===================================================

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENA, MOTOR_SPEED);


  // ===================================================
  // RED STRIP ALWAYS BRIGHT
  // ===================================================

  analogWrite(RED, RED_BRIGHTNESS);


  // ===================================================
  // UPDATE ESTIMATED VALUES EVERY SECOND
  // ===================================================

  if (currentTime - lastCalculation >= CALCULATION_INTERVAL) {

    float elapsedHours =
      (currentTime - lastCalculation) / 3600000.0;

    lastCalculation = currentTime;


    // -------------------------------
    // CHARGING
    // -------------------------------

    if (charging) {

      // Battery increases
      batteryPercent +=
        BATTERY_CHARGE_PER_HOUR * elapsedHours;

      if (batteryPercent > MAX_BATTERY) {
        batteryPercent = MAX_BATTERY;
      }

      // Distance does NOT increase
      // while charging
    }


    // -------------------------------
    // DRIVING
    // -------------------------------

    else {

      // Distance travelled
      distanceKm +=
        estimatedSpeed * elapsedHours;

      // Battery decreases
      batteryPercent -=
        BATTERY_USE_PER_HOUR * elapsedHours;

      if (batteryPercent < MIN_BATTERY) {
        batteryPercent = MIN_BATTERY;
      }
    }
  }


  // ===================================================
  // LCD SCREEN CHANGE
  // ===================================================

  if (currentTime - lastDisplay >= DISPLAY_INTERVAL) {

    lastDisplay = currentTime;

    screen++;

    if (screen > 2) {
      screen = 0;
    }

    lcd.clear();
  }


  // ===================================================
  // SCREEN 0
  // SPEED + DISTANCE
  // ===================================================

  if (screen == 0) {

    lcd.setCursor(0, 0);
    lcd.print("EST SPEED:");
    lcd.print((int)estimatedSpeed);
    lcd.print("km/h");

    lcd.setCursor(0, 1);
    lcd.print("DIST:");
    lcd.print(distanceKm, 2);
    lcd.print(" km");
  }


  // ===================================================
  // SCREEN 1
  // BATTERY
  // ===================================================

  if (screen == 1) {

    lcd.setCursor(0, 0);
    lcd.print("BATTERY:");
    lcd.print((int)batteryPercent);
    lcd.print("%");

    lcd.setCursor(0, 1);

    int blocks =
      map((int)batteryPercent, 0, 100, 0, 8);

    for (int i = 0; i < 8; i++) {

      if (i < blocks) {
        lcd.write(byte(0));
      }
      else {
        lcd.write(byte(1));
      }
    }
  }


  // ===================================================
  // SCREEN 2
  // CHARGING STATUS
  // ===================================================

  if (screen == 2) {

    lcd.setCursor(0, 0);

    if (charging) {
      lcd.print("CHARGING: ON");
    }
    else {
      lcd.print("CHARGING: OFF");
    }

    lcd.setCursor(0, 1);
    lcd.print("BAT:");
    lcd.print((int)batteryPercent);
    lcd.print("%");
  }
}
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ==================================================
// LCD
// ==================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// ==================================================
// PIN DEFINITIONS
// ==================================================

// Source switches
const byte SOLAR_SWITCH = 2;
const byte GENERATOR_SWITCH = 5;
const byte GRID_SWITCH = 8;

// LED MOSFET controls
const byte SOLAR_LED = 3;
const byte GENERATOR_LED = 6;

// FAN RELAYS
// These are ACTIVE-LOW
const byte SOLAR_FAN_RELAY = 4;
const byte GENERATOR_FAN_RELAY = 7;
const byte GRID_FAN_RELAY = 9;

// Emergency
const byte EMERGENCY_LED = 10;
const byte BUZZER = 11;

// Temperature sensor
const byte TEMP_DATA = 12;

// Generator red indicator
const byte GENERATOR_RED_LED = 13;


// ==================================================
// TEMPERATURE SENSOR
// ==================================================

OneWire oneWire(TEMP_DATA);
DallasTemperature temperatureSensor(&oneWire);


// ==================================================
// SIMULATION SETTINGS
// ==================================================

// Starting battery percentage
float batteryPercent = 50.0;

// DEMO battery capacity.
// Change this when we use your actual battery rating.
const float BATTERY_CAPACITY_WH = 600.0;

// Charging efficiency
const float CHARGING_EFFICIENCY = 0.85;


// ==================================================
// SOLAR SIMULATION
// ==================================================

const float SOLAR_VOLTAGE = 18.0;
const float SOLAR_POWER = 120.0;


// ==================================================
// GRID SIMULATION
// ==================================================

const float GRID_POWER = 500.0;


// ==================================================
// GENERATOR SIMULATION
// ==================================================

// 10 kVA generator model
// Approx. 8 kW maximum electrical output

const float generatorPower[4] = {
  2.0,
  4.0,
  6.0,
  8.0
};

const float generatorFuelRate[4] = {
  1.2,
  1.5,
  1.8,
  2.3
};

const int generatorStages = 4;

int generatorStage = 1;

float generatorEnergy = 0.0;
float generatorFuelUsed = 0.0;


// ==================================================
// TEMPERATURE LIMITS
// ==================================================

const float WARNING_TEMPERATURE = 50.0;
const float EMERGENCY_TEMPERATURE = 60.0;


// ==================================================
// TIMERS
// ==================================================

unsigned long lastBatteryUpdate = 0;
unsigned long lastGeneratorUpdate = 0;
unsigned long lastGeneratorStageChange = 0;
unsigned long lastDisplayChange = 0;
unsigned long lastEmergencyLED = 0;
unsigned long lastBuzzer = 0;

const unsigned long DISPLAY_TIME = 2000;
const unsigned long GENERATOR_STAGE_TIME = 10000;


// ==================================================
// DISPLAY
// ==================================================

byte displayPage = 0;


// ==================================================
// STATUS
// ==================================================

bool emergency = false;
bool sourceConflict = false;


// ==================================================
// SETUP
// ==================================================

void setup()
{
  // Serial monitor
  Serial.begin(9600);

  // Source switches
  pinMode(SOLAR_SWITCH, INPUT_PULLUP);
  pinMode(GENERATOR_SWITCH, INPUT_PULLUP);
  pinMode(GRID_SWITCH, INPUT_PULLUP);

  // LED MOSFETs
  pinMode(SOLAR_LED, OUTPUT);
  pinMode(GENERATOR_LED, OUTPUT);

  // Fan relays
  pinMode(SOLAR_FAN_RELAY, OUTPUT);
  pinMode(GENERATOR_FAN_RELAY, OUTPUT);
  pinMode(GRID_FAN_RELAY, OUTPUT);

  // Emergency
  pinMode(EMERGENCY_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Generator indicator
  pinMode(GENERATOR_RED_LED, OUTPUT);

  // Initial safe state
  digitalWrite(SOLAR_LED, LOW);
  digitalWrite(GENERATOR_LED, LOW);

  relayOff(SOLAR_FAN_RELAY);
  relayOff(GENERATOR_FAN_RELAY);
  relayOff(GRID_FAN_RELAY);

  digitalWrite(EMERGENCY_LED, LOW);
  digitalWrite(BUZZER, LOW);
  digitalWrite(GENERATOR_RED_LED, LOW);

  // Temperature sensor
  temperatureSensor.begin();

  // LCD
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENERGY SYSTEM");

  lcd.setCursor(0, 1);
  lcd.print("STARTING...");

  delay(1500);

  lcd.clear();

  unsigned long now = millis();

  lastBatteryUpdate = now;
  lastGeneratorUpdate = now;
  lastGeneratorStageChange = now;
  lastDisplayChange = now;
}


// ==================================================
// MAIN LOOP
// ==================================================

void loop()
{
  unsigned long now = millis();

  // ------------------------------------------------
  // READ SWITCHES
  // ------------------------------------------------

  bool solarOn =
    digitalRead(SOLAR_SWITCH) == LOW;

  bool generatorOn =
    digitalRead(GENERATOR_SWITCH) == LOW;

  bool gridOn =
    digitalRead(GRID_SWITCH) == LOW;


  // ------------------------------------------------
  // READ TEMPERATURE
  // ------------------------------------------------

  temperatureSensor.requestTemperatures();

  float temperature =
    temperatureSensor.getTempCByIndex(0);


  // ------------------------------------------------
  // COUNT ACTIVE SOURCES
  // ------------------------------------------------

  int activeSources = 0;

  if (solarOn)
    activeSources++;

  if (generatorOn)
    activeSources++;

  if (gridOn)
    activeSources++;


  // More than one source = conflict
  sourceConflict = activeSources > 1;


  // ------------------------------------------------
  // TEMPERATURE EMERGENCY
  // ------------------------------------------------

  if (temperature >= EMERGENCY_TEMPERATURE)
  {
    emergency = true;
  }
  else if (!sourceConflict)
  {
    emergency = false;
  }


  // =================================================
  // EMERGENCY / SOURCE CONFLICT
  // =================================================

  if (emergency || sourceConflict)
  {
    emergencyMode();
    return;
  }


  // =================================================
  // NORMAL SYSTEM
  // =================================================

  digitalWrite(EMERGENCY_LED, LOW);
  digitalWrite(BUZZER, LOW);


  // ------------------------------------------------
  // NO SOURCE
  // ------------------------------------------------

  if (activeSources == 0)
  {
    allNormalOutputsOff();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("NO SOURCE");

    lcd.setCursor(0, 1);
    lcd.print("BAT:");

    lcd.print((int)batteryPercent);
    lcd.print("%");

    delay(300);

    return;
  }


  // =================================================
  // SOLAR MODE
  // =================================================

  if (solarOn)
  {
    solarMode(now);
  }


  // =================================================
  // GENERATOR MODE
  // =================================================

  else if (generatorOn)
  {
    generatorMode(now);
  }


  // =================================================
  // GRID MODE
  // =================================================

  else if (gridOn)
  {
    gridMode(now);
  }
}


// ==================================================
// SOLAR MODE
// ==================================================

void solarMode(unsigned long now)
{
  // Solar LED ON
  digitalWrite(SOLAR_LED, HIGH);

  // Generator LED OFF
  digitalWrite(GENERATOR_LED, LOW);

  // Generator red indicator OFF
  digitalWrite(GENERATOR_RED_LED, LOW);


  // Solar fan ON
  relayOn(SOLAR_FAN_RELAY);

  // Other fans OFF
  relayOff(GENERATOR_FAN_RELAY);
  relayOff(GRID_FAN_RELAY);


  // Battery charging
  updateSolarBattery(now);


  // LCD
  showSolarDisplay();
}


// ==================================================
// GENERATOR MODE
// ==================================================

void generatorMode(unsigned long now)
{
  // Solar LED OFF
  digitalWrite(SOLAR_LED, LOW);

  // Generator LED ON
  digitalWrite(GENERATOR_LED, HIGH);

  // Generator red indicator ON
  digitalWrite(GENERATOR_RED_LED, HIGH);


  // Generator fan ON
  relayOn(GENERATOR_FAN_RELAY);

  // Other fans OFF
  relayOff(SOLAR_FAN_RELAY);
  relayOff(GRID_FAN_RELAY);


  // Generator calculations
  updateGenerator(now);


  // LCD
  showGeneratorDisplay(now);
}


// ==================================================
// GRID MODE
// ==================================================

void gridMode(unsigned long now)
{
  // LEDs
  digitalWrite(SOLAR_LED, LOW);
  digitalWrite(GENERATOR_LED, LOW);

  // Generator red indicator OFF
  digitalWrite(GENERATOR_RED_LED, LOW);


  // Grid fan ON
  relayOn(GRID_FAN_RELAY);

  // Other fans OFF
  relayOff(SOLAR_FAN_RELAY);
  relayOff(GENERATOR_FAN_RELAY);


  // Battery charging
  updateGridBattery(now);


  // LCD
  showGridDisplay();
}


// ==================================================
// SOLAR BATTERY CALCULATION
// ==================================================

void updateSolarBattery(unsigned long now)
{
  if (now - lastBatteryUpdate >= 2000)
  {
    float hours =
      (now - lastBatteryUpdate) / 3600000.0;

    float energy =
      SOLAR_POWER *
      hours *
      CHARGING_EFFICIENCY;

    float batteryIncrease =
      (energy / BATTERY_CAPACITY_WH) * 100.0;

    batteryPercent += batteryIncrease;

    if (batteryPercent > 100.0)
      batteryPercent = 100.0;

    lastBatteryUpdate = now;
  }
}


// ==================================================
// GRID BATTERY CALCULATION
// ==================================================

void updateGridBattery(unsigned long now)
{
  if (now - lastBatteryUpdate >= 2000)
  {
    float hours =
      (now - lastBatteryUpdate) / 3600000.0;

    float energy =
      GRID_POWER *
      hours *
      CHARGING_EFFICIENCY;

    float batteryIncrease =
      (energy / BATTERY_CAPACITY_WH) * 100.0;

    batteryPercent += batteryIncrease;

    if (batteryPercent > 100.0)
      batteryPercent = 100.0;

    lastBatteryUpdate = now;
  }
}


// ==================================================
// GENERATOR CALCULATION
// ==================================================

void updateGenerator(unsigned long now)
{
  // Automatically change generator load
  if (now - lastGeneratorStageChange >= GENERATOR_STAGE_TIME)
  {
    generatorStage++;

    if (generatorStage >= generatorStages)
      generatorStage = 0;

    lastGeneratorStageChange = now;
  }


  // First calculation
  if (lastGeneratorUpdate == 0)
  {
    lastGeneratorUpdate = now;
    return;
  }


  float hours =
    (now - lastGeneratorUpdate) / 3600000.0;


  float power =
    generatorPower[generatorStage];

  float fuelRate =
    generatorFuelRate[generatorStage];


  // Energy produced
  float energy =
    power * hours;


  // Fuel consumed
  float fuel =
    fuelRate * hours;


  generatorEnergy += energy;
  generatorFuelUsed += fuel;


  // Battery receives energy
  float batteryEnergy =
    energy * CHARGING_EFFICIENCY;


  float batteryIncrease =
    (batteryEnergy / BATTERY_CAPACITY_WH) * 100.0;


  batteryPercent += batteryIncrease;


  if (batteryPercent > 100.0)
    batteryPercent = 100.0;


  lastGeneratorUpdate = now;
}


// ==================================================
// SOLAR LCD
// ==================================================

void showSolarDisplay()
{
  if (millis() - lastDisplayChange >= DISPLAY_TIME)
  {
    displayPage++;

    if (displayPage >= 3)
      displayPage = 0;

    lastDisplayChange = millis();

    lcd.clear();
  }


  if (displayPage == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("SOLAR CHARGING");

    lcd.setCursor(0, 1);
    lcd.print("TEMP:");

    lcd.print(getTemperature(), 1);
    lcd.print((char)223);
    lcd.print("C");
  }


  else if (displayPage == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("SOLAR:");

    lcd.print(18.0, 1);
    lcd.print("V");

    lcd.setCursor(0, 1);
    lcd.print("POWER:");

    lcd.print(120);
    lcd.print("W");
  }


  else
  {
    lcd.setCursor(0, 0);
    lcd.print("BATTERY:");

    lcd.print((int)batteryPercent);
    lcd.print("%");

    lcd.setCursor(0, 1);

    showBatteryBlocks();
  }
}


// ==================================================
// GENERATOR LCD
// ==================================================

void showGeneratorDisplay(unsigned long now)
{
  if (millis() - lastDisplayChange >= DISPLAY_TIME)
  {
    displayPage++;

    if (displayPage >= 4)
      displayPage = 0;

    lastDisplayChange = millis();

    lcd.clear();
  }


  float power =
    generatorPower[generatorStage];

  float fuelRate =
    generatorFuelRate[generatorStage];


  if (displayPage == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("GENERATOR ON");

    lcd.setCursor(0, 1);
    lcd.print("LOAD:");

    int loadPercent =
      (int)((power / 8.0) * 100.0);

    lcd.print(loadPercent);
    lcd.print("%");
  }


  else if (displayPage == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("POWER:");

    lcd.print(power, 1);
    lcd.print("kW");

    lcd.setCursor(0, 1);
    lcd.print("FUEL:");

    lcd.print(fuelRate, 1);
    lcd.print("L/h");
  }


  else if (displayPage == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("ENERGY:");

    lcd.print(generatorEnergy, 2);
    lcd.print("kWh");

    lcd.setCursor(0, 1);
    lcd.print("FUEL:");

    lcd.print(generatorFuelUsed, 2);
    lcd.print("L");
  }


  else
  {
    lcd.setCursor(0, 0);
    lcd.print("BATTERY:");

    lcd.print((int)batteryPercent);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("GENERATOR ACTIVE");
  }
}


// ==================================================
// GRID LCD
// ==================================================

void showGridDisplay()
{
  if (millis() - lastDisplayChange >= DISPLAY_TIME)
  {
    displayPage++;

    if (displayPage >= 3)
      displayPage = 0;

    lastDisplayChange = millis();

    lcd.clear();
  }


  if (displayPage == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("GRID CHARGING");

    lcd.setCursor(0, 1);
    lcd.print("POWER:500W");
  }


  else if (displayPage == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("BATTERY:");

    lcd.print((int)batteryPercent);
    lcd.print("%");

    lcd.setCursor(0, 1);
    showBatteryBlocks();
  }


  else
  {
    float remainingEnergy =
      BATTERY_CAPACITY_WH *
      (100.0 - batteryPercent) /
      100.0;


    float chargingPower =
      500.0 * CHARGING_EFFICIENCY;


    float hours =
      remainingEnergy / chargingPower;


    int totalMinutes =
      (int)(hours * 60.0);


    int hoursLeft =
      totalMinutes / 60;

    int minutesLeft =
      totalMinutes % 60;


    lcd.setCursor(0, 0);
    lcd.print("FULL CHARGE IN");

    lcd.setCursor(0, 1);

    if (hoursLeft < 10)
      lcd.print("0");

    lcd.print(hoursLeft);
    lcd.print(":");

    if (minutesLeft < 10)
      lcd.print("0");

    lcd.print(minutesLeft);
  }
}


// ==================================================
// BATTERY BLOCKS
// ==================================================

void showBatteryBlocks()
{
  int blocks =
    (int)(batteryPercent / 12.5);

  if (blocks > 8)
    blocks = 8;

  if (blocks < 0)
    blocks = 0;


  for (int i = 0; i < blocks; i++)
  {
    lcd.write(byte(255));
  }
}


// ==================================================
// GET TEMPERATURE
// ==================================================

float getTemperature()
{
  return temperatureSensor.getTempCByIndex(0);
}


// ==================================================
// EMERGENCY MODE
// ==================================================

void emergencyMode()
{
  // STOP normal charging indicators
  digitalWrite(SOLAR_LED, LOW);
  digitalWrite(GENERATOR_LED, LOW);
  digitalWrite(GENERATOR_RED_LED, LOW);


  // ALL THREE FAN RELAYS ON
  relayOn(SOLAR_FAN_RELAY);
  relayOn(GENERATOR_FAN_RELAY);
  relayOn(GRID_FAN_RELAY);


  unsigned long now = millis();


  // Fast red LED
  if (now - lastEmergencyLED >= 150)
  {
    digitalWrite(
      EMERGENCY_LED,
      !digitalRead(EMERGENCY_LED)
    );

    lastEmergencyLED = now;
  }


  // Buzzer
  if (now - lastBuzzer >= 250)
  {
    digitalWrite(
      BUZZER,
      !digitalRead(BUZZER)
    );

    lastBuzzer = now;
  }


  // LCD
  lcd.clear();


  if (sourceConflict)
  {
    lcd.setCursor(0, 0);
    lcd.print("SOURCE CONFLICT");

    lcd.setCursor(0, 1);
    lcd.print("ALL POWER OFF");
  }


  else
  {
    lcd.setCursor(0, 0);
    lcd.print("!! EMERGENCY !!");

    lcd.setCursor(0, 1);
    lcd.print("TEMP:");

    lcd.print(getTemperature(), 1);
    lcd.print((char)223);
    lcd.print("C");
  }


  delay(50);
}


// ==================================================
// NORMAL OUTPUT OFF
// ==================================================

void allNormalOutputsOff()
{
  digitalWrite(SOLAR_LED, LOW);
  digitalWrite(GENERATOR_LED, LOW);
  digitalWrite(GENERATOR_RED_LED, LOW);

  relayOff(SOLAR_FAN_RELAY);
  relayOff(GENERATOR_FAN_RELAY);
  relayOff(GRID_FAN_RELAY);
}


// ==================================================
// ACTIVE-LOW RELAY CONTROL
// ==================================================

void relayOn(byte pin)
{
  digitalWrite(pin, LOW);
}


void relayOff(byte pin)
{
  digitalWrite(pin, HIGH);
}