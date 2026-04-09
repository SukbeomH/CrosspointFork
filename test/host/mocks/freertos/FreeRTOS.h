#pragma once
// ---------------------------------------------------------------
// FreeRTOS stubs for host-based unit tests.
// ---------------------------------------------------------------

#include <cstdint>

using BaseType_t = int;
using TickType_t = uint32_t;
using TaskHandle_t = void*;

#define pdTRUE 1
#define pdFALSE 0
#define configTICK_RATE_HZ 1000

inline void vTaskDelay(TickType_t /*ticks*/) {}
inline TickType_t xTaskGetTickCount() { return 0; }
