#pragma once

#include "common/core/serialise.hpp"

// should put this in serialise
template <typename... Ts>
constexpr size_t message_size() {
    return (sizeof(Ts) + ...);
}

namespace common {
    struct motor_control_t {
        uint16_t ramp_time_ms = 0;
        uint16_t target_speed = 0;

        static void serialise(std::span<uint8_t>& payload, const motor_control_t& data) {
            serialise_t::write(payload, data.ramp_time_ms);
            serialise_t::write(payload, data.target_speed);
        }

        static motor_control_t deserialise(std::span<uint8_t> payload) {
            motor_control_t result;

            result.ramp_time_ms = serialise_t::read<uint16_t>(payload); 
            result.target_speed = serialise_t::read<uint16_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return message_size<uint16_t, uint16_t>();
        }
    };
}