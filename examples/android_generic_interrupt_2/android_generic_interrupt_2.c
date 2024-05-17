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
    int8_t rslt;
    uint8_t n_ints = 1;
    uint8_t n_status = 1;
    uint8_t android_comp, get_android_comp;
    uint8_t gpr_ctrl_host = BMA5_ENABLE;
    struct bma530_int_map int_map;
    struct bma5_int_conf_types int_config;
    struct bma530_int_status_types int_status;
    struct bma530_generic_interrupt_types conf;
    struct bma530_feat_eng_gpr_0 gpr_0;
    struct bma5_dev dev;

    int_config.int_src = BMA5_INT_2;

    int_status.int_src = BMA530_INT_STATUS_INT2;

    conf.generic_interrupt = BMA530_GEN_INT_2;

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

    printf("Default configurations\n");
    rslt = bma530_get_default_generic_int_config(&conf, n_ints, &dev);
    bma5_check_rslt("bma530_get_default_generic_int_config", rslt);

    printf("slope_thres 0x%x\n", conf.gen_int.slope_thres);
    printf("comb_sel 0x%x\n", conf.gen_int.comb_sel);
    printf("axis_sel 0x%x\n", conf.gen_int.axis_sel);
    printf("hysteresis 0x%x\n", conf.gen_int.hysteresis);
    printf("criterion_sel 0x%x\n", conf.gen_int.criterion_sel);
    printf("acc_ref_up 0x%x\n", conf.gen_int.acc_ref_up);
    printf("duration 0x%x\n", conf.gen_int.duration);
    printf("wait_time 0x%x\n", conf.gen_int.wait_time);
    printf("quiet_time 0x%x\n", conf.gen_int.quiet_time);
    printf("ref_acc_x 0x%x\n", conf.gen_int.ref_acc_x);
    printf("ref_acc_y 0x%x\n", conf.gen_int.ref_acc_y);
    printf("ref_acc_z 0x%x\n", conf.gen_int.ref_acc_z);

    conf.gen_int.slope_thres = 0xA;
    conf.gen_int.comb_sel = 0x0;
    conf.gen_int.axis_sel = 0x7;
    conf.gen_int.hysteresis = 0x2;
    conf.gen_int.criterion_sel = 0x1;
    conf.gen_int.acc_ref_up = 0x1;
    conf.gen_int.duration = 0xA;
    conf.gen_int.wait_time = 0x3;
    conf.gen_int.quiet_time = 0x40;
    conf.gen_int.ref_acc_x = 0x0;
    conf.gen_int.ref_acc_y = 0x0;
    conf.gen_int.ref_acc_z = 0x800;

    rslt = bma530_set_generic_int_config(&conf, n_ints, &dev);
    bma5_check_rslt("bma530_set_generic_int_config", rslt);

    rslt = bma530_get_android_comp_mode(&android_comp, &dev);
    bma5_check_rslt("bma530_get_android_comp_mode", rslt);

    android_comp = 1;

    rslt = bma530_set_android_comp_mode(&android_comp, &dev);
    bma5_check_rslt("bma530_set_android_comp_mode", rslt);

    printf("Get Android compatibility mode\n\n");
    rslt = bma530_get_android_comp_mode(&get_android_comp, &dev);
    bma5_check_rslt("bma530_get_android_comp_mode", rslt);

    printf("Android compatibility mode : %d\n", get_android_comp);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.gen_int2_en = BMA5_ENABLE;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    /* Map generic interrupt 1 */
    int_map.gen_int2_int_map = BMA530_GEN_INT2_INT_MAP_INT2;
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

    printf("Shake the board to get generic interrupt 2 interrupt\n");

    for (;;)
    {
        rslt = bma530_get_int_status(&int_status, n_status, &dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.gen_int2_int_status & BMA5_ENABLE)
        {
            rslt = bma530_set_int_status(&int_status, n_status, &dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            printf("Generic interrupt 2 interrupt occurred\n");

            break;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
