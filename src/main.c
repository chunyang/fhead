#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <fit/fit.h>
#include <fit/fit_convert.h>

#define TIME_BUF_SIZE 256

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
} fit_summary;

fit_summary* summarize(const char *file_name);
int update(fit_summary *summary, const FIT_UINT8 *mesg, FIT_UINT16 mesg_num);

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
        free(summary);
    }

    return EXIT_SUCCESS;
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
    fit_summary *summary = malloc(sizeof *summary);

    if (!summary) {
        fprintf(stderr, "Error allocating memory!\n");
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
                        /*fprintf(stderr, "Unknown message number %hd\n",
                                mesg_num);*/
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
        }
        default:
            return 1;
    }

    return 0;
}

void print(fit_summary* summary)
{
    char buf[TIME_BUF_SIZE];
    time_t t;
    struct tm *tmp;

    printf("Product information:\n"
            "  Manufacturer: %hu\n"
            "  Product: %hu\n"
            "  Serial number: %u\n"
            "  Software version: %hu\n"
            "  Hardware version: %hhu\n",
            summary->manufacturer,
            summary->product,
            summary->serial_number,
            summary->software_version,
            summary->hardware_version);

    t = summary->time_created;
    tmp = localtime(&t);
    strftime(buf, sizeof buf, "%X %x", tmp);
    printf("File information:\n"
            "  File type: %hhu\n"
            "  Number: %hu\n"
            "  Time: %s\n",
            summary->type,
            summary->number,
            buf);

}
