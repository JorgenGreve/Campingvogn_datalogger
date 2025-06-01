// gprs_commands.h
#ifndef MESSAGE_QUEUE_CMD_H
#define MESSAGE_QUEUE_CMD_H

typedef enum
{
    SENSE_IDLE = 0,
    SENSE_RUN,
} SenseCommand;

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
    SENSE_DATA_READY,
} DataCommand;


typedef enum
{
    MAIN_IDLE = 0,
    MAIN_ALL_DATA_READY,
    MAIN_IS_IT_TIME_TO_TX,
    MAIN_TRANSMIT_DATA,
} MainCommand;


typedef enum
{
    GPS_DATA_STRUCT = 0,
    TEMP_HUMID_DATA_STRUCT,
} IsTheStructInUse;

#endif
