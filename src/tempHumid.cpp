#include "Wire.h"
#include "Adafruit_AHTX0.h"
#include "dataTASK.h"
#include <stdint.h>
#include "tempHumid.h"
#include "global.h"

#define MUX_ADD             0x70           // MUX I2C address


Adafruit_AHTX0 aht;


void initAHT10(void)
{
    Wire.begin(21, 22); // CONNECT DIRECTLY TO ONE AHT10 SENSOR
    if (!aht.begin())
    {
        Serial.print("AHT10 sensor on channel >undefined<"); Serial.println(" not found");
        //Serial.print("AHT10 sensor on channel "); Serial.print(muxChannel); Serial.println(" not found");
    }

}


void tcaSelect(uint8_t channel)
{
    if(channel > 7) return;
    Wire.beginTransmission(MUX_ADD);    // MUX address
    Wire.write(1 << channel);               // MUX channel select, 1<<0 = channel 0, 1<<1 = channel 1 (1<<channel)
    Wire.endTransmission();
}


bool getTempHumid(uint8_t muxChannel)
{
    //tcaSelect(muxChannel);
    //vTaskDelay(50 / portTICK_PERIOD_MS);
    
    sensors_event_t humidity, temp;
    if(aht.getEvent(&humidity, &temp))
    {
        tempHumidDataTemporary.tempHumidStructInUse = true;
        
        if(0 == muxChannel)         // Inside temp/humid data
        {
            tempHumidDataTemporary.tempCaravan = temp.temperature;
            tempHumidDataTemporary.humidCaravan = humidity.relative_humidity;
            tempHumidDataTemporary.tempHumidDataReadyCaravan = true;
        }
        else if(1 == muxChannel)    // Outside temp/humid data
        {
            tempHumidDataTemporary.tempOutside = temp.temperature;
            tempHumidDataTemporary.humidOutside = humidity.relative_humidity;
            tempHumidDataTemporary.tempHumidDataReadyOutside = true;
        }

        tempHumidDataTemporary.tempHumidStructInUse = false;
        return true;
    }
    else
    {
        Serial.println("AHT10: ERROR");
        return false;
    }
}

