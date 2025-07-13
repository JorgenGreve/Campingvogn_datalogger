#include <Arduino.h>

#define RESTART_CNT_DWN         5           // 1 is minimum, else cntDwn will overrun (underrun)


void ERRORhandler(const char *ERROR_IDENTIFIER)
{
    uint8_t cntDwn = RESTART_CNT_DWN;
    while(1)
    {
        if(cntDwn == RESTART_CNT_DWN)
        {
            Serial.println("");
            Serial.println("!!!!!!!!! ERROR !!!!!!!!!!!!!");
            Serial.print(ERROR_IDENTIFIER); Serial.println(" has been called");
            Serial.println("      System halted          ");
            Serial.print("Rebooting in ");
        }
        Serial.print(cntDwn); Serial.print(", ");
        cntDwn--;
        if(cntDwn == RESTART_CNT_DWN - RESTART_CNT_DWN)
        {
            Serial.print("");
            esp_restart();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

















