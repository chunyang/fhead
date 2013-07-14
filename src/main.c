#include <stdio.h>
#include <stdlib.h>

#include "summary.h"
#include "print.h"

#define VERBOSE 0

void print_record_table(fit_summary *summary);

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
        // print_summary(summary);
        print_record_table(summary);
        destroy_summary(summary);
    }

    return EXIT_SUCCESS;
}

void print_record_table(fit_summary *summary)
{
    unsigned int num_events = summary->data.num_events;
    unsigned int num_records = summary->data.num_records;

    FIT_DATE_TIME *time = malloc(num_records * sizeof *time);
    FIT_UINT32 *distance = malloc(num_records * sizeof *distance);
    FIT_UINT16 *speed = malloc(num_records * sizeof *speed);

    unsigned int event_idx = 0;
    unsigned int record_idx = 0;

    FIT_DATE_TIME time_base = FIT_DATE_TIME_INVALID;
    FIT_DATE_TIME time_last_stop = FIT_DATE_TIME_INVALID;
    int time_adj = 0;

    while (event_idx < num_events) {
        FIT_EVENT_MESG event = summary->data.events[event_idx];

        // Only care about timer events
        if (event.event != FIT_EVENT_TIMER) {
            event_idx++;
            continue;
        }

        // printf("timestamp: %u\tevent type: %hhu\n", event.timestamp, event.event_type);

        if (event.event_type == FIT_EVENT_TYPE_START) {
            // printf("start time: %u\n", event.timestamp);
            if (time_base  == FIT_DATE_TIME_INVALID) {
                time_base = event.timestamp;
            }

            if (time_last_stop == FIT_DATE_TIME_INVALID) {
                time_last_stop = event.timestamp;
            }

            time_adj = event.timestamp - time_last_stop;

            while (record_idx < summary->data.event_index[event_idx+1]) {
                FIT_RECORD_MESG record = summary->data.records[record_idx];

                printf("base: %u\tlast: %u\tadj: %d\n", time_base, time_last_stop, time_adj);
                time[record_idx] = record.timestamp - time_base - time_adj;
                distance[record_idx] = record.distance;
                speed[record_idx] = record.speed;

                record_idx++;
            }

        } else if (event.event_type == FIT_EVENT_TYPE_STOP_ALL) {
            // printf("stop time: %u\n", event.timestamp);
            time_last_stop = event.timestamp;
        }

        event_idx++;
    }

    for (record_idx = 0; record_idx < num_records; record_idx++) {
        printf("%u\t%u\t%hu\n", time[record_idx], distance[record_idx],
                speed[record_idx]);
    }

    free(time);
    free(distance);
    free(speed);
}
