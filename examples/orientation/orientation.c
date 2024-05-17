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
#include "common.h"
#include "bma530_features.h"

/******************************************************************************/
/*!                Macro definition                                           */

/* Name Orientation output macros */
#define FACE_UP            UINT8_C(0x00)
#define FACE_DOWN          UINT8_C(0x01)

#define PORTRAIT_UP_RIGHT  UINT8_C(0x00)
#define LANDSCAPE_LEFT     UINT8_C(0x01)
#define PORTRAIT_UP_DOWN   UINT8_C(0x02)
#define LANDSCAPE_RIGHT    UINT8_C(0x03)

/******************************************************************************/
int main(void)
{
    struct bma5_dev dev;
    int8_t rslt;
    uint8_t n_ints = 1;
    uint8_t n_status = 1;
    uint8_t gpr_ctrl_host = BMA5_ENABLE;
    struct bma530_int_map int_map = { 0 };
    struct bma5_int_conf_types int_config = { 0 };
    struct bma530_orient conf = { 0 };
    struct bma530_orient get_conf = { 0 };
    struct bma530_feat_eng_gpr_0 gpr_0 = { 0 };
    struct bma530_int_status_types int_status = { 0 };
    struct bma530_feat_eng_feat_out feat_out = { 0 };

    /* Variables to store the output of orientation. */
    uint8_t orientation_out = 0;
    uint8_t orientation_faceup_down = 0;

    int_config.int_src = BMA5_INT_2;
    int_status.int_src = BMA530_INT_STATUS_INT2;

    /* Assign context parameter selection */
    enum bma5_context context;
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

    printf("\nDefault configurations\n\n");
    rslt = bma530_get_orient_config(&conf, &dev);
    bma5_check_rslt("bma530_get_orient_config", rslt);

    printf("blocking :: 0x%x\n", conf.blocking);
    printf("hold_time :: 0x%x\n", conf.hold_time);
    printf("hysteresis :: 0x%x\n", conf.hysteresis);
    printf("mode :: 0x%x\n", conf.mode);
    printf("slope_thres :: 0x%x\n", conf.slope_thres);
    printf("theta :: 0x%x\n", conf.theta);
    printf("ud_en :: 0x%x\n", conf.ud_en);

    conf.hold_time = 0x5;
    conf.hysteresis = 0x20;
    conf.slope_thres = 0xCD;
    conf.theta = 0x27;

    conf.ud_en = 1;
    conf.blocking = 0;
    conf.mode = 0;
    rslt = bma530_set_orient_config(&conf, &dev);
    bma5_check_rslt("bma530_set_orient_config", rslt);

    printf("\n\nSet Upside down orientation detection : %d\n", conf.ud_en);
    printf("Set Blocking : %d\n", conf.blocking);
    printf("Set Mode : %d\n", conf.mode);

    rslt = bma530_get_orient_config(&get_conf, &dev);
    bma5_check_rslt("bma530_get_orient_config", rslt);

    printf("\n\nGet Upside down orientation detection : %d\n", get_conf.ud_en);
    printf("Get Blocking : %d\n", get_conf.blocking);
    printf("Get Mode : %d\n", get_conf.mode);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.orient_en = 0x01;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    /* Map orientation */
    int_map.orient_int_map = BMA530_ORIENT_INT_MAP_INT2;
    rslt = bma530_set_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_set_int_map", rslt);

    /* Map hardware interrupt pin configurations */
    rslt = bma5_get_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_get_int_conf", rslt);

    int_config.int_conf.int_mode = BMA5_INT2_MODE_PULSED_SHORT;
    int_config.int_conf.int_od = BMA5_INT2_OD_PUSH_PULL;
    int_config.int_conf.int_lvl = BMA5_INT2_LVL_ACTIVE_LOW;

    rslt = bma5_set_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_set_int_conf", rslt);

    printf("\nMove the board to get orientation interrupt\n");

    for (;;)
    {
        rslt = bma530_get_int_status(&int_status, n_status, &dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.orient_int_status & BMA5_ENABLE)
        {
            printf("\nOrientation interrupt occurred\n");

            rslt = bma530_get_feat_eng_feature_out(&feat_out, &dev);
            bma5_check_rslt("bma530_get_feat_eng_feature_out", rslt);

            orientation_out = feat_out.orientation_portrait_landscape;
            orientation_faceup_down = feat_out.orientation_face_up_down;

            printf("Orientation output : %d \n", orientation_out);
            printf("Orientation face-up/down output : %d\n", orientation_faceup_down);

            rslt = bma530_set_int_status(&int_status, n_status, &dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            switch (orientation_out)
            {
                case LANDSCAPE_LEFT:
                    printf("\nOrientation state is landscape left\n");
                    break;
                case LANDSCAPE_RIGHT:
                    printf("\nOrientation state is landscape right\n");
                    break;
                case PORTRAIT_UP_DOWN:
                    printf("\nOrientation state is portrait upside down\n");
                    break;
                case PORTRAIT_UP_RIGHT:
                    printf("\nOrientation state is portrait upright\n");
                    break;
                default:
                    printf("\nInvalid");
            }

            switch (orientation_faceup_down)
            {
                case FACE_UP:
                    printf("\nOrientation state is face up\n");
                    break;
                case FACE_DOWN:
                    printf("\nOrientation state is face down\n");
                    break;
                default:
                    printf("\nInvalid");
            }

            break;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
