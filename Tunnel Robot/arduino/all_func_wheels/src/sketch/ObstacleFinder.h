// use infrared sensors to find obstacles

#ifndef ALL_FUNC_WHEELS_OBSTACLEFINDER_H
#define ALL_FUNC_WHEELS_OBSTACLEFINDER_H

#include "Arduino.h"
#include "IRPins.h"
#include "MiscDefinitions.h"

class ObstacleFinder
{
public:  // private
    static const int SAMPLE_COUNT = 300;

    int threshold;
    int values[IR_COUNT];

    void getAllIRValues() {

        long totals[IR_COUNT];
        // initialize
        for (size_t i = 0; i < IR_COUNT; ++i) {
            totals[i] = 0;
        }

        // take samples
        for (int i = SAMPLE_COUNT; i > 0; --i) {
            for (int sensorIndex = 0; sensorIndex < IR_COUNT; ++sensorIndex) {
                totals[sensorIndex] += analogRead(IR_PINS[sensorIndex]);
            }
        }

        // calculate average
        for (size_t i = 0; i < IR_COUNT; ++i) {
            values[i] = totals[i] / SAMPLE_COUNT;
        }
    }

public:
    void calibrateThreshold() {
        // In the starting position, there is wood to the right and to the back
        // and nothing to the front and left.
        // So we put the threshold in between those values

        getAllIRValues();

        threshold = ((((values[0] + values[1] + values[5] + values[6]) / 4)
                      + ((values[2] + values[3] + values[4]) / 3))
                     / 3);  // TODO: magic number 3 to try to err on the side of false positives

#ifdef VERBOSE
        Serial.print("obstacle threshold set to ");
        Serial.println(threshold);
#endif  // VERBOSE
    }

    /**
     * look for obstacles
     * @return string representing whether obstacles are seen in each direction
     * format: right, front, left, back
     */
    String getReport() {
        String report = "";

        getAllIRValues();  // set the values in the values array

        //check right
#ifdef VERBOSE
        Serial.print("IR_R1 giving ");
        Serial.print(values[0]);
#endif  // VERBOSE
        if (values[0] > threshold || values[1] > threshold)
            report += '1';
        else
            report += '0';

        //check front
        if (values[2] > threshold)
            report += '1';
        else
            report += '0';

        //check left
        if (values[3] > threshold || values[4] > threshold)
            report += '1';
        else
            report += '0';

        //check back
        if (values[5] > threshold || values[6] > threshold)
            report += '1';
        else
            report += '0';

        return report;
    }
};

#endif //ALL_FUNC_WHEELS_OBSTACLEFINDER_H
