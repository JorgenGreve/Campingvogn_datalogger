#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H


#define GPRS_INIT_ERROR         "GPRS_INIT_ERROR"
#define GPS_INIT_ERROR          "GPS_INIT_ERROR"
#define GPS_RETRY_ERROR         "GPS_RETRY_ERROR"
#define SENSE_RETRY_ERROR       "SENSE_RETRY_ERROR"

void ERRORhandler(const char *ERROR_IDENTIFIER);

#endif