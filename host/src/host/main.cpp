#include <iostream>

#include <httplib.h>

#include "host/networking/tcp_server.hpp"
#include "host/logging/logger.hpp"

#include "common/messages/init_esp.hpp"

void on_esp_init(common::init_esp_t init_esp) {
    LOG_INFO("Received from esp: {}", init_esp.id);
}

void on_connect(common::esp_id_t id) {
    LOG_INFO("ESP: {} connected", id);
}

void on_disconnect(common::esp_id_t id) {
    LOG_INFO("ESP: {} disconnected", id);
}

int main() {
    host::logger_t::init();
    host::tcp_server_t tcp_server;

    tcp_server.register_receive_callback<common::init_esp_t, &on_esp_init>();
    tcp_server.register_on_connect(on_connect);
    tcp_server.register_on_disconnect(on_disconnect);
    tcp_server.start();

    if (tcp_server.toggle_accepting(true) == host::tcp_status_t::fail_to_accept) {
        LOG_INFO("tcp_server not able to accept");

        return -1;
    }

    httplib::Server http_server;

    http_server.Get("/api/esp/", [&](const httplib::Request& req, httplib::Response& res) {
        auto status = tcp_server.send_to_client(0, common::motor_control_t{
            .ramp_time_ms = 7500,
            .target_speed = 700
        });
    });

    http_server.listen("0.0.0.0", 8081);
}