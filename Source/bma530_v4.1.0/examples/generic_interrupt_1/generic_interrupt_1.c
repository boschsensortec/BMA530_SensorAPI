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
    uint8_t gpr_ctrl_host = BMA5_ENABLE;
    uint8_t n_ints = 1;
    uint8_t n_status = 1;

    struct bma530_int_map int_map;
    struct bma5_int_conf_types int_config;
    struct bma530_generic_interrupt_types conf, set_conf;
    struct bma530_int_status_types int_status;
    struct bma530_feat_eng_gpr_0 gpr_0;

    int_config.int_src = BMA5_INT_1;
    int_status.int_src = BMA530_INT_STATUS_INT1;
    conf.generic_interrupt = BMA530_GEN_INT_1;
    set_conf.generic_interrupt = BMA530_GEN_INT_1;

    /* Assign context parameter selection */
    enum bma5_context context;
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

    set_conf.gen_int.slope_thres = 0xA;
    set_conf.gen_int.comb_sel = 0x0;
    set_conf.gen_int.axis_sel = 0x7;
    set_conf.gen_int.hysteresis = 0x2;
    set_conf.gen_int.criterion_sel = 0x1;
    set_conf.gen_int.acc_ref_up = 0x1;
    set_conf.gen_int.duration = 0xA;
    set_conf.gen_int.wait_time = 0x3;
    set_conf.gen_int.quiet_time = 0x40;
    set_conf.gen_int.ref_acc_x = 0x0;
    set_conf.gen_int.ref_acc_y = 0x0;
    set_conf.gen_int.ref_acc_z = 0x800;

    rslt = bma530_set_generic_int_config(&set_conf, n_ints, &dev);
    bma5_check_rslt("bma530_set_generic_int_config", rslt);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.gen_int1_en = BMA5_ENABLE;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    /* Map generic interrupt 1 */
    int_map.gen_int1_int_map = BMA530_GEN_INT1_INT_MAP_INT1;
    rslt = bma530_set_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_set_int_map", rslt);

    /* Map hardware interrupt pin configurations */
    rslt = bma5_get_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_get_int_conf", rslt);

    int_config.int_conf.int_mode = BMA5_INT1_MODE_LATCHED;
    int_config.int_conf.int_od = BMA5_INT1_OD_PUSH_PULL;
    int_config.int_conf.int_lvl = BMA5_INT1_LVL_ACTIVE_HIGH;

    rslt = bma5_set_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_set_int_conf", rslt);

    printf("Shake the board to get generic interrupt 1 interrupt\n");

    for (;;)
    {
        rslt = bma530_get_int_status(&int_status, n_status, &dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.gen_int1_int_status & BMA5_ENABLE)
        {
            rslt = bma530_set_int_status(&int_status, n_status, &dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            printf("Generic interrupt 1 interrupt occurred\n");

            break;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
