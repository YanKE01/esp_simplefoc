/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tuple>
#include <vector>
#include "esp_hal_bldc_6pwm.h"

/**
 * @brief First is mcpwm group id, second is whether it has been used.
 *
 */
static std::vector<std::tuple<int, int>> HardwareResource = {
    std::make_tuple(0, 0),
    std::make_tuple(1, 0),
};

static int auto_mcpwm_group = -1;

typedef struct mcpwm_ctx
{
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_cmpr_handle_t comparator[3];
    mcpwm_oper_handle_t oper[3];
    mcpwm_gen_handle_t generator[3][2];
} mcpwm_ctx_t;

mcpwm_ctx_t mcpwm_dev;

void hal_bldc_6pwm_init()
{
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = _PWM_TIMEBASE_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = (uint32_t)(1 * _PWM_TIMEBASE_RESOLUTION_HZ / _PWM_FREQUENCY),
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &mcpwm_dev.timer));

    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &mcpwm_dev.oper[i]));
    }

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(mcpwm_dev.oper[i], mcpwm_dev.timer));
    }

    mcpwm_comparator_config_t comparator_config;
    comparator_config.flags.update_cmp_on_tez = true;
    comparator_config.flags.update_cmp_on_tep = false;
    comparator_config.flags.update_cmp_on_sync = false;

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_comparator(mcpwm_dev.oper[i], &comparator_config, &mcpwm_dev.comparator[i]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(mcpwm_dev.comparator[i], (0)));
    }

    mcpwm_generator_config_t generator_config = {};

    int gen_gpios[3][2] = {{11, 12}, {13, 14}, {15, 16}}; // three gpio port

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            generator_config.gen_gpio_num = gen_gpios[i][j];
            ESP_ERROR_CHECK(mcpwm_new_generator(mcpwm_dev.oper[i], &generator_config, &mcpwm_dev.generator[i][j]));
        }
    }

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(mcpwm_dev.generator[i][0],
                                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
        // go low on compare threshold
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(mcpwm_dev.generator[i][0],
                                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, mcpwm_dev.comparator[i], MCPWM_GEN_ACTION_LOW)));
    }

    mcpwm_dead_time_config_t dead_time_config;
    dead_time_config.negedge_delay_ticks = 5;
    dead_time_config.flags.invert_output = false;

    // set complementary PWM
    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(mcpwm_dev.generator[i][0], mcpwm_dev.generator[i][1], &dead_time_config));
    }

    ESP_ERROR_CHECK(mcpwm_timer_enable(mcpwm_dev.timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(mcpwm_dev.timer, MCPWM_TIMER_START_NO_STOP));
}

void hal_bldc_6pwm_write()
{
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(mcpwm_dev.comparator[0], 250));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(mcpwm_dev.comparator[1], 250));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(mcpwm_dev.comparator[2], 250));
}

static void setMcpwmGroupUsed(int group_id)
{
    for (auto &vp : HardwareResource)
    {
        int mcpwm_group = std::get<0>(vp);
        if (mcpwm_group == group_id)
        {
            vp = {mcpwm_group, 1};
        }
    }
}

static void setMcpwmGroupUnUsed(int group_id)
{
    for (auto &vp : HardwareResource)
    {
        int mcpwm_group = std::get<0>(vp);
        if (mcpwm_group == group_id)
        {
            vp = {mcpwm_group, 0};
        }
    }
}

static int checkAvailableDriver()
{
    auto_mcpwm_group = -1;

    for (auto &vp : HardwareResource)
    {
        if (std::get<1>(vp) == 0)
        {
            auto_mcpwm_group = std::get<0>(vp);
        }
    }

    if (auto_mcpwm_group != -1)
    {
        printf("MCPWM Group: %d is idle\n", auto_mcpwm_group);
        return 1;
    }

    printf("no available mcpwm driver\n");
    return 0;
}

BLDCDriver6PWM::BLDCDriver6PWM(int phA_h, int phA_l, int phB_h, int phB_l, int phC_h, int phC_l, int en)
{
    // Pin initialization
    pwmA_h = phA_h;
    pwmB_h = phB_h;
    pwmC_h = phC_h;
    pwmA_l = phA_l;
    pwmB_l = phB_l;
    pwmC_l = phC_l;

    // enable_pin pin
    enable_pin = en;

    // default power-supply value
    voltage_power_supply = DEF_POWER_SUPPLY;
    voltage_limit = NOT_SET;
    pwm_frequency = NOT_SET;

    // dead zone initial - 2%
    dead_zone = 0.02f;
}

int BLDCDriver6PWM::init()
{
    int ret = -1;
    if (_isset(enable_pin))
    {
        pinMode(enable_pin, OUTPUT);
    }

    // sanity check for the voltage limit configuration
    if (!_isset(voltage_limit) || voltage_limit > voltage_power_supply)
        voltage_limit = voltage_power_supply;

    // set phase state to disabled
    phase_state[0] = PhaseState::PHASE_OFF;
    phase_state[1] = PhaseState::PHASE_OFF;
    phase_state[2] = PhaseState::PHASE_OFF;

    // set zero to PWM
    dc_a = dc_b = dc_c = 0;

    // ESP32 Platform specific function. Auto select driver.
    ret = checkAvailableDriver();
    if (ret == 0)
    {
        initialized = 0;
        printf("No available Driver.\n");
        return 0;
    }

    setMcpwmGroupUsed(auto_mcpwm_group);
    mcpwm_group = auto_mcpwm_group;
    printf("Auto. Current Driver uses Mcpwm GroupId:%d\n", mcpwm_group);

    mcpwm_timer_config_t timer_config = {
        .group_id = mcpwm_group,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = _PWM_TIMEBASE_RESOLUTION_HZ,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = (uint32_t)(1 * _PWM_TIMEBASE_RESOLUTION_HZ / _PWM_FREQUENCY),
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_operator_config_t operator_config = {
        .group_id = mcpwm_group, // operator must be in the same group to the timer
    };

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper[i]));
    }

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper[i], timer));
    }

    mcpwm_comparator_config_t comparator_config;
    comparator_config.flags.update_cmp_on_tez = true;
    comparator_config.flags.update_cmp_on_tep = false;
    comparator_config.flags.update_cmp_on_sync = false;

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_comparator(oper[i], &comparator_config, &comparator[i]));
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator[i], (0)));
    }

    mcpwm_generator_config_t generator_config = {};
    int gen_gpios[3][2] = {{pwmA_h, pwmA_l}, {pwmB_h, pwmB_l}, {pwmC_h, pwmC_l}};

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            generator_config.gen_gpio_num = gen_gpios[i][j];
            ESP_ERROR_CHECK(mcpwm_new_generator(oper[i], &generator_config, &generator[i][j]));
        }
    }

    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator[i][0],
                                                                  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
        // go low on compare threshold
        ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator[i][0],
                                                                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator[i], MCPWM_GEN_ACTION_LOW)));
    }

    mcpwm_dead_time_config_t dead_time_config;
    dead_time_config.negedge_delay_ticks = 5;
    dead_time_config.posedge_delay_ticks = 0;
    dead_time_config.flags.invert_output = true;

    // set complementary PWM
    for (int i = 0; i < 3; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(generator[i][0], generator[i][1], &dead_time_config));
    }

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
    mcpwm_period = timer_config.period_ticks;
    initialized = 1;
    return 1;
}

int BLDCDriver6PWM::deinit()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (mcpwm_del_generator(generator[i][j]) != ESP_OK)
            {
                return 0;
            }
        }

        if (mcpwm_del_comparator(comparator[i]) != ESP_OK)
        {
            return 0;
        }

        if (mcpwm_del_operator(oper[i]) != ESP_OK)
        {
            return 0;
        }
    }

    if (mcpwm_timer_disable(timer) != ESP_OK)
    {
        return 0;
    }

    if (mcpwm_del_timer(timer) != ESP_OK)
    {
        return 0;
    }

    setMcpwmGroupUnUsed(mcpwm_group);

    initialized = 0;
    return 1;
}

void BLDCDriver6PWM::disable()
{
    // set phase state to disabled
    setPhaseState(PhaseState::PHASE_OFF, PhaseState::PHASE_OFF, PhaseState::PHASE_OFF);
    // set zero to PWM
    setPwm(0, 0, 0);
    // disable the driver - if enable_pin pin available
    if (_isset(enable_pin))
    {
        digitalWrite(enable_pin, !enable_active_high);
    }
}

void BLDCDriver6PWM::enable()
{
    // enable_pin the driver - if enable_pin pin available
    if (_isset(enable_pin))
    {
        digitalWrite(enable_pin, enable_active_high);
    }
    // set phase state enabled
    setPhaseState(PhaseState::PHASE_ON, PhaseState::PHASE_ON, PhaseState::PHASE_ON);
    // set zero to PWM
    setPwm(0, 0, 0);
}

void BLDCDriver6PWM::setPwm(float Ua, float Ub, float Uc)
{
    // limit the voltage in driver
    Ua = _constrain(Ua, 0, voltage_limit);
    Ub = _constrain(Ub, 0, voltage_limit);
    Uc = _constrain(Uc, 0, voltage_limit);
    // calculate duty cycle
    // limited in [0,1]
    dc_a = _constrain(Ua / voltage_power_supply, 0.0f, 1.0f);
    dc_b = _constrain(Ub / voltage_power_supply, 0.0f, 1.0f);
    dc_c = _constrain(Uc / voltage_power_supply, 0.0f, 1.0f);
    // hardware specific writing
    // hardware specific function - depending on driver and mcu
    halPwmWrite();
}

void BLDCDriver6PWM::halPwmWrite()
{
    if (!initialized)
    {
        return;
    }

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator[0], (uint32_t)((mcpwm_period * 0.5))));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator[1], (uint32_t)((mcpwm_period * 0.5))));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator[2], (uint32_t)((mcpwm_period * 0.5))));
}

void BLDCDriver6PWM::setPhaseState(PhaseState sa, PhaseState sb, PhaseState sc)
{
    phase_state[0] = sa;
    phase_state[1] = sb;
    phase_state[2] = sc;
}
