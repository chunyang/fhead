#include "print.h"

void iprintf(const char *format, ...);

void print_record(FIT_RECORD_MESG *mesg)
{
    unsigned int i;
    int compressed_speed_distance_num_invalid = 0;
    int speed_1s_num_invalid = 0;

    if (mesg->timestamp != FIT_DATE_TIME_INVALID) {
        iprintf("timestamp: %u s\n", mesg->timestamp);
    }

    if (mesg->position_lat != FIT_SINT32_INVALID) {
        iprintf("latitude: %.5f %c\n",
                fabs(SEMICIRCLE_TO_DEG(mesg->position_lat)),
                mesg->position_lat >= 0 ? 'N' : 'S');
    }

    if (mesg->position_long != FIT_SINT32_INVALID) {
        iprintf("longitude: %.5f %c\n",
                fabs(SEMICIRCLE_TO_DEG(mesg->position_long)),
                mesg->position_long >= 0 ? 'E' : 'W');
    }

    if (mesg->distance != FIT_UINT32_INVALID) {
        iprintf("distance: %.4f mi\n", CM_TO_MI(mesg->distance));
    }

    if (mesg->time_from_course != FIT_SINT32_INVALID) {
        iprintf("time from course: %.3f s\n",
                mesg->time_from_course / 1000.f);
    }

    if (mesg->total_cycles != FIT_UINT32_INVALID) {
        iprintf("total cycles: %u\n", mesg->total_cycles);
    }

    if (mesg->accumulated_power != FIT_UINT32_INVALID) {
        iprintf("accumulated power: %u W\n", mesg->accumulated_power);
    }

    if (mesg->altitude != FIT_UINT16_INVALID) {
        iprintf("altitude: %.1f m\n", mesg->altitude / 5.f - 500.f);
    }

    if (mesg->speed != FIT_UINT16_INVALID) {
        iprintf("speed: %.3f mph\n", MMS_TO_MPH(mesg->speed));
        float pace = MMS_TO_MINMI(mesg->speed);
        iprintf("pace: %u:%02u / mi\n", (unsigned int) pace,
                (unsigned int) (fmod(pace, (unsigned int) pace) * 60.f));
    }

    if (mesg->power != FIT_UINT16_INVALID) {
        iprintf("power: %hu W\n", mesg->power);
    }

    if (mesg->grade != FIT_SINT16_INVALID) {
        iprintf("grade: %.2f %%\n", mesg->grade / 100.f);
    }

    if (mesg->compressed_accumulated_power != FIT_UINT16_INVALID) {
        iprintf("compressed accumulated power: %hu\n",
                mesg->compressed_accumulated_power);
    }

    if (mesg->vertical_speed != FIT_SINT16_INVALID) {
        iprintf("vertical speed: %.3f m/s\n", mesg->vertical_speed / 1000.f);
    }

    if (mesg->calories != FIT_UINT16_INVALID) {
        iprintf("calories: %hu kcal\n", mesg->calories);
    }

    if (mesg->cadence256 != FIT_UINT16_INVALID) {
        iprintf("cadence: %.3f rpm\n", mesg->cadence256 / 256.f);
    }

    if (mesg->heart_rate != FIT_UINT8_INVALID) {
        iprintf("heart rate: %hhu bpm\n", mesg->heart_rate);
    }

    if (mesg->cadence != FIT_UINT8_INVALID) {
        iprintf("cadence: %hhu rpm\n", mesg->cadence);
    }

    // for (i = 0; i < FIT_RECORD_MESG_COMPRESSED_SPEED_DISTANCE_COUNT; i++) {
    //     if (mesg->compressed_speed_distance[i] == FIT_BYTE_INVALID) {
    //         compressed_speed_distance_num_invalid++;
    //     }
    // }
    // if (compressed_speed_distance_num_invalid != FIT_RECORD_MESG_COMPRESSED_SPEED_DISTANCE_COUNT) {
    //     iprintf("\n", mesg->compressed_speed_distance);
    //     // FIXME
    // }

    if (mesg->resistance != FIT_UINT8_INVALID) {
        iprintf("resistance: %hhu (0 is min, 254 is max)\n",
                mesg->resistance);
    }

    if (mesg->cycle_length != FIT_UINT8_INVALID) {
        iprintf("cycle length: %.2f\n", mesg->cycle_length / 100.f);
    }

    if (mesg->temperature != FIT_SINT8_INVALID) {
        iprintf("temperature: %hhd C\n", mesg->temperature);
    }

    // for (i = 0; i < FIT_RECORD_MESG_SPEED_1S_COUNT; i++) {
    //     if (mesg->speed_1s[i] == FIT_BYTE_INVALID) {
    //         speed_1s_num_invalid++;
    //     }
    // }
    // if (speed_1s_num_invalid != FIT_RECORD_MESG_SPEED_1S_COUNT) {
    //     iprintf("\n", mesg->speed_1s);
    //     // FIXME
    // }

    if (mesg->cycles != FIT_UINT8_INVALID) {
        iprintf("cycles: %hhu\n", mesg->cycles);
    }

    // if (mesg->left_right_balance != FIT_LEFT_RIGHT_BALANCE_INVALID) {
    //     iprintf("\n", mesg->left_right_balance);
    //     // FIXME
    // }

    if (mesg->gps_accuracy != FIT_UINT8_INVALID) {
        iprintf("gps accuracy: %hhu m\n", mesg->gps_accuracy);
    }

    if (mesg->left_torque_effectiveness != FIT_UINT8_INVALID) {
        iprintf("left torque effectiveness: %.1f %%\n",
                mesg->left_torque_effectiveness * 0.5f);
    }

    if (mesg->right_torque_effectiveness != FIT_UINT8_INVALID) {
        iprintf("right torque effectiveness: %.1f %%\n",
                mesg->right_torque_effectiveness * 0.5f);
    }

    if (mesg->left_pedal_smoothness != FIT_UINT8_INVALID) {
        iprintf("left pedal smoothness: %.1f %%\n",
                mesg->left_pedal_smoothness * 0.5f);
    }

    if (mesg->right_pedal_smoothness != FIT_UINT8_INVALID) {
        iprintf("right pedal smoothness: %.1f %%\n",
                mesg->right_pedal_smoothness * 0.5f);
    }

    if (mesg->combined_pedal_smoothness != FIT_UINT8_INVALID) {
        iprintf("combined pedal smoothness: %.1f %%\n",
                mesg->combined_pedal_smoothness * 0.5f);
    }
}

void print_event(FIT_EVENT_MESG *mesg)
{
    if (mesg->timestamp != FIT_DATE_TIME_INVALID) {
        iprintf("timestamp: %u\n", mesg->timestamp);
    }

    if (mesg->data != FIT_UINT32_INVALID) {
        iprintf("data: %u\n", mesg->data);
    }

    if (mesg->data16 != FIT_UINT16_INVALID) {
        iprintf("data16: %hu\n", mesg->data16);
    }

    if (mesg->event != FIT_EVENT_INVALID) {
        iprintf("event: %hhu\n", mesg->event);
    }

    if (mesg->event_type != FIT_EVENT_TYPE_INVALID) {
        iprintf("event type: %hhu\n", mesg->event_type);
    }

    if (mesg->event_group != FIT_UINT8_INVALID) {
        iprintf("event type: %hhu\n", mesg->event_group);
    }
}

/**
 * Indented printf
 */
void iprintf(const char *format, ...)
{
    unsigned int i;
    for (i = 0; i < print_indent_level; i++) {
        printf("  ");
    }

    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

void print_increase_indent()
{
    print_indent_level++;
}

void print_decrease_indent()
{
    if (print_indent_level > 0) print_indent_level--;
}
