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
int main(void)
{
    struct bma5_dev dev;
    int8_t rslt;
    uint8_t android_comp = 0;
    uint8_t get_android_comp = 0;
    uint8_t n_status = 1;
    uint8_t n_ints = 1;
    uint8_t gpr_ctrl_host = BMA5_ENABLE;
    struct bma530_int_map int_map = { 0 };
    struct bma5_int_conf_types int_config = { 0 };
    struct bma530_tilt conf = { 0 };
    struct bma530_feat_eng_gpr_0 gpr_0 = { 0 };
    struct bma530_int_status_types int_status = { 0 };

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

    rslt = bma530_get_android_comp_mode(&android_comp, &dev);
    bma5_check_rslt("bma530_get_android_comp_mode", rslt);

    android_comp = 1;

    rslt = bma530_set_android_comp_mode(&android_comp, &dev);
    bma5_check_rslt("bma530_set_android_comp_mode", rslt);

    printf("Get Android compatibility mode\n\n");
    rslt = bma530_get_android_comp_mode(&get_android_comp, &dev);
    bma5_check_rslt("bma530_get_android_comp_mode", rslt);

    printf("Android compatibility mode : %d\n", get_android_comp);

    printf("\nDefault configurations\n\n");
    rslt = bma530_get_tilt_config(&conf, &dev);
    bma5_check_rslt("bma530_get_tilt_config", rslt);

    printf("beta_acc_mean :: 0x%x\n", conf.beta_acc_mean);
    printf("min_tilt_angle :: 0x%x\n", conf.min_tilt_angle);
    printf("segment_size :: 0x%x\n", conf.segment_size);

    conf.beta_acc_mean = 0xf069;
    conf.min_tilt_angle = 0xd2;
    conf.segment_size = 0x64;

    rslt = bma530_set_tilt_config(&conf, &dev);
    bma5_check_rslt("bma530_set_tilt_config", rslt);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.tilt_en = BMA5_ENABLE;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    /* Map tilt */
    int_map.tilt_int_map = BMA530_TILT_INT_MAP_INT2;
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

    printf("\nTilt the board to get tilt interrupt\n");

    for (;;)
    {
        rslt = bma530_get_int_status(&int_status, n_status, &dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.tilt_int_status & BMA5_ENABLE)
        {
            rslt = bma530_set_int_status(&int_status, n_status, &dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            printf("Tilt interrupt occurred\n");

            break;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
