/**
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <math.h>
#include "common.h"
#include "bma530_features.h"

/******************************************************************************/
/*!                Macro definition                                           */

/*! FIFO raw data buffer size */
#define BMA530_FIFO_RAW_DATA_BUFFER_SIZE  UINT16_C(520)

/*! Number of accel frames to be extracted from FIFO
 * Calculation:
 * fifo_watermark_level = 300, accel_frame_len = 3, header_byte = 1, sensortime = 3.
 * fifo_accel_frame_count = (300 / (3 + 1 + 3)) = 43 frames
 *
 * Additional frames given if received FIFO frames is more than WATERMARK_LEVEL set.
 */
#define ACCEL_FRAME_LEN                   UINT16_C(380)

/*! FIFO watermark level */
#define WATERMARK_LEVEL                   UINT16_C(300)

/*! Sensortime resolution in seconds */
#define SENSORTIME_RESOLUTION             (0.0003125f)

/*! Earth's gravity in m/s^2 */
#define GRAVITY_EARTH                     (9.80665f)

/******************************************************************************/
/*!                 Global Variables                                          */

/*! Number of accel frames to be extracted from FIFO */
uint16_t fifo_accel_frame_length = ACCEL_FRAME_LEN;

/*! Number of bytes of FIFO data */
uint8_t fifo_data[BMA530_FIFO_RAW_DATA_BUFFER_SIZE] = { 0 };

/*! Array of accelerometer and sensortime frames
 * Array size same as fifo_accel_frame_length */
struct bma5_sens_fifo_axes_data_8_bit fifo_acc_data[ACCEL_FRAME_LEN];

/******************************************************************************/
/*!           Static Function Declaration                                     */

/*!
 * @brief This internal API is used to enable accel and interrupt configuration settings.
 *
 *  @param[in] dev       : Structure instance of bma5_dev.
 *
 *  return Status of API
 */
static int8_t get_accel_and_int_settings(struct bma5_dev *dev);

/*!
 * @brief This internal API gets FIFO configurations.
 *
 * @param[in, out] fifo_conf       : Structure instance of bma5_fifo_conf.
 * @param[in] dev                    : Structure instance of bma5_dev.
 *
 * return Status of API
 */
static int8_t get_fifo_conf(const struct bma5_fifo_conf *fifo_conf, struct bma5_dev *dev);

/*!
 * @brief This internal API gets FIFO 8 bit data with FIFO full interrupt.
 *
 * @param[in] accel_length         : Store accel frame length.
 * @param[in] fifo_accel_data      : Structure instance of bma5_sens_fifo_axes_data_8_bit.
 * @param[in] fifoframe            : Structure instance of bma5_fifo_frame.
 * @param[in] fifo_conf            : Structure instance of bma5_fifo_conf.
 * @param[in] dev                  : Structure instance of bma5_dev.
 *
 * return Status of API
 */
static int8_t get_fifo_full_8_bit_data(struct bma5_sens_fifo_axes_data_8_bit *fifo_accel_data,
                                       struct bma5_fifo_frame fifoframe,
                                       const struct bma5_fifo_conf *fifo_conf,
                                       struct bma5_dev *dev);

/*!
 * @brief This internal API converts raw sensor values(LSB) to meters per seconds square.
 *
 *  @param[in] val       : Raw sensor value.
 *  @param[in] g_range   : Accel Range selected (2G, 4G, 8G, 16G).
 *  @param[in] bit_width : Resolution of the sensor.
 *
 *  @return Accel values in meters per second square.
 *
 */
static float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width);

/* Set FIFO water-mark level */
uint16_t fifo_watermark_level = WATERMARK_LEVEL;

/******************************************************************************/
int main(void)
{
    struct bma5_dev dev;
    int8_t rslt;
    struct bma530_int_map int_map, get_int_map;
    struct bma5_fifo_conf fifo_conf;

    /* Initialize FIFO frame structure */
    struct bma5_fifo_frame fifoframe = { 0 };

    enum bma5_context context;

    /* Assign context parameter selection */
    context = BMA5_SMARTPHONE;

    /* Interface reference is given as a parameter
     *         For I2C : BMA5_I2C_INTF
     *         For SPI : BMA5_SPI_INTF
     */
    rslt = bma5_interface_init(&dev, BMA5_I2C_INTF, context);
    bma5_check_rslt("bma5_interface_init", rslt);

    rslt = bma530_init(&dev);
    bma5_check_rslt("bma530_init", rslt);
    printf("BMA530 Chip ID is 0x%X\n", dev.chip_id);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    int_map.fifo_full_int_map = BMA530_FIFO_FULL_INT_MAP_INT1;
    rslt = bma530_set_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_set_int_map", rslt);

    rslt = bma530_get_int_map(&get_int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    rslt = get_accel_and_int_settings(&dev);
    bma5_check_rslt("get_accel_and_int_settings", rslt);

    /* Get FIFO configuration register */
    rslt = bma5_get_fifo_conf(&fifo_conf, &dev);
    bma5_check_rslt("bma5_get_fifo_conf", rslt);

    fifo_conf.fifo_cfg = BMA5_FIFO_CFG_ENABLE;
    fifo_conf.fifo_acc_x = BMA5_FIFO_ACC_X_ENABLE;
    fifo_conf.fifo_acc_y = BMA5_FIFO_ACC_Y_ENABLE;
    fifo_conf.fifo_acc_z = BMA5_FIFO_ACC_Z_ENABLE;
    fifo_conf.fifo_compression = BMA5_FIFO_COMPRESSION_ACC_8BIT;
    fifo_conf.fifo_sensor_time = BMA5_FIFO_SENSOR_TIME_EACH_FRAME;
    fifo_conf.fifo_size = BMA5_FIFO_SIZE_MAX_512_BYTES;
    fifo_conf.fifo_stop_on_full = BMA5_ENABLE;

    rslt = get_fifo_conf(&fifo_conf, &dev);
    bma5_check_rslt("get_fifo_conf", rslt);

    /* Update FIFO structure */
    fifoframe.data = fifo_data;

    rslt = bma5_set_fifo_wm(fifo_watermark_level, &dev);
    bma5_check_rslt("bma5_set_fifo_wm", rslt);

    rslt = get_fifo_full_8_bit_data(fifo_acc_data, fifoframe, &fifo_conf, &dev);

    bma5_coines_deinit();

    return rslt;
}

/*!
 * @brief This internal API is used to enable accel and interrupt configuration settings.
 */
static int8_t get_accel_and_int_settings(struct bma5_dev *dev)
{
    int8_t rslt;
    uint8_t n_ints = 1;
    uint8_t sensor_ctrl;
    struct bma5_acc_conf acc_cfg, get_acc_cfg;
    struct bma5_int_conf_types int_config;

    int_config.int_src = BMA5_INT_1;

    /* Get accel configurations */
    rslt = bma5_get_acc_conf_0(&sensor_ctrl, dev);
    bma5_check_rslt("bma5_get_acc_conf_0", rslt);

    rslt = bma5_get_acc_conf(&acc_cfg, dev);
    bma5_check_rslt("bma5_get_acc_conf", rslt);

    /* Set accel configurations */
    acc_cfg.acc_odr = BMA5_ACC_ODR_HZ_6K4;
    acc_cfg.acc_bwp = BMA5_ACC_BWP_NORM_AVG4;
    acc_cfg.power_mode = BMA5_POWER_MODE_HPM;

    acc_cfg.acc_range = BMA5_ACC_RANGE_MAX_2G;
    acc_cfg.acc_iir_ro = BMA5_ACC_IIR_RO_DB_60;
    acc_cfg.noise_mode = BMA5_NOISE_MODE_LOWER_POWER;
    acc_cfg.acc_drdy_int_auto_clear = BMA5_ACC_DRDY_INT_AUTO_CLEAR_DISABLED;

    rslt = bma5_set_acc_conf(&acc_cfg, dev);
    bma5_check_rslt("bma5_get_acc_conf", rslt);

    /* Enable accel */
    sensor_ctrl = BMA5_SENSOR_CTRL_ENABLE;

    rslt = bma5_set_acc_conf_0(sensor_ctrl, dev);
    bma5_check_rslt("bma5_set_acc_conf_0", rslt);

    rslt = bma5_get_acc_conf_0(&sensor_ctrl, dev);
    bma5_check_rslt("bma5_set_acc_conf_0", rslt);

    rslt = bma5_get_acc_conf(&get_acc_cfg, dev);
    bma5_check_rslt("bma5_get_acc_conf", rslt);

    printf("Sensor CTRL : %d\n", sensor_ctrl);
    printf("ODR : %d\n", get_acc_cfg.acc_odr);
    printf("BW : %d\n", get_acc_cfg.acc_bwp);
    printf("Power mode : %d\n", get_acc_cfg.power_mode);
    printf("Range : %d\n", get_acc_cfg.acc_range);
    printf("IIR RO : %d\n", get_acc_cfg.acc_iir_ro);
    printf("Noise mode : %d\n", get_acc_cfg.noise_mode);
    printf("Auto Int clear : %d\n", get_acc_cfg.acc_drdy_int_auto_clear);

    rslt = bma5_get_int_conf(&int_config, n_ints, dev);
    bma5_check_rslt("bma5_get_int_conf", rslt);

    int_config.int_conf.int_mode = BMA5_INT1_MODE_LATCHED;
    int_config.int_conf.int_od = BMA5_INT1_OD_PUSH_PULL;
    int_config.int_conf.int_lvl = BMA5_INT1_LVL_ACTIVE_HIGH;

    rslt = bma5_set_int_conf(&int_config, n_ints, dev);
    bma5_check_rslt("bma5_set_int_conf", rslt);

    return rslt;
}

/*!
 * @brief This internal API gets FIFO configurations.
 */
static int8_t get_fifo_conf(const struct bma5_fifo_conf *fifo_conf, struct bma5_dev *dev)
{
    int8_t rslt;
    struct bma5_fifo_conf read_fifo_conf = { 0 };

    /* Set FIFO configuration.
     * NOTE 1: FIFO works only on header mode */
    rslt = bma5_set_fifo_conf(fifo_conf, dev);
    bma5_check_rslt("bma5_set_fifo_conf", rslt);

    printf("\nSet FIFO conf\n");
    printf("fifo en %d\n", fifo_conf->fifo_cfg);
    printf("fifo_x_en %d\n", fifo_conf->fifo_acc_x);
    printf("fifo_y_en %d\n", fifo_conf->fifo_acc_y);
    printf("fifo_z_en %d\n", fifo_conf->fifo_acc_z);
    printf("fifo_compression_en %d\n", fifo_conf->fifo_compression);
    printf("fifo_sensor_time %d\n", fifo_conf->fifo_sensor_time);
    printf("fifo_size %d\n", fifo_conf->fifo_size);

    /* Get FIFO configuration register */
    rslt = bma5_get_fifo_conf(&read_fifo_conf, dev);
    bma5_check_rslt("bma5_get_fifo_conf", rslt);

    printf("\nGet FIFO conf\n");
    printf("fifo en %d\n", read_fifo_conf.fifo_cfg);
    printf("fifo_x_en %d\n", read_fifo_conf.fifo_acc_x);
    printf("fifo_y_en %d\n", read_fifo_conf.fifo_acc_y);
    printf("fifo_z_en %d\n", read_fifo_conf.fifo_acc_z);
    printf("fifo_compression_en %d\n", read_fifo_conf.fifo_compression);
    printf("fifo_sensor_time %d\n", read_fifo_conf.fifo_sensor_time);
    printf("fifo_size %d\n", read_fifo_conf.fifo_size);

    return rslt;
}

/*!
 * @brief This internal API gets FIFO 8 bit data with FIFO full interrupt.
 */
static int8_t get_fifo_full_8_bit_data(struct bma5_sens_fifo_axes_data_8_bit *fifo_accel_data,
                                       struct bma5_fifo_frame fifoframe,
                                       const struct bma5_fifo_conf *fifo_conf,
                                       struct bma5_dev *dev)
{
    int8_t rslt = BMA5_OK;
    uint8_t n_status = 1;
    struct bma530_int_status_types int_status = { 0 };
    uint8_t loop = 0;
    uint16_t idx = 0;
    float x = 0, y = 0, z = 0;

    int_status.int_src = BMA530_INT_STATUS_INT1;

    printf("Get FIFO data");

    while (loop < 3)
    {
        /* Get fifo full interrupt 2 status */
        rslt = bma530_get_int_status(&int_status, n_status, dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.fifo_full_int_status & BMA5_ENABLE)
        {
            printf("\n\nIteration %d\n\n", loop);

            /* Read FIFO data */
            rslt = bma5_read_fifo_data(&fifoframe, fifo_conf, dev);
            bma5_check_rslt("bma5_read_fifo_data", rslt);

            rslt = bma530_set_int_status(&int_status, n_status, dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            if (rslt == BMA5_OK)
            {
                /* Parse the FIFO data to extract accelerometer and sensortime data from the FIFO buffer */
                (void)bma5_extract_acc_sens_time_8_bit(fifo_accel_data, &fifoframe, fifo_conf, dev);

                printf(
                    "\nCount, Accel_LSB_X, Accel_LSB_Y, Accel_LSB_Z, Acc_ms2_X, Acc_ms2_Y, Acc_ms2_Z, Sensortime(s)\n");

                /* Print the parsed accelerometer and sensortime data from the FIFO buffer */
                for (idx = 0; idx < fifoframe.fifo_avail_frames; idx++)
                {
                    /* Converting lsb to meter per second squared for 8 bit resolution at 2G range */
                    x = lsb_to_ms2(fifo_accel_data[idx].x, (float)2, BMA5_8_BIT_RESOLUTION);
                    y = lsb_to_ms2(fifo_accel_data[idx].y, (float)2, BMA5_8_BIT_RESOLUTION);
                    z = lsb_to_ms2(fifo_accel_data[idx].z, (float)2, BMA5_8_BIT_RESOLUTION);

                    /* Print the data in m/s2 */
                    printf("%d,  %d,  %d,  %d,  %4.2f,  %4.2f,  %4.2f,  %.4lf s\n",
                           idx,
                           fifo_accel_data[idx].x,
                           fifo_accel_data[idx].y,
                           fifo_accel_data[idx].z,
                           x,
                           y,
                           z,
                           (fifo_accel_data[idx].sensor_time * SENSORTIME_RESOLUTION));
                }

                loop++;
            }
        }
    }

    return rslt;
}

/*!
 *  @brief This internal API converts raw sensor values(LSB) to meters per seconds square.
 */
static float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width)
{
    double power = 2;

    float half_scale = (float)((pow((double)power, (double)bit_width) / 2.0f));

    return (GRAVITY_EARTH * val * g_range) / half_scale;
}
