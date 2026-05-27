#pragma once

#include <cmath>

#include <freertos/FreeRTOS.h>

#include "common/messages/motor_control.hpp"
#include "task_events/motor_command.hpp"
#include "components/motor.hpp"

namespace client {
    class motor_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            xTaskCreate(
                run,
                "motor_task_t",
                s_stack_size,
                nullptr,
                tskIDLE_PRIORITY + priority_offset,
                &s_handle
            );
        }
        
        static TaskHandle_t get_handle() {
            return s_handle;
        }

    private:
        static void run(void* parameters) {
            motor_t motor;
            motor.init(GPIO_NUM_23, GPIO_NUM_22);

            while (true) {
                // block thread and wait for motor_control commands
                common::motor_control_t motor_control;
                motor_command_t::receive(motor_control); 

                // do motor stuff
                motor.set_active_state(motor_control.is_active);

                if (motor_control.is_active) {
                    adjust_motor(motor, motor_control);
                }
            }

            vTaskDelete(nullptr);
        }

        static void adjust_motor(motor_t& motor, const common::motor_control_t& motor_control) {
            float start = static_cast<float>(motor_control.starting_duty);
            float end = static_cast<float>(motor_control.target_duty);

            const float loop_ms = 50.0f;
            const float time_step = loop_ms / static_cast<float>(motor_control.ramp_time_ms);
            float time = 0.0f;

            motor.set_motor_direction(client::motor_direction_t::clockwise);

            while (time < 1.0f) {
                float time_ease = ease(time);
                float value = std::lerp(start, end, std::clamp(time_ease, 0.0f, 1.0f));

                motor.set_motor_duty(static_cast<uint32_t>(value));

                time += time_step;

                vTaskDelay(pdMS_TO_TICKS(loop_ms));
            }

            motor.set_motor_duty(motor_control.target_duty);
        }

        static float ease(float x) {
            return -(std::cos(M_PI * x) - 1) / 2;
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 2048;
    };
} 
