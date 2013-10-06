#include "summary.h"

fit_summary* create_summary();
int update(fit_summary *summary, const FIT_UINT8 *mesg, FIT_UINT16 mesg_num);

int push_record(fit_summary *summary, const FIT_RECORD_MESG *mesg);
int push_event(fit_summary *summary, const FIT_EVENT_MESG *mesg);

int resize_records(fit_summary *summary);
int resize_events(fit_summary *summary);

int resize_field(void **field, size_t nmemb, size_t size);

fit_summary* create_summary()
{
    fit_summary *summary = malloc(sizeof *summary);

    if (!summary) {
        fprintf(stderr, "Error allocating memory!\n");
        return NULL;
    }

    summary->manufacturer = FIT_MANUFACTURER_INVALID;
    summary->product = FIT_UINT16_INVALID;
    summary->serial_number = FIT_UINT32Z_INVALID;
    summary->software_version = FIT_UINT16_INVALID;
    summary->hardware_version = FIT_UINT8_INVALID;

    summary->type = FIT_FILE_INVALID;
    summary->number = FIT_UINT16_INVALID;
    summary->time_created = FIT_DATE_TIME_INVALID;

    summary->total_wall_time = FIT_DATE_TIME_INVALID;
    summary->total_timer_time = FIT_DATE_TIME_INVALID;
    summary->local_timestamp = FIT_LOCAL_DATE_TIME_INVALID;
    /*summary->num_sessions = FIT_UINT16_INVALID;*/
    summary->sport = FIT_SPORT_INVALID;

    summary->data.num_records = 0;
    summary->data.max_records = MIN_RECORDS;
    summary->data.records =
        malloc(MIN_RECORDS * sizeof *summary->data.records);

    summary->data.num_events = 0;
    summary->data.max_events = MIN_EVENTS;
    summary->data.events =
        malloc(MIN_EVENTS * sizeof *summary->data.events);
    summary->data.event_index =
        malloc(MIN_EVENTS * sizeof *summary->data.event_index);

    summary->data.num_laps = 0;
    summary->data.max_laps = MIN_LAPS;
    summary->data.laps =
        malloc(MIN_LAPS * sizeof *summary->data.laps);
    summary->data.lap_index =
        malloc(MIN_LAPS * sizeof *summary->data.lap_index);

    summary->data.num_sessions = 0;
    summary->data.max_sessions = MIN_SESSIONS;
    summary->data.sessions =
        malloc(MIN_SESSIONS * sizeof *summary->data.sessions);
    summary->data.session_index =
        malloc(MIN_SESSIONS * sizeof *summary->data.session_index);

    return summary;
}

void destroy_summary(fit_summary *summary)
{
    if (summary) {
        if (summary->data.records) free(summary->data.records);
        if (summary->data.events) free(summary->data.events);
        if (summary->data.event_index) free(summary->data.event_index);
        if (summary->data.laps) free(summary->data.laps);
        if (summary->data.lap_index) free(summary->data.lap_index);
        if (summary->data.sessions) free(summary->data.sessions);
        if (summary->data.session_index) free(summary->data.session_index);

        free(summary);
    }
}

fit_summary* summarize(const char *file_name)
{
    /* For reading input files */
    FILE *file;
    FIT_UINT8 buf[8];
    FIT_CONVERT_RETURN convert_return = FIT_CONVERT_CONTINUE;
    FIT_UINT32 buf_size;
    FIT_UINT32 mesg_index = 0;

    /* For keeping track of and returning information */
    fit_summary *summary = create_summary();

    if (!summary) {
        return NULL;
    }

    /* Open file for reading */
    if (!(file = fopen(file_name, "rb"))) {
        fprintf(stderr, "Cannot access %s\n", file_name);
        return NULL;
    }

    /* Initialize FitConvert */
    FitConvert_Init(FIT_TRUE);

    /* Read messages */
    while (!feof(file) && (convert_return == FIT_CONVERT_CONTINUE)) {
        for (buf_size = 0; (buf_size < sizeof buf) && !feof(file); buf_size++) {
            buf[buf_size] = getc(file);
        }

        do {
            convert_return = FitConvert_Read(buf, buf_size);

            switch (convert_return) {
                case FIT_CONVERT_MESSAGE_AVAILABLE:
                {
                    const FIT_UINT8 *mesg = FitConvert_GetMessageData();
                    FIT_UINT16 mesg_num = FitConvert_GetMessageNumber();

                    if (update(summary, mesg, mesg_num)) {
#if VERBOSE
                        fprintf(stderr, "Unknown message number %hd\n",
                                mesg_num);
#endif
                    }

                    mesg_index++;
                }
                default:
                    break;
            }
        } while (convert_return == FIT_CONVERT_MESSAGE_AVAILABLE);
    }

    fclose(file);

    if (convert_return != FIT_CONVERT_END_OF_FILE) {
        fprintf(stderr, "Error reading %s. Data may be incomplete\n",
                file_name);
    }

    /* Calculate derived values */

    return summary;
}

int update(fit_summary *summary, const FIT_UINT8 *mesg, FIT_UINT16 mesg_num)
{
    switch (mesg_num) {
        case FIT_MESG_NUM_FILE_ID:
        {
            const FIT_FILE_ID_MESG *file_id = (const FIT_FILE_ID_MESG*) mesg;

            summary->manufacturer = file_id->manufacturer;
            summary->product = file_id->product;
            summary->serial_number = file_id->serial_number;

            summary->type = file_id->type;
            summary->number = file_id->number;
            summary->time_created = file_id->time_created;
            return 0;
        }
        case FIT_MESG_NUM_FILE_CREATOR:
        {
            const FIT_FILE_CREATOR_MESG *creator =
                (const FIT_FILE_CREATOR_MESG*) mesg;

            summary->software_version = creator->software_version;
            summary->hardware_version = creator->hardware_version;
            return 0;
        }
        case FIT_MESG_NUM_RECORD:
        {
            return push_record(summary, (const FIT_RECORD_MESG*) mesg);
        }
        case FIT_MESG_NUM_EVENT:
        {
            return push_event(summary, (const FIT_EVENT_MESG*) mesg);
        }
        case FIT_MESG_NUM_LAP:
        {
            return push_lap(summary, (const FIT_LAP_MESG*) mesg);
        }
        case FIT_MESG_NUM_SESSION:
        {
            return push_session(summary, (const FIT_SESSION_MESG*) mesg);
        }
        default:
            return 1;
    }

    return 0;
}

int push_record(fit_summary *summary, const FIT_RECORD_MESG *mesg)
{
    if (summary->data.num_records == summary->data.max_records) {
        /* Increase size of data arrays */
        if (resize_records(summary)) {
            fprintf(stderr, "Unable to allocate storage for data!\n");
            exit(ENOMEM);
        }
    }

    unsigned int i = summary->data.num_records++;

    memcpy(&summary->data.records[i], mesg, FIT_RECORD_MESG_SIZE);

    return 0;
}

int push_event(fit_summary *summary, const FIT_EVENT_MESG *mesg)
{
    if (summary->data.num_events == summary->data.max_events) {
        /* Increase the size of event arrays */
        if (resize_events(summary)) {
            fprintf(stderr, "Unable to allocate storage for events!\n");
            exit(ENOMEM);
        }
    }

    unsigned int i = summary->data.num_events++;

    summary->data.event_index[i] = summary->data.num_records;
    memcpy(&summary->data.events[i], mesg, FIT_EVENT_MESG_SIZE);

    return 0;
}

int push_lap(fit_summary *summary, const FIT_LAP_MESG *mesg)
{
    if (summary->data.num_laps == summary->data.max_laps) {
        /* Increase the size of lap arrays */
        if (resize_laps(summary)) {
            fprintf(stderr, "Unable to allocate storage for laps!\n");
            exit(ENOMEM);
        }
    }

    unsigned int i = summary->data.num_laps++;

    summary->data.lap_index[i] = summary->data.num_records;
    memcpy(&summary->data.laps[i], mesg, FIT_LAP_MESG_SIZE);

    return 0;
}

int push_session(fit_summary *summary, const FIT_SESSION_MESG *mesg)
{
    if (summary->data.num_sessions == summary->data.max_sessions) {
        /* Increase the size of session arrays */
        if (resize_sessions(summary)) {
            fprintf(stderr, "Unable to allocate storage for sessions!\n");
            exit(ENOMEM);
        }
    }

    unsigned int i = summary->data.num_sessions++;

    summary->data.session_index[i] = summary->data.num_records;
    memcpy(&summary->data.sessions[i], mesg, FIT_SESSION_MESG_SIZE);

    return 0;
}

/**
 * Double the size of data record arrays in summary struct
 */
int resize_records(fit_summary *summary)
{
    int res = 0;

    summary->data.max_records *= 2;

    res |= resize_field((void**) &summary->data.records,
                        summary->data.max_records,
                        sizeof *summary->data.records);

    return res;
}

/**
 * Double the size of event arrays in summary struct
 */
int resize_events(fit_summary *summary)
{
    int res = 0;

    summary->data.max_events *= 2;

    res |= resize_field((void**) &summary->data.events,
                        summary->data.max_events,
                        sizeof *summary->data.events);

    res |= resize_field((void**) &summary->data.event_index,
                        summary->data.max_events,
                        sizeof *summary->data.event_index);

    return res;
}

/**
 * Double the size of lap arrays in summary struct
 */
int resize_laps(fit_summary *summary)
{
    int res = 0;

    summary->data.max_laps *= 2;

    res |= resize_field((void**) &summary->data.laps,
                        summary->data.max_laps,
                        sizeof *summary->data.laps);

    res |= resize_field((void**) &summary->data.lap_index,
                        summary->data.max_laps,
                        sizeof *summary->data.lap_index);

    return res;
}

/**
 * Double the size of session arrays in summary struct
 */
int resize_sessions(fit_summary *summary)
{
    int res = 0;

    summary->data.max_sessions *= 2;

    res |= resize_field((void**) &summary->data.sessions,
                        summary->data.max_sessions,
                        sizeof *summary->data.sessions);

    res |= resize_field((void**) &summary->data.session_index,
                        summary->data.max_sessions,
                        sizeof *summary->data.session_index);

    return res;
}

/**
 * Resize array to be nmemb elements of size
 */
int resize_field(void **field, size_t nmemb, size_t size)
{
    void *tmp;

    if (!(tmp = realloc(*field, nmemb * size))) {
        return 1;
    }

    *field = tmp;
    return 0;
}

/**
 * Print summary to the output
 */
void print_summary(fit_summary* summary)
{
    int i;
    char buf[TIME_BUF_SIZE];
    time_t t;
    struct tm *tmp;

    t = summary->time_created + TIME_OFFSET;
    tmp = localtime(&t);
    strftime(buf, sizeof buf, "%x %X", tmp);
    iprintf("File information:\n");
    INC_INDENT();
        iprintf("File type: %hhu\n", summary->type);
        iprintf("Number: %hu\n", summary->number);
        iprintf("Time: %s\n", buf);
    DEC_INDENT();

    iprintf("Product information:\n");
    INC_INDENT();
        iprintf("Manufacturer: %hu\n", summary->manufacturer);
        iprintf("Product: %hu\n", summary->product);
        iprintf("Serial number: %lu\n", summary->serial_number);
        iprintf("Software version: %hu\n", summary->software_version);
        iprintf("Hardware version: %hhu\n", summary->hardware_version);
    DEC_INDENT();

    if (summary->data.num_records > 0) {
        int time_start = summary->data.records[0].timestamp;
        int time_offset;
        int record_num_pad_width = (int) log10(summary->data.num_records) + 1;

        iprintf("Records:\n");
        INC_INDENT();
            for (i = 0; i < summary->data.num_records; i++) {
                // Record number
                iprintf("%*d:\t", record_num_pad_width, i+1);
                FIT_RECORD_MESG *record = &summary->data.records[i];

                // Elapsed time
                if (record->timestamp != FIT_DATE_TIME_INVALID) {
                    time_offset = record->timestamp - time_start;
                    pretty_format_time(buf, TIME_BUF_SIZE, time_offset);
                    printf("%s\t", buf);
                }

                // Distance
                if (record->distance != FIT_SINT32_INVALID) {
                    printf("%.4f mi\t", CM_TO_MI(record->distance));
                }

                // Speed
                if (record->speed != FIT_UINT16_INVALID) {
                    printf("%.3f mph\t", MMS_TO_MPH(record->speed));
                }

                // Location
                if (record->position_lat != FIT_SINT32_INVALID) {
                    printf("%.5f,%.5f\t",
                            SEMICIRCLE_TO_DEG(record->position_lat),
                            SEMICIRCLE_TO_DEG(record->position_long));
                }

                // End of record
                printf("\n");
            };
        DEC_INDENT();

    }

    if (summary->data.num_laps > 0) {
        int lap_num_pad_width = (int) log10(summary->data.num_laps) + 1;

        iprintf("Laps:\n");
        INC_INDENT();
            for (i = 0; i < summary->data.num_laps; i++) {
                // Lap number
                iprintf("%*d:\t", lap_num_pad_width, i+1);
                FIT_LAP_MESG *lap = &summary->data.laps[i];

                // Timer time
                if (lap->total_timer_time != FIT_UINT32_INVALID) {
                    pretty_format_time(buf, TIME_BUF_SIZE,
                                       lap->total_timer_time / 1000.f);
                    printf("%s\t", buf);
                }

                // Distance
                if (lap->total_distance != FIT_UINT32_INVALID) {
                    printf("%.4f mi\t", CM_TO_MI(lap->total_distance));
                }

                // Average speed
                if (lap->avg_speed != FIT_UINT16_INVALID) {
                    printf("%.3f mph\t", MMS_TO_MPH(lap->avg_speed));
                }

                // Sport
                if (lap->sport != FIT_SPORT_INVALID) {
                    pretty_format_sport(buf, TIME_BUF_SIZE, lap->sport);
                    printf("%s\t", buf);
                }

                // End of lap
                printf("\n");
            }
        DEC_INDENT();
    }

    if (summary->data.num_sessions > 0) {
        int session_num_pad_width = (int) log10(summary->data.num_sessions)+1;

        iprintf("Sessions:\n");
        INC_INDENT();
            for (i = 0; i < summary->data.num_sessions; i++) {
                // Session number
                iprintf("%*d:\t", session_num_pad_width, i+1);
                FIT_SESSION_MESG *session = &summary->data.sessions[i];

                // Timer time
                if (session->total_timer_time != FIT_UINT32_INVALID) {
                    pretty_format_time(buf, TIME_BUF_SIZE,
                                       session->total_timer_time / 1000.f);
                    printf("%s\t", buf);
                }

                // Distance
                if (session->total_distance != FIT_UINT32_INVALID) {
                    printf("%.4f mi\t", CM_TO_MI(session->total_distance));
                }

                // Average speed
                if (session->avg_speed != FIT_UINT16_INVALID) {
                    printf("%.3f mph\t", MMS_TO_MPH(session->avg_speed));
                }

                // Sport
                if (session->sport != FIT_SPORT_INVALID) {
                    pretty_format_sport(buf, TIME_BUF_SIZE, session->sport);
                    printf("%s\t", buf);
                }

                // End of session
                printf("\n");
            }
        DEC_INDENT();
    }

    // printf("Data:\n");
    // for (i = 0; i < summary->data.num_records; i++) {
    //     /* printf("%g min/mi\n", SPEED_TO_PACE(summary->data.speeds[i])); */
    //     printf("%g\t%g\n", M_TO_MI(summary->data.distances[i]),
    //             SPEED_TO_PACE(summary->data.speeds[i]));
    // }

    // printf("Data:\n");
    // print_increase_indent();
    // for (i = 0; i < summary->data.num_records; i++) {
    //     printf("  record number: %u\n", i);
    //     print_record(&summary->data.records[i]);
    //     printf("\n");
    // }
    // print_decrease_indent();

    // printf("Events (%u):\n", summary->data.num_events);
    // print_increase_indent();
    // for (i = 0; i < summary->data.num_events; i++) {
    //     printf("  record index: %u\n", summary->data.event_index[i]);
    //     print_event(&summary->data.events[i]);
    //     printf("\n");
    // }
    // print_decrease_indent();
}
