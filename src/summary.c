#include "summary.h"

int update(fit_summary *summary, const FIT_UINT8 *mesg, FIT_UINT16 mesg_num);
int push_data(fit_summary *summary, const FIT_RECORD_MESG *mesg);
int resize_data(fit_summary *summary);
int resize_data_field(void **field, size_t nmemb, size_t size);

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
    summary->num_sessions = FIT_UINT16_INVALID;
    summary->sport = FIT_SPORT_INVALID;

    summary->data.num_records = 0;
    summary->data.max_records = MIN_RECORDS;
    summary->data.timestamps =
        malloc(MIN_RECORDS * sizeof *(summary->data.timestamps));
    summary->data.timer_times =
        malloc(MIN_RECORDS * sizeof *(summary->data.timer_times));
    summary->data.latitudes =
        malloc(MIN_RECORDS * sizeof *(summary->data.latitudes));
    summary->data.longitudes =
        malloc(MIN_RECORDS * sizeof *(summary->data.longitudes));
    summary->data.distances =
        malloc(MIN_RECORDS * sizeof *(summary->data.distances));
    summary->data.speeds =
        malloc(MIN_RECORDS * sizeof *(summary->data.speeds));

    return summary;
}

void destroy(fit_summary *summary)
{
    if (summary) {
        if (summary->data.timestamps) free(summary->data.timestamps);
        if (summary->data.timer_times) free(summary->data.timer_times);
        if (summary->data.latitudes) free(summary->data.latitudes);
        if (summary->data.longitudes) free(summary->data.longitudes);
        if (summary->data.distances) free(summary->data.distances);
        if (summary->data.speeds) free(summary->data.speeds);

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
            return push_data(summary, (const FIT_RECORD_MESG*) mesg);
        }
        default:
            return 1;
    }

    return 0;
}

int push_data(fit_summary *summary, const FIT_RECORD_MESG *mesg)
{
    if (summary->data.num_records == summary->data.max_records) {
        /* Increase size of data arrays */
        if (resize_data(summary)) {
            fprintf(stderr, "Unable to allocate storage for data!\n");
            exit(ENOMEM);
        }
    }

    unsigned int i = summary->data.num_records++;

    summary->data.timestamps[i] = mesg->timestamp;
    summary->data.latitudes[i] = mesg->position_lat;
    summary->data.longitudes[i] = mesg->position_long;
    summary->data.distances[i] = mesg->distance;
    summary->data.speeds[i] = mesg->speed;

    return 0;
}

/**
 * Double the size of data arrays in summary struct
 */
int resize_data(fit_summary *summary)
{
    int res = 0;

    summary->data.max_records *= 2;

    res |= resize_data_field((void**) &summary->data.timestamps,
                                summary->data.max_records,
                                sizeof *summary->data.timestamps);

    res |= resize_data_field((void**) &summary->data.timer_times,
                                summary->data.max_records,
                                sizeof *summary->data.timer_times);

    res |= resize_data_field((void**) &summary->data.latitudes,
                                summary->data.max_records,
                                sizeof *summary->data.latitudes);

    res |= resize_data_field((void**) &summary->data.longitudes,
                                summary->data.max_records,
                                sizeof *summary->data.longitudes);

    res |= resize_data_field((void**) &summary->data.distances,
                                summary->data.max_records,
                                sizeof *summary->data.distances);

    res |= resize_data_field((void**) &summary->data.speeds,
                                summary->data.max_records,
                                sizeof *summary->data.speeds);

    return res;
}

/**
 * Resize array to be nmemb elements of size
 */
int resize_data_field(void **field, size_t nmemb, size_t size)
{
    void *tmp;

    if (!(tmp = realloc(*field, nmemb * size))) {
        return 1;
    }

    *field = tmp;
    return 0;
}
