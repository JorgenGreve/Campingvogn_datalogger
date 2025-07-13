// gprs_commands.h
#ifndef MESSAGE_QUEUE_CMD_H
#define MESSAGE_QUEUE_CMD_H

typedef enum
{
    SENSE_IDLE = 0,
    SENSE_SAMPLE,
} SenseCommand;

typedef enum
{
    GPS_IDLE = 0,
    GPS_SAMPLE,
} GpsCommand;


typedef enum
{
    GPRS_IDLE = 0,
    GPRS_TRANSMIT,
    GPRS_RECONNECT,
} GprsCommand;


typedef enum
{
    DATA_IDLE = 0,
    DATA_GPS_DATA_READY,
    DATA_SENSE_DATA_READY,
    DATA_TIME_TO_SAVE,
    DATA_SAVE_TO_RAM,
} DataCommand;


typedef enum
{
    MAIN_IDLE = 0,
} MainCommand;


typedef enum
{
    GPS_DATA_STRUCT = 0,
    TEMP_HUMID_DATA_STRUCT,
} IsTheStructInUse;

#endif
