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
    uint16_t loop = 10;
    uint8_t n_ints = 1;
    uint8_t gpr_ctrl_host = BMA5_ENABLE;
    struct bma5_int_conf_types int_config = { 0 };
    struct bma530_step_cntr conf = { 0 };
    struct bma530_feat_eng_gpr_0 gpr_0 = { 0 };
    struct bma530_feat_eng_feat_out feat_out = { 0 };

    /* The step activities are listed in array. */
    const char *activity_output[4] = { "UNKNOWN", "STILL", "WALKING", "RUNNING" };

    int_config.int_src = BMA5_INT_1;

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

    /* Map hardware interrupt pin configurations */
    rslt = bma5_get_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_get_int_conf", rslt);

    int_config.int_conf.int_mode = BMA5_INT1_MODE_LATCHED;
    int_config.int_conf.int_od = BMA5_INT1_OD_PUSH_PULL;
    int_config.int_conf.int_lvl = BMA5_INT1_LVL_ACTIVE_HIGH;

    rslt = bma5_set_int_conf(&int_config, n_ints, &dev);
    bma5_check_rslt("bma5_set_int_conf", rslt);

    rslt = bma530_get_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_get_feat_eng_gpr_0", rslt);

    gpr_0.step_en = BMA5_ENABLE;

    rslt = bma530_set_feat_eng_gpr_0(&gpr_0, &dev);
    bma5_check_rslt("bma530_set_feat_eng_gpr_0", rslt);

    rslt = bma5_set_regs(BMA5_REG_FEAT_ENG_GPR_CTRL, &gpr_ctrl_host, 1, &dev);
    bma5_check_rslt("bma5_set_regs", rslt);

    printf("\nDefault configurations\n\n");
    rslt = bma530_get_default_step_counter_config(&conf, &dev);
    bma5_check_rslt("bma530_get_default_step_counter_config", rslt);

    printf("acc_mean_decay_coeff :: 0x%x\n", conf.acc_mean_decay_coeff);
    printf("activity_detection_factor :: 0x%x\n", conf.activity_detection_factor);
    printf("activity_detection_thres :: 0x%x\n", conf.activity_detection_thres);
    printf("en_half_step :: 0x%x\n", conf.en_half_step);
    printf("en_mcr_pp :: 0x%x\n", conf.en_mcr_pp);
    printf("en_step_dur_pp :: 0x%x\n", conf.en_step_dur_pp);
    printf("envelope_down_decay_coeff :: 0x%x\n", conf.envelope_down_decay_coeff);
    printf("envelope_down_thres :: 0x%x\n", conf.envelope_down_thres);
    printf("envelope_up_decay_coeff :: 0x%x\n", conf.envelope_up_decay_coeff);
    printf("envelope_up_thres :: 0x%x\n", conf.envelope_up_thres);
    printf("filter_cascade_enabled :: 0x%x\n", conf.filter_cascade_enabled);
    printf("filter_coeff_a_1 :: 0x%x\n", conf.filter_coeff_a_1);
    printf("filter_coeff_a_2 :: 0x%x\n", conf.filter_coeff_a_2);
    printf("filter_coeff_b_0 :: 0x%x\n", conf.filter_coeff_b_0);
    printf("filter_coeff_b_1 :: 0x%x\n", conf.filter_coeff_b_1);
    printf("filter_coeff_b_2 :: 0x%x\n", conf.filter_coeff_b_2);
    printf("filter_coeff_scale_a :: 0x%x\n", conf.filter_coeff_scale_a);
    printf("filter_coeff_scale_b :: 0x%x\n", conf.filter_coeff_scale_b);
    printf("mcr_thres :: 0x%x\n", conf.mcr_thres);
    printf("peak_duration_min_running :: 0x%x\n", conf.peak_duration_min_running);
    printf("peak_duration_min_walking :: 0x%x\n", conf.peak_duration_min_walking);
    printf("reset_counter :: 0x%x\n", conf.reset_counter);
    printf("sc_en :: 0x%x\n", conf.sc_en);
    printf("sd_en :: 0x%x\n", conf.sd_en);
    printf("step_buffer_size :: 0x%x\n", conf.step_buffer_size);
    printf("step_counter_increment :: 0x%x\n", conf.step_counter_increment);
    printf("step_dur_mean_decay_coeff :: 0x%x\n", conf.step_dur_mean_decay_coeff);
    printf("step_dur_thres :: 0x%x\n", conf.step_dur_thres);
    printf("step_duration_max :: 0x%x\n", conf.step_duration_max);
    printf("step_duration_window :: 0x%x\n", conf.step_duration_window);
    printf("watermark_level :: 0x%x\n", conf.watermark_level);

    conf.acc_mean_decay_coeff = 0xEAC8;
    conf.activity_detection_factor = 0x3;
    conf.activity_detection_thres = 0xF3C;
    conf.en_half_step = 0x0;
    conf.en_mcr_pp = 0x0;
    conf.en_step_dur_pp = 0x0;
    conf.envelope_down_decay_coeff = 0xD938;
    conf.envelope_down_thres = 0x84;
    conf.envelope_up_decay_coeff = 0xF1CC;
    conf.envelope_up_thres = 0x132;
    conf.filter_cascade_enabled = 0x1;
    conf.filter_coeff_a_1 = 0x41EF;
    conf.filter_coeff_a_2 = 0xE897;
    conf.filter_coeff_b_0 = 0x55F;
    conf.filter_coeff_b_1 = 0xABE;
    conf.filter_coeff_b_2 = 0x55F;
    conf.filter_coeff_scale_a = 0xE;
    conf.filter_coeff_scale_b = 0xE;
    conf.mcr_thres = 0x0;
    conf.peak_duration_min_running = 0xC;
    conf.peak_duration_min_walking = 0xC;
    conf.reset_counter = 0x0;
    conf.sc_en = 0x1;
    conf.sd_en = 0x1;
    conf.step_buffer_size = 0x7;
    conf.step_counter_increment = 0x100;
    conf.step_dur_mean_decay_coeff = 0xFD54;
    conf.step_dur_thres = 0x0;
    conf.step_duration_max = 0x4A;
    conf.step_duration_window = 0xA0;
    conf.watermark_level = 0x0;

    rslt = bma530_set_step_counter_config(&conf, &dev);
    bma5_check_rslt("bma530_set_step_counter_config", rslt);

    printf("\nMove the board in steps to perform step activity\n");

    /* Loop to get step activity. */
    while (loop)
    {
        /* Give delay to perform activity */
        dev.delay_us(1000000, dev.intf_ptr);

        rslt = bma530_get_feat_eng_feature_out(&feat_out, &dev);
        bma5_check_rslt("bma530_get_feat_eng_feature_out", rslt);

        /* Print the step activity output. */
        printf("Step activity = %s\n", activity_output[feat_out.activ_stat]);

        loop--;
    }

    bma5_coines_deinit();

    return rslt;
}
