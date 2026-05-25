#pragma once

#include "common/core/serialise.hpp"

namespace common {
    struct motor_speed_t {
        uint16_t ramp_time_ms = 0;
        uint32_t target_duty = 0;

        static void serialise(std::span<uint8_t>& payload, const motor_speed_t& data) {
            serialise_t::write(payload, data.ramp_time_ms);
            serialise_t::write(payload, data.target_duty);
        }

        static motor_speed_t deserialise(std::span<uint8_t> payload) {
            motor_speed_t result;

            result.ramp_time_ms = serialise_t::read<uint16_t>(payload); 
            result.target_duty = serialise_t::read<uint32_t>(payload); 

            return result;
        }

        static constexpr size_t payload_size() {
            return serialise_t::message_size<uint16_t, uint32_t>();
        }
    };

    struct motor_stop_t {
        EMPTY_MESSAGE(motor_stop_t)
    };
}