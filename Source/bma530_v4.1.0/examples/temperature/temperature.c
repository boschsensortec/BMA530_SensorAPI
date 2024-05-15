/**\
 * Copyright (c) 2023 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **/

#include <stdio.h>
#include "common.h"
#include "bma530_features.h"

/******************************************************************************/
int main(void)
{
    struct bma5_dev dev;
    int8_t rslt;
    uint8_t loop = 0;
    struct bma5_temp_conf config;
    struct bma5_sensor_status status;
    uint8_t temperature = 0;
    int8_t temp_celsius = 0;

    enum bma5_context context;

    /* Assign context parameter selection */
    context = BMA5_SMARTPHONE;

    /* Interface reference is given as a parameter
     *         For I2C : BMA5_I2C_INTF
     *         For SPI : BMA5_SPI_INTF
     */
    rslt = bma5_interface_init(&dev, BMA5_SPI_INTF, context);
    bma5_check_rslt("bma5_interface_init", rslt);

    rslt = bma530_init(&dev);
    bma5_check_rslt("bma530_init", rslt);
    printf("BMA530 Chip ID is 0x%X\n", dev.chip_id);

    /* Get temperature config */
    rslt = bma5_get_temp_conf(&config, &dev);
    bma5_check_rslt("bma5_get_temp_conf", rslt);

    config.temp_rate = BMA5_TEMP_RATE_HZ_25;
    config.temp_meas_src = BMA5_TEMP_MEAS_SRC_TMP_INT;
    config.temp_ext_sel = BMA5_TEMP_EXT_SEL_INT2;

    /* Set temperature config */
    rslt = bma5_set_temp_conf(&config, &dev);
    bma5_check_rslt("bma5_set_temp_conf", rslt);

    printf("\nCount, Temparature data\n");

    while (loop < 50)
    {
        /* Get temperature data ready status */
        rslt = bma5_get_sensor_status(&status, &dev);
        bma5_check_rslt("bma5_get_sensor_status", rslt);

        if (status.temperature_rdy & BMA5_ENABLE)
        {
            rslt = bma5_set_sensor_status(&status, &dev);
            bma5_check_rslt("bma5_set_sensor_status", rslt);

            /* Get temperature data */
            rslt = bma5_get_temp_data(&temperature, &dev);
            bma5_check_rslt("bma5_get_temp_data", rslt);

            if (rslt == BMA5_OK)
            {
                temp_celsius = (int8_t)(temperature + 23);
            }

            printf("%d,  \t%d(Deg C)\n", loop, temp_celsius);

            loop++;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
