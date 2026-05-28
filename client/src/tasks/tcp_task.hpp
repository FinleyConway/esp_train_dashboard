#pragma once

#include <freertos/FreeRTOS.h>

#include "common/messages/motor_control.hpp"
#include "task_events/motor_command.hpp"
#include "networking/tcp_client.hpp"

// https://docs.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/05-Implementing-a-task

namespace client {
    class tcp_task_t {
    public:
        static void init(UBaseType_t priority_offset) {
            xTaskCreate(
                run,
                "tcp_task_t",
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
            tcp_client_t client;

            register_messages(client);

            while (true) {
                try_connect(client);

                while (client.is_connected()) {
                    tcp_status_t status = client.listen_to_server();

                    if (status == tcp_status_t::unknown_packet) {
                        // assert, message wasnt registerd
                        // I should also really add logs for more context with errno.....
                        continue;
                    }
                    
                    if (status == tcp_status_t::failure) {
                        break;
                    }
                }

                client.disconnect();
            }

            on_client_disconnect();

            vTaskDelete(nullptr);
        }

        static void register_messages(tcp_client_t& client) {
            client.register_receieve_callback<common::motor_control_t, &on_motor_control>();
        }

        static void try_connect(tcp_client_t& client) {
            // TODO: Add retry attempts

            constexpr TickType_t retry_delay = pdMS_TO_TICKS(5000);
            
            while (client.try_connect() != tcp_status_t::success) {
                vTaskDelay(retry_delay);
            }
        }

        static void on_client_disconnect() {
            motor_command_t::send(common::motor_control_t {
                .starting_duty = 0,
                .target_duty = 0,
                .ramp_time_ms = 0,
                .is_active = false
            });
        }

    private:
        static void on_motor_control(const common::motor_control_t& motor_control) {
            motor_command_t::send(motor_control);
        }

    private:
        inline static TaskHandle_t s_handle = nullptr;
        inline static uint32_t s_stack_size = 8192;
    };        
}