#include <driver/gpio.h>
#include <nvs_flash.h>

#include "networking/wifi.hpp"
#include "networking/tcp_client.hpp"

#include "motor.hpp"

#include <cmath>

client::motor_t motor;

static float ease(float x) {
    return -(std::cos(M_PI * x) - 1) / 2;
}

static void adjust_motor_speed(uint16_t current_speed, uint16_t target_duty, uint16_t ramp_speed_ms) {
    float a = static_cast<float>(current_speed);
    float b = static_cast<float>(target_duty);

    const float loop_ms = 50.0f;
    const float time_step = loop_ms / static_cast<float>(ramp_speed_ms);
    float time = 0.0f;

    motor.set_motor_direction(client::motor_direction_t::clockwise);

    while (time < 1.0f) {
        float time_ease = ease(time);
        float value = std::lerp(a, b, std::clamp(time_ease, 0.0f, 1.0f));

        motor.set_motor_speed(static_cast<uint32_t>(value));

        time += time_step;

        vTaskDelay(pdMS_TO_TICKS(loop_ms));
    }

    motor.set_motor_speed(target_duty);
}

void on_motor_speed_change(const common::motor_speed_t& motor_control) {
    motor.set_active_state(true);

    adjust_motor_speed(
        motor.get_current_duty(),
        motor_control.target_duty,
        motor_control.ramp_time_ms
    );
}

void on_motor_stop(common::motor_stop_t) {
    motor.set_active_state(false);
}

extern "C" void app_main() {
    nvs_flash_init();

    motor.init(GPIO_NUM_23, GPIO_NUM_22);

    client::wifi_status_t status = client::wifi_boot_t::connect();

    if (status == client::wifi_status_t::failure) {
        ESP_LOGI("ESP_MAIN", "Failed to associate to AP, dying...");

        return;
    }

    client::tcp_client_t c;
    
    c.register_receieve_callback<common::motor_speed_t, &on_motor_speed_change>();
    c.register_receieve_callback<common::motor_stop_t, &on_motor_stop>();

    if (c.try_connect() == client::tcp_status_t::success) { 

        ESP_LOGI("ESP_MAIN", "Connected");

        while (c.is_connected()) {
            vTaskDelay(pdMS_TO_TICKS(10));

            auto status = c.listen_to_server();

            if (status == client::tcp_status_t::success) {
                ESP_LOGI("ESP_MAIN", "Received!");
            }

            if (status == client::tcp_status_t::unknown_packet) {
                ESP_LOGI("ESP_MAIN", "Should probably assert this?");
            }

            if (status == client::tcp_status_t::failure) {
                ESP_LOGI("ESP_MAIN", "Fail");
            }
        }
    }
}