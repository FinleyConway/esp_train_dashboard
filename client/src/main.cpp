#include <driver/gpio.h>
#include <nvs_flash.h>

#include "networking/wifi.hpp"
#include "networking/tcp_client.hpp"

#include "motor.hpp"

client::motor_t motor;

void on_motor_control(const common::motor_control_t& motor_control) {
    ESP_LOGI("MOTOR", "%d", motor_control.target_speed);
    ESP_LOGI("MOTOR", "%d", motor_control.ramp_time_ms);

    motor.set_motor_direction(client::motor_direction_t::clockwise);
    motor.set_motor_speed(motor_control.target_speed);
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
    
    c.register_receieve_callback<common::motor_control_t, &on_motor_control>();

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