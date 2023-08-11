/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "BLDCDriver.h"
#include "foc_utils.h"
#include "time_utils.h"
#include "defaults.h"
#include "drivers/hardware_api.h"
#include "driver/mcpwm_prelude.h"

#define _PWM_FREQUENCY 20000                           /*!< default frequency of MCPWM */
#define _PWM_FREQUENCY_MAX 50000                       /*!< Max frequency of MCPWM */
#define _PWM_TIMEBASE_RESOLUTION_HZ (10 * 1000 * 1000) /*!< Resolution of MCPWM */

void hal_bldc_6pwm_init();
void hal_bldc_6pwm_write();

class BLDCDriver6PWM : public BLDCDriver
{
public:
    /**
     * @brief Construct a new BLDCDriver6PWM object
     *
     * @param phA_h
     * @param phA_l
     * @param phB_h
     * @param phB_l
     * @param phC_h
     * @param phC_l
     * @param en
     */
    BLDCDriver6PWM(int phA_h, int phA_l, int phB_h, int phB_l, int phC_h, int phC_l, int en = NOT_SET);

    /**
     * @brief Motor hardware init function, only support MCPWM.
     *
     * @return
     *     - 0 Failed
     *     - 1 Success
     */
    int init() override;

    /**
     * @brief Motor hardware init function, only support MCPWM.
     *
     * @return
     *     - 0 Failed
     *     - 1 Success
     */
    int deinit();

    /**
     * @brief Motor disable function.
     *
     */
    void disable() override;

    /**
     * @brief Motor enable function.
     *
     */
    void enable() override;

    // hardware variables
    int pwmA_h, pwmA_l; //!< phase A pwm pin number
    int pwmB_h, pwmB_l; //!< phase B   pin number
    int pwmC_h, pwmC_l; //!< phase C pwm pin number
    int enable_pin;     //!< enable pin number
    bool enable_active_high = true;

    float dead_zone; //!< a percentage of dead-time(zone) (both high and low side in low) for each pwm cycle [0,1]

    PhaseState phase_state[3]; //!< phase state (active / disabled)

    /**
     * Set phase voltages to the harware.
     *
     * @param Ua - phase A voltage
     * @param Ub - phase B voltage
     * @param Uc - phase C voltage
     */
    void setPwm(float Ua, float Ub, float Uc) override;

    /**
     * @brief PWM generating function.
     *
     */
    void halPwmWrite();

    /**
     * @brief Set phase voltages to the harware.
     *
     * @param sa phase A state : active / disabled ( high impedance )
     * @param sb phase B state : active / disabled ( high impedance )
     * @param sc phase C state : active / disabled ( high impedance )
     */
    virtual void setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc) override;

private:
    mcpwm_gen_handle_t generator[3][2] = {};
    mcpwm_cmpr_handle_t comparator[3];
    mcpwm_oper_handle_t oper[3];
    mcpwm_timer_handle_t timer = NULL;
    uint32_t mcpwm_period;
    int mcpwm_group;
};