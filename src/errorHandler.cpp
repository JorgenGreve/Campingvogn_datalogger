#include <Arduino.h>


void ERRORhandler(const char *ERROR_IDENTIFIER)
{
    uint8_t cntDwn = 10;
    while(1)
    {
        if(cntDwn == 10)
        {
            Serial.println("");
            Serial.println("!!!!!!!!! ERROR !!!!!!!!!!!!!");
            Serial.print(ERROR_IDENTIFIER); Serial.println(" has been called");
            Serial.println("      System halted          ");
            Serial.print("Rebooting in ");
        }
        Serial.print(cntDwn); Serial.print(", ");
        cntDwn--;
        if(cntDwn == 0)
        {
            Serial.print("");
            esp_restart();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

















