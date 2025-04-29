/*
 * tof_processing.cpp
 */

#include <stdio.h>
#include <math.h>
#include "tof_processing.h"
#include "geometry.h"
#include "params.h"
#include "dds.h"

int tof_mode;
bool dump_distances = false;


#define TARGET_STATUS(md,x,y)   md->target_status[x + y * imageWidth]
#define DISTANCE(md,x,y)        md->distance_mm[x + y * imageWidth]
#define VALID(st)               (st == 5 || st == 9)

float tof_orientation[] = { 45, 0, -45, 135, 180, -135 };

extern int __SENSOR_ID__;

TOFProcessor::TOFProcessor()
    : dump_distances(false), alarmMax(1), alarmCount(0), alarm(false), ball_got(false)
{
    threshold = param_get_int("TOF_THRESHOLD", 50);
    printf("[TOFProcessor] Starting with distance threshold = %d\n", threshold);
}

void TOFProcessor::print_distance_matrix()
{
    for (int y = 0 ; y <= imageWidth * (imageWidth - 1) ; y += imageWidth) {
        for (int x = 0; x < imageWidth ; x++) {
            printf("\t");
            int st = measurementData->target_status[x + y];
            if (VALID(st))
                printf("%d", measurementData->distance_mm[x + y] );
            else
                printf("*");
        }
        printf("\n");
    }
    printf("\n");
}


void TOFMatrixProcessor::process_data()
{
    if (dump_distances) {
        print_distance_matrix();
    }
}

TOFSpotProcessor::TOFSpotProcessor(int _slice_row, int _slice_col)
    : TOFProcessor(), slice_row(_slice_row), slice_col(_slice_col)
{
    DDS::insert("distance", 500);
    distance_handle = DDS::get_handle("distance");
    alarm_handle = DDS::get_handle("alarm");
    printf("[TOFSpotProcessor] Starting with slices (row, col) = %d, %d\n", slice_row, slice_col);
}


int TOFSpotProcessor::getMinColFromSlice()
{
    int min_val = -1;
    for (int i = 0; i < imageWidth; i++) {
        int st = TARGET_STATUS(measurementData, slice_col, i);
        if (VALID(st)) {
            int d = DISTANCE(measurementData, slice_col, i);
            //printf("%d ", d);
            if (min_val == -1)
                min_val = d;
            else if (d < min_val) {
                min_val = d;
            }
        }
    }
    //printf("\n");
    return min_val;
}


int TOFSpotProcessor::getMinRowFromSlice()
{
    int min_val = -1;
    for (int i = 0; i < imageWidth; i++) {
        int st = TARGET_STATUS(measurementData, i, slice_row);
        if (VALID(st)) {
            int d = DISTANCE(measurementData, i, slice_row);
            //printf("%d ", d);
            if (min_val == -1)
                min_val = d;
            else if (d < min_val) {
                min_val = d;
            }
        }
    }
    //printf("\n");
    return min_val;
}


int TOFSpotProcessor::getMinFromMatrix()
{
    int min_val = -1;
    for (int c = 0; c < imageWidth; c++) {
        for (int r = 0; r < imageWidth; r++) {
            int st = TARGET_STATUS(measurementData, c, r);
            if (VALID(st)) {
                int d = DISTANCE(measurementData, c, r);
                //printf("%d ", d);
                if (d > 0) {
                    if (min_val == -1)
                        min_val = d;
                    else if (d < min_val) {
                        min_val = d;
                    }
                }
            }
        }
    }
    //printf("\n");
    return min_val;
}


int TOFSpotProcessor::getMinFromSlice()
{
    if (slice_row >= 0)
        return getMinRowFromSlice();
    else if (slice_col >= 0)
        return getMinColFromSlice();
    else
        return getMinFromMatrix();
}

extern "C" void board_userled(int led, bool ledon);

void TOFSpotProcessor::process_data()
{
    int dist = getMinFromSlice();

    if (dist < 0)
        return;

    bool stale_alarm;

    DDS::read_handle(alarm_handle, (void *)&alarm, stale_alarm);

    if (!ball_got) {
        if (dist < threshold && !alarm) {
            ++alarmCount;
            if (alarmCount >= alarmMax) {
                ball_got = true;
                alarmCount = 0;
            }
        }
        else
            alarmCount = 0;
    }
    else {
        if (dist >= threshold && !alarm) {
            ++alarmCount;
            if (alarmCount >= alarmMax) {
                ball_got = false;
                alarmCount = 0;
                printf("GOOOOOLLLLL!!!\n");
                board_userled(GREEN_LED, true);
                alarm = true;
                DDS::publish_handle(alarm_handle, (void *)&alarm, sizeof(bool));
            }
        }
        else
            alarmCount = 0;
    }

    if (!alarm) {
        board_userled(GREEN_LED, false);
    }

    if (dump_distances) {
        printf("Distance %d, threshold %d\n",
               (int)dist, (int)threshold);
    }

    DDS::publish_handle(distance_handle, (void *)&dist, sizeof(int));

}


