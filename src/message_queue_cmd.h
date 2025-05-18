// gprs_commands.h
#ifndef MESSAGE_QUEUE_CMD_H
#define MESSAGE_QUEUE_CMD_H


typedef enum 
{
    MAIN_IDLE = 0,
    MAIN_SETUP_GPS,
    MAIN_SETUP_GPRS,
    MAIN_SETUP_DONE,
    MAIN_RUN_GPS,
    MAIN_RUN_TEMP_HUMID,
    MAIN_RUN_GPRS,
} MainCommand;

typedef enum 
{
    GPRS_IDLE = 0,
    GPRS_SETUP,
    GPRS_RUN,
} GprsCommand;

typedef enum 
{
    GPS_IDLE = 0,
    GPS_SETUP,
    GPS_RUN,
    GPS_FETCH_DATA,
    GPS_CHECK_SIGNAL,
    GPS_RESET,
    GPS_RESET_MODEM,
} GpsCommand;

#endif
