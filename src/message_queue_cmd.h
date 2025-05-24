// gprs_commands.h
#ifndef MESSAGE_QUEUE_CMD_H
#define MESSAGE_QUEUE_CMD_H


typedef enum 
{
    GPS_IDLE = 0,
    GPS_RUN,
} GpsCommand;


typedef enum 
{
    GPRS_IDLE = 0,
    GPRS_SETUP,
    GPRS_RUN,
} GprsCommand;


typedef enum 
{
    DATA_IDLE = 0,
    GPS_DATA_READY,
    TEMP_HUMID_DATA_READY,
} DataCommand;


typedef enum 
{
    MAIN_IDLE = 0,
    MAIN_RUN_GPS,
    MAIN_RUN_TEMP_HUMID,
    MAIN_RUN_GPRS,
} MainCommand;

#endif
