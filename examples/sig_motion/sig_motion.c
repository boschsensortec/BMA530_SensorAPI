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
    struct bma530_int_map int_map = { 0 };
    struct bma5_int_conf_types int_config = { 0 };
    struct bma530_sig_motion conf = { 0 };
    struct bma530_feat_eng_gpr_0 gpr_0 = { 0 };
    struct bma530_int_status_types int_status = { 0 };

    int_config.int_src = BMA5_INT_1;
    int_status.int_src = BMA530_INT_STATUS_INT1;

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
    rslt = bma530_get_default_sig_motion_config(&conf, &dev);
    bma5_check_rslt("bma530_get_default_sig_motion_config", rslt);

    printf("block_size :: 0x%x\n", conf.block_size);
    printf("mcr_max :: 0x%x\n", conf.mcr_max);
    printf("mcr_min :: 0x%x\n", conf.mcr_min);
    printf("p2p_max :: 0x%x\n", conf.p2p_max);
    printf("p2p_min :: 0x%x\n", conf.p2p_min);

    conf.block_size = 0xFA;
    conf.mcr_max = 0x11;
    conf.mcr_min = 0x11;
    conf.p2p_max = 0x253;
    conf.p2p_min = 0x26;

    rslt = bma530_set_sig_motion_config(&conf, &dev);
    bma5_check_rslt("bma530_set_sig_motion_config", rslt);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.sig_mo_en = BMA5_ENABLE;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    rslt = bma530_get_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_get_int_map", rslt);

    /* Map sig motion */
    int_map.sig_mo_int_map = BMA530_SIG_MO_INT_MAP_INT1;
    rslt = bma530_set_int_map(&int_map, &dev);
    bma5_check_rslt("bma530_set_int_map", rslt);

    /* Map hardware interrupt pin configurations */
    rslt = bma5_get_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_get_int_conf", rslt);

    int_config.int_conf.int_mode = BMA5_INT1_MODE_PULSED_SHORT;
    int_config.int_conf.int_od = BMA5_INT1_OD_PUSH_PULL;
    int_config.int_conf.int_lvl = BMA5_INT1_LVL_ACTIVE_LOW;

    rslt = bma5_set_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_set_int_conf", rslt);

    printf("\nMove the board to get significant motion interrupt\n");

    for (;;)
    {
        rslt = bma530_get_int_status(&int_status, n_status, &dev);
        bma5_check_rslt("bma530_get_int_status", rslt);

        if (int_status.int_status.sig_mo_int_status & BMA5_ENABLE)
        {
            rslt = bma530_set_int_status(&int_status, n_status, &dev);
            bma5_check_rslt("bma530_set_int_status", rslt);

            printf("Significant motion interrupt occurred\n");

            break;
        }
    }

    bma5_coines_deinit();

    return rslt;
}
