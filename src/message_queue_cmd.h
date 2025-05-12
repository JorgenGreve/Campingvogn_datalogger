// gprs_commands.h
#ifndef MESSAGE_QUEUE_CMD_H
#define MESSAGE_QUEUE_CMD_H


typedef enum 
{
    MAIN_IDLE = 0,
    MAIN_SETUP_GPS,
    MAIN_SETUP_GPRS,
    MAIN_FETCH_GPS,
} MainCommand;

typedef enum 
{
    GPRS_IDLE = 0,
    GPRS_NO_COMMAND,
    GPRS_POST_DATA,
    GPRS_CHECK_SIGNAL,
    GPRS_RESET_MODEM,
} GprsCommand;

typedef enum 
{
    GPS_IDLE = 0,
    GPS_SETUP,
    GPS_FETCH_DATA,
    GPS_CHECK_SIGNAL,
    GPS_RESET,
    GPS_RESET_MODEM,
} GpsCommand;

#endif
