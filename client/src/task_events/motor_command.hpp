#pragma once

#include <freertos/FreeRTOS.h>

#include "common/messages/motor_control.hpp"

namespace client {
    class motor_command_t {
    public:
        static void create() {
            if (m_event != nullptr) {
                // add log or assert

                return;
            }

            // creating a latest-value queue
            m_event = xQueueCreate(1, sizeof(common::motor_control_t));

            if (m_event == nullptr) {
                // add log
            }
        }

        static void send(const common::motor_control_t& motor_control, TickType_t block_ms = 10) {
            if (m_event == nullptr) return; // add log or assert

            xQueueOverwrite(m_event, &motor_control);
        }

        static bool receive(common::motor_control_t& motor_control, TickType_t timeout_ms = portMAX_DELAY) {
            if (m_event == nullptr) return false; // add log or assert

            return xQueueReceive(m_event, &motor_control, timeout_ms) == pdPASS;
        }

    private:
        inline static QueueHandle_t m_event = nullptr;
    };
}