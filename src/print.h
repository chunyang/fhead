#ifndef PRINT_H
#define PRINT_H

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include <fit/fit_example.h>

// Convert cm to mi 
#define CM_TO_MI(m) ((m) * 0.00000621371192237334)

// Convert mm/s to mph
#define MMS_TO_MPH(m) ((m) * 0.0022369362920544)

// Convert mm/s to min/mi
#define MMS_TO_MINMI(s) (26822.3996649131 / (s))

// Convert semicircles to degrees
#define SEMICIRCLE_TO_DEG(s) ((s) * 8.38190317153931e-08)

#define TIME_BUF_SIZE 256
#define TIME_OFFSET 631065600   // 12 AM UTC Dec 31, 1989

static unsigned int print_indent_level;

void print_record(FIT_RECORD_MESG *mesg);
void print_event(FIT_EVENT_MESG *event);

void print_increase_indent();
void print_decrease_indent();

#endif
