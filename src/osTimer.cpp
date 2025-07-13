#include "Arduino.h"
#include "osTimer.h"
#include "global.h"
#include "message_queue_cmd.h"
#include "message_queue.h"

// Timer callback to check GPRS connection
TimerHandle_t gpsSampleTimer;
TimerHandle_t senseSampleTimer;
TimerHandle_t gprsCheckTimer;
TimerHandle_t gprsTestConnLoatTimer;
TimerHandle_t gpsTestSigLostTimer;

void createOsTimers(void)
{
    /********************** GPS SAMPLE TIMER  *********************************/
    gpsSampleTimer = xTimerCreate("gpsSampleTimer",
                           pdMS_TO_TICKS(2000),         // Countdown time in ms
                           pdTRUE,                      // Auto-reload
                           NULL,                        // Timer ID (valgfri)
                           osTimerCallback);            // Callback
    vTimerSetTimerID(gpsSampleTimer, (void*)OS_TIMER_GPS_SAMPLE_TIMER);
    startOsTimer(gpsSampleTimer);

    /********************** SENSE SAMPLE TIMER  *********************************/
    senseSampleTimer = xTimerCreate("senseSampleTimer",
                           pdMS_TO_TICKS(2000),         // Countdown time in ms
                           pdTRUE,                      // Auto-reload
                           NULL,                        // Timer ID (valgfri)
                           osTimerCallback);            // Callback
    vTimerSetTimerID(senseSampleTimer, (void*)OS_TIMER_SENSE_SAMPLE_TIMER);
    startOsTimer(senseSampleTimer);
    
    /********************** GPRS CHECK CONNECTION TIMER  **********************/
    gprsCheckTimer = xTimerCreate("gprsCheckTimer",
                           pdMS_TO_TICKS(10000),        // Countdown time in ms
                           pdTRUE,                      // Auto-reload
                           NULL,                        // Timer ID (valgfri)
                           osTimerCallback);            // Callback
    vTimerSetTimerID(gprsCheckTimer, (void*)OS_TIMER_GPRS_CHECK_TIMER);
    startOsTimer(gprsCheckTimer);

    /********************** TEST GPRS CONNECTION LOST TIMER  **********************/
    /*gprsTestConnLoatTimer = xTimerCreate("gprsTestConnLoatTimer",
                           pdMS_TO_TICKS(30000),        // Countdown time in ms
                           pdTRUE,                      // Auto-reload
                           NULL,                        // Timer ID (valgfri)
                           osTimerCallback);            // Callback
    vTimerSetTimerID(gprsTestConnLoatTimer, (void*)OS_TIMER_GPRS_TEST_LOST_CONN);
    startOsTimer(gprsTestConnLoatTimer);*/

    /********************** TEST GPS SIGNAL LOST TIMER  **********************/
    /*gpsTestSigLostTimer = xTimerCreate("gpsTestSigLostTimer",
                           pdMS_TO_TICKS(30000),        // Countdown time in ms
                           pdFALSE,                     // Auto-reload
                           NULL,                        // Timer ID (valgfri)
                           osTimerCallback);            // Callback
    vTimerSetTimerID(gpsTestSigLostTimer, (void*)OS_TIMER_GPS_TEST_LOST_SIG);
    startOsTimer(gpsTestSigLostTimer);*/
}



void startOsTimer(TimerHandle_t xTimer)
{
    int id = (int)(intptr_t)pvTimerGetTimerID(xTimer);
    bool timerStatus = false;

    if(xTimer != NULL)
    {
        timerStatus = xTimerStart(xTimer, 0);
        Serial.print("OS TIMER: Timer started with ID = "); Serial.println(id);
    }
    else
    {
        Serial.print("OS TIMER: ERROR timer is already started. ID = "); Serial.println(id);
    }

    if(timerStatus == pdFAIL)
    {
        Serial.print("OS TIMER: ERROR could not start timer. ID = "); Serial.println(id);
    }
}



void osTimerCallback(TimerHandle_t xTimer)
{
    int id = (int)(intptr_t)pvTimerGetTimerID(xTimer);

    //Serial.print("OS TIMER: Timer called -> callback ID = "); Serial.println(id);

    switch (id)
    {
    case OS_TIMER_GPS_SAMPLE_TIMER:
        {
            GpsCommand gpsCmd = GPS_SAMPLE;
            xQueueSend(gpsQueue, &gpsCmd, pdMS_TO_TICKS(500));
        }
        break;
    
    case OS_TIMER_SENSE_SAMPLE_TIMER:
        {
            SenseCommand senseCmd = SENSE_SAMPLE;
            xQueueSend(senseQueue, &senseCmd, pdMS_TO_TICKS(500));
        }
        break;

    case OS_TIMER_GPRS_CHECK_TIMER:
        if(!checkGPRS())
        {
            GprsCommand gprsCmd = GPRS_RECONNECT;
            xQueueSend(gprsQueue, &gprsCmd, pdMS_TO_TICKS(500));
        }
        break;

    case OS_TIMER_GPRS_TEST_LOST_CONN:
        {
            static uint8_t cnt = 0;

            if(cnt == 0)    // Stop GPRS connection
            {
                Serial.println("💣💣💣 Forcerer GPRS drop med simulateGprsDrop = true 💣💣💣");
                simulateGprsDrop = true;
                cnt = 0;
            }
            else            // Restart GPRS connection
            {
                Serial.println("💣💣💣 Forcerer GPRS drop med simulateGprsDrop = false 💣💣💣");
                simulateGprsDrop = false;
            }
            cnt++;
        }
        break;

    case OS_TIMER_GPS_TEST_LOST_SIG:
        Serial.println("💣💣💣 Forcerer GPS signal drop 💣💣💣");
        modem.sendAT("+CGPIO=0,48,1,0"); // Disable GPS power
        break;

    default:
        Serial.print("OS TIMER: ERROR unknown timer callback ID = "); Serial.println(id);
        break;
    }



}



