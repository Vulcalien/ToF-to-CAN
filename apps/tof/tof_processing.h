/*
 * tof_processing.h
 */

#ifndef __TOF_PROCESSING
#define __TOF_PROCESSING

#include "SparkFun_VL53L5CX_Library.h"
#include "shared_data.h"

#define TOF_SINGLE    0
#define TOF_MATRIX    1

class TOFProcessor {
 public:
    TOFProcessor();
    void on_tof_data(VL53L5CX_ResultsData * _measurementData, int _imageWidth) {
        measurementData = _measurementData;
        imageWidth = _imageWidth;
        process_data();
    };
    virtual void process_data() = 0;
    void print_distance_matrix();
    void dump(bool d) { dump_distances = d;};
    void set_alarm_max(int mx) { alarmMax = mx; alarmCount = 0; };
    void set_threshold(int th) { threshold = th; };
 protected:
    bool dump_distances;
    int alarmMax, alarmCount;
    int distance_offset;
    bool alarm;
    int heading;
    VL53L5CX_ResultsData * measurementData;
    int imageWidth;
    int threshold;
    void * distance_handle;
    void * alarm_handle;
    bool ball_got;
};


class TOFMatrixProcessor : public TOFProcessor {
public:
    TOFMatrixProcessor() : TOFProcessor() { };
    void process_data();
};

class TOFSpotProcessor : public TOFProcessor {
public:
    TOFSpotProcessor(int _slice_row, int _slice_col);
    void process_data();
private:
    int getMinColFromSlice();
    int getMinRowFromSlice();
    int getMinFromSlice();
    int getMinFromMatrix();
    int slice_row;
    int slice_col;
};


#endif
