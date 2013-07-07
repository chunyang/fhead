#ifndef SUMMARY_H
#define SUMMARY_H

#include <stdio.h>
#include <stdlib.h>

#include <fit/fit.h>
#include <fit/fit_convert.h>

#define MIN_RECORDS 64

typedef struct records {
    unsigned int num_records;
    unsigned int max_records;
    FIT_DATE_TIME *timestamps;  // s
    FIT_DATE_TIME *timer_times; // s
    FIT_SINT32 *latitudes;      // semicircles
    FIT_SINT32 *longitudes;     // semicircles
    FIT_UINT32 *distances;      // 100 * m,
    FIT_UINT16 *speeds;         // 1000 * m/s,
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

fit_summary* create_summary();
void destroy(fit_summary *summary);

fit_summary* summarize(const char *file_name);

#endif
