#ifndef __VL53L0X_PORT_PIN_H
#define __VL53L0X_PORT_PIN_H

#define VL53L0X_XshutPort   GPIOC
#define VL53L0X_XshutPin    GPIO_PIN_13

#define VL53L0X_SDA_PIN GPIO_PIN_14
#define VL53L0X_SDA_PORT GPIOC
#define VL53L0X_SCL_PIN GPIO_PIN_15
#define VL53L0X_SCL_PORT GPIOC

#define CCC // GPIO时钟选择(A~G)，和上方SDA、SCL的端口对应
#define BBB // 如果SDA和SCL为同一端口，定义一行即可

#endif
