// https://www.youtube.com/watch?v=XWQsqPQOW-U

/* MUX address
    A0  A1  A2  MUX address
    0   0   0   0x70
    1   0   0   0x71
    0   1   0   0x72
    1   1   0   0x73
    0   0   1   0x74
    1   0   1   0x75
    0   1   1   0x76
    1   1   1   0x77
*/

#include "Wire.h"
#include "Adafruit_AHTX0.h"
#include "data.h"
#include <stdint.h>
#include "tempHumid.h"

#define MUX_ADD             0x70           // MUX I2C address

Adafruit_AHTX0 aht;


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
    
    Wire.begin(21, 22); // CONNECT DIRECTLY TO ONE AHT10 SENSOR
    
    if (!aht.begin())
    {
        Serial.print("AHT10 sensor on channel "); Serial.print(muxChannel); Serial.println(" not found");
        return false;
    }
    else
    {
        sensors_event_t humidity, temp;
        if(aht.getEvent(&humidity, &temp))
        {
            tempHumidData.tempHumidStructInUse = true;
            
            if(0 == muxChannel)         // Inside temp/humid data
            {
                tempHumidData.tempCaravan = temp.temperature;
                tempHumidData.humidCaravan = humidity.relative_humidity;
                tempHumidData.tempHumidDataReadyCaravan = true;
            }
            else if(1 == muxChannel)    // Outside temp/humid data
            {
                tempHumidData.tempOutside = temp.temperature;
                tempHumidData.humidOutside = humidity.relative_humidity;
                tempHumidData.tempHumidDataReadyOutside = true;
            }

            tempHumidData.tempHumidStructInUse = false;
            return true;
        }
        else
        {
            return false;
        }
    }
}

