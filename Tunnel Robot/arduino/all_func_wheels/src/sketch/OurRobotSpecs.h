// definition that apply specifically to how our robot is built

#ifndef ALL_FUNC_WHEELS_OURROBOTSPECS_H
#define ALL_FUNC_WHEELS_OURROBOTSPECS_H

// TODO: these might be able to be tuned better
// WHEEL_WIDTH tuned by making turns accurate 90 degrees
// TWELVE_INCH_DISTANCE tuned by making forward accurate 12 inches
#ifdef MASTER_SLAVE_STRATEGY
#define WHEEL_WIDTH 1495  // distance from left wheel to right wheel - in units that the encoder gives me
#define TWELVE_INCH_DISTANCE 2456  // in units of the encoder
#endif  // MASTER_SLAVE_STRATEGY
#ifdef CARTESIAN_STRATEGY
#define WHEEL_WIDTH 1440
#define TWELVE_INCH_DISTANCE 2427
#endif  // CARTESIAN_STRATEGY

#endif //ALL_FUNC_WHEELS_OURROBOTSPECS_H
