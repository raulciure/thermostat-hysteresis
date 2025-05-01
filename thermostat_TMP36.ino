#include <Adafruit_LEDBackpack.h>

#define ON true
#define OFF false
#define INTERVAL 2
#define TEMP_ADJ_VALUE 0.5


int TEMP_PIN = A0;
int COOL_PIN = 4;
int HEAT_PIN = 5;
int SEL_TEMP_BUTTON_PIN = 2;
int INC_TEMP_PIN = 7;
int DEC_TEMP_PIN = 6;

Adafruit_7segment display = Adafruit_7segment();

bool coolMode;
bool heatMode;
bool showSetTempFlag;
float temp;
float setTemp;
float hystRes;

unsigned long lastMillis;

void setup()
{
    showSetTempFlag = false;

    display.begin(0x70);

    pinMode(COOL_PIN, OUTPUT);
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(SEL_TEMP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(INC_TEMP_PIN, INPUT_PULLUP);
    pinMode(DEC_TEMP_PIN, INPUT_PULLUP);

    // Create interrupt for showing the set temperature on display
    attachInterrupt(digitalPinToInterrupt(SEL_TEMP_BUTTON_PIN), showSetTemp_ISR, RISING);

    turnCoolMode(OFF);
    turnHeatMode(OFF);

    lastMillis = millis();

    setTemp = 24;
    hystRes = 1;

    // run loop code once
    temp = getTemp();

    display.print((double)temp, 1);
    display.writeDisplay();

    thermostatControl(temp);
}

void loop()
{
    if(isTimeIntervalElapsed(INTERVAL) == true)
    {
        temp = getTemp();
        
        if(showSetTempFlag == false)
        {
            display.print((double)temp, 1);
            display.writeDisplay();
        }

        thermostatControl(temp);
    }
    if(showSetTempFlag == true) // if show selected temp flag is true
    {
        display.print((double)setTemp, 1);
        display.writeDisplay();

        if(isTimeIntervalElapsed(INTERVAL) == true)
        {
            showSetTempFlag = false;
        }
    }
}

bool isTimeIntervalElapsed(int interval)
{
    unsigned long int currentMillis;
    currentMillis = millis();
    if(currentMillis - lastMillis >= interval * 1000)  // if more than one second passed
    {
        lastMillis = currentMillis; // update lastMillis
        return true;
    }
    return false;
}

float getTemp()
{
    int sensorInput;
    float temp;

    sensorInput = analogRead(TEMP_PIN); // read sensor
    temp = (float)sensorInput / 1024; // find percentage of input reading
    temp *= 5;  // multiply by 5 to get voltage
    temp -= 0.5; // substract the offset
    temp *= 100; // convert to degrees

    return temp;
}

void thermostatControl(float temp)
{
    if(!(temp >= setTemp - hystRes && temp <= setTemp + hystRes)) // if temp is outside hysteresis range
    {
        if(temp > setTemp + hystRes)
        {
            if(coolMode == OFF)
            {
                turnCoolMode(ON);
            }
            return;
        }
        if(temp < setTemp - hystRes)
        {
            if(heatMode == OFF)
            {
                turnHeatMode(ON);
            }
            return;
        }
    }
    else // if temp is in range
    {
        if(coolMode == ON)
        {
            turnCoolMode(OFF);
        }
        if(heatMode == ON)
        {
            turnHeatMode(OFF);
        }
    }
}

void turnCoolMode(bool onoff)
{
    if(onoff == ON) // turn cool mode ON
    {
        // turn on LED and do other things
        digitalWrite(COOL_PIN, HIGH);
        turnHeatMode(OFF);
        coolMode = ON;
    }
    else // turn cool mode OFF
    {
        digitalWrite(COOL_PIN, LOW);
        coolMode = OFF;
    }
}

void turnHeatMode(bool onoff)
{
    if(onoff == ON) // turn heat mode ON
    {
        // turn on LED and do other things
        digitalWrite(HEAT_PIN, HIGH);
        turnCoolMode(OFF);
        heatMode = ON;
    }
    else // turn heat mode OFF
    {
        digitalWrite(HEAT_PIN, LOW);
        heatMode = OFF;
    }
}

void showSetTemp_ISR()
{
    showSetTempFlag = true;

    if(digitalRead(INC_TEMP_PIN) == HIGH) // Increase temperature
    {
        setTemp += TEMP_ADJ_VALUE;
    }
    if(digitalRead(DEC_TEMP_PIN) == HIGH) // Decrease temperature
    {
        setTemp -= TEMP_ADJ_VALUE;
    }
}