#ifndef __VL53L0X_USER_H
#define __VL53L0X_USER_H

#include "vl53l0x.h"

VL53L0X_Error vl53l0x_set_mode(VL53L0X_Dev_t *dev, u8 mode);
uint16_t vl53l0x_distance(VL53L0X_Dev_t *dev);

#endif