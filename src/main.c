#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "summary.h"

#define VERBOSE 0

#define M_TO_MI(m) ((m) * 0.00000621371192237334)     // Convert cm to mi
#define SPEED_TO_PACE(s) (26822.3996649131/(s))  // Convert mm/s to min/mi

#define TIME_BUF_SIZE 256
#define TIME_OFFSET 631065600   // 12 AM UTC Dec 31, 1989

void print(fit_summary* summary);

int main(int argc, const char *argv[])
{
    fit_summary *summary;

    /* Check arguments */
    if (argc < 2) {
        printf("Usage: %s FIT-FILE\n", argv[0]);
        return EXIT_SUCCESS;
    }

    summary = summarize(argv[1]);

    if (summary) {
        print(summary);
        destroy(summary);
    }

    return EXIT_SUCCESS;
}

void print(fit_summary* summary)
{
    int i;
    char buf[TIME_BUF_SIZE];
    time_t t;
    struct tm *tmp;

    printf("Product information:\n"
            "  Manufacturer: %hu\n"
            "  Product: %hu\n"
            "  Serial number: %lu\n"
            "  Software version: %hu\n"
            "  Hardware version: %hhu\n",
            summary->manufacturer,
            summary->product,
            summary->serial_number,
            summary->software_version,
            summary->hardware_version);

    t = summary->time_created + TIME_OFFSET;
    tmp = localtime(&t);
    strftime(buf, sizeof buf, "%X %x", tmp);
    printf("File information:\n"
            "  File type: %hhu\n"
            "  Number: %hu\n"
            "  Time: %s\n",
            summary->type,
            summary->number,
            buf);

    // printf("Data:\n");
    // for (i = 0; i < summary->data.num_records; i++) {
    //     /* printf("%g min/mi\n", SPEED_TO_PACE(summary->data.speeds[i])); */
    //     printf("%g\t%g\n", M_TO_MI(summary->data.distances[i]),
    //             SPEED_TO_PACE(summary->data.speeds[i]));
    // }

    printf("Events (summary->data.num_events):\n");
    for (i = 0; i < summary->data.num_events; i++) {
        printf("  record index: %u\n"
                "  timestamp: %u\n"
                "  data: %u\n"
                "  data16: %hu\n"
                "  event: %hhu\n"
                "  event_type: %hhu\n"
                "  event_group: %hhu\n\n",
                summary->data.event_index[i],
                summary->data.events[i].timestamp,
                summary->data.events[i].data,
                summary->data.events[i].data16,
                summary->data.events[i].event,
                summary->data.events[i].event_type,
                summary->data.events[i].event_group);
    }
}
