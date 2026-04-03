#ifndef MPU_CONFIG
#define MPU_CONFIG

#include <stdint.h>

typedef enum{
    ERR_MPU_NOT_PRESENT, MPU_CONFIG_SUCCESS
}mpu_err_t;

mpu_err_t mpu_init(void);

#endif
