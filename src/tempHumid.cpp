#include "Wire.h"
#include "Adafruit_AHTX0.h"
#include "dataTASK.h"
#include <stdint.h>
#include "tempHumid.h"
#include "global.h"

#define AHT10_ADD               0x38
#define I2C_SDA_PIN             21      // pin 21 (Wire_SDA)
#define I2C_SCL_PIN             22      // pin 22 (Wire_SCL)
#define AHT10_ERROR_VAL        -99

Adafruit_AHTX0 aht;



void powerOn(uint8_t aht10_pin)
{
    digitalWrite(aht10_pin, HIGH);
    delay(50);
}

void powerOff(uint8_t aht10_pin)
{
    digitalWrite(aht10_pin, LOW);
}

bool readAHT10(uint8_t aht10_pin, float &temperature, float &humidity)
{
    powerOn(aht10_pin);

    if (!aht.begin())
    {
        if(aht10_pin == AHT10_IN_PIN)
        {
            Serial.println("❌ AHT10 IN sensor not found");
        }

        if(aht10_pin == AHT10_OUT_PIN)
        {
            Serial.println("❌ AHT10 OUT sensor not found");
        }

        powerOff(aht10_pin);
        return false;
    }

    sensors_event_t humidityEvent, tempEvent;
    aht.getEvent(&humidityEvent, &tempEvent);

    temperature = tempEvent.temperature;
    humidity = humidityEvent.relative_humidity;

    powerOff(aht10_pin);

    if(temperature == -50 && humidity == 0)
    {
        return false;
    }

    return true;
}


void initAHT10(void)
{
    pinMode(AHT10_IN_PIN, OUTPUT);
    pinMode(AHT10_OUT_PIN, OUTPUT);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);     // Initialize I2C

    powerOn(AHT10_IN_PIN);
    if (!aht.begin())
    {
        Serial.println("❌ AHT10 IN sensor not found");
    }
    powerOff(AHT10_IN_PIN);
    
    powerOn(AHT10_OUT_PIN);
    if (!aht.begin())
    {
        Serial.println("❌ AHT10 OUT sensor not found");
    }
    powerOff(AHT10_OUT_PIN);
}



bool getTempHumid(uint8_t aht10_pin)
{
    bool readAHT10_OK = false;

    if(aht10_pin == AHT10_IN_PIN)
    {
        tempHumidDataTemporary.tempHumidStructInUse = true;
        readAHT10_OK = readAHT10(AHT10_IN_PIN, tempHumidDataTemporary.tempCaravan, tempHumidDataTemporary.humidCaravan);
        //Serial.println("AHT10 IN run");

        if(readAHT10_OK == false)
        {
            tempHumidDataTemporary.tempCaravan = AHT10_ERROR_VAL;
            tempHumidDataTemporary.humidCaravan = AHT10_ERROR_VAL;
            
        }
        //Serial.print("AHT10 IN temp = "); Serial.println(tempHumidDataTemporary.tempCaravan);
        tempHumidDataTemporary.tempHumidStructInUse = false;
    }
    
    if(aht10_pin == AHT10_OUT_PIN)
    {
        tempHumidDataTemporary.tempHumidStructInUse = true;
        readAHT10_OK = readAHT10(AHT10_OUT_PIN, tempHumidDataTemporary.tempOutside, tempHumidDataTemporary.humidOutside);
        //Serial.println("AHT10 OUT run");
        if(readAHT10_OK == false)
        {
            tempHumidDataTemporary.tempOutside = AHT10_ERROR_VAL;
            tempHumidDataTemporary.humidOutside = AHT10_ERROR_VAL;
        }
        tempHumidDataTemporary.tempHumidStructInUse = false;
    }

    if(readAHT10_OK == false)
    {
        return false;
    }

    return true;
}

