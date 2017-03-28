// definitions of infrared pins

#ifndef ALL_FUNC_WHEELS_IRPINS_H
#define ALL_FUNC_WHEELS_IRPINS_H

#include "MiscDefinitions.h"

#define IR_L1 A14
#define IR_L2 A13
#define IR_BR A12  // not in order clockwise
#define IR_BL A11
#define IR_R1 A9
#define IR_R2 A10
#define IR_F1 A8

#define IR_COUNT 7

const pin IR_PINS[IR_COUNT] = {IR_R1, IR_R2, IR_F1, IR_L1, IR_L2, IR_BR, IR_BL};
// right, front, left, back - is the order the obstacle finder uses

#endif //ALL_FUNC_WHEELS_IRPINS_H
