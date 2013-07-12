#ifndef SUMMARY_H
#define SUMMARY_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fit/fit.h>
#include <fit/fit_convert.h>

#include "print.h"

#define MIN_RECORDS 64
#define MIN_EVENTS 16

typedef struct records {
    unsigned int num_records;
    unsigned int max_records;
    FIT_RECORD_MESG *records;

    unsigned int num_events;
    unsigned int max_events;
    FIT_EVENT_MESG *events;
    unsigned int *event_index;  // Identifies where the event takes place within the records
} records;

typedef struct fit_summary {
    /* Product information */
    FIT_MANUFACTURER manufacturer;
    FIT_UINT16 product;
    FIT_UINT32Z serial_number;
    FIT_UINT16 software_version;
    FIT_UINT8 hardware_version;

    /* File information */
    FIT_FILE type;
    FIT_UINT16 number;          // Only set for files that are not created/erased.
    FIT_DATE_TIME time_created; // Only set for files that can be created/erased.

    /* Activity information */
    FIT_DATE_TIME total_wall_time;
    FIT_DATE_TIME total_timer_time;
    FIT_LOCAL_DATE_TIME local_timestamp;    // timestamp epoch expressed in local time, used to convert activity timestamps to local time 
    FIT_UINT16 num_sessions;
    FIT_SPORT sport;

    /* data */
    records data;
} fit_summary;

fit_summary* summarize(const char *file_name);

void destroy_summary(fit_summary *summary);

void print_summary(fit_summary* summary);

#endif
