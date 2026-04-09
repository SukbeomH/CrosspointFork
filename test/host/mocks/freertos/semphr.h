#pragma once
// ---------------------------------------------------------------
// FreeRTOS semaphore stubs for host-based unit tests.
// ---------------------------------------------------------------

#include <cstdint>

using SemaphoreHandle_t = void*;
using BaseType_t = int;
using TickType_t = uint32_t;

#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFFUL

inline SemaphoreHandle_t xSemaphoreCreateMutex() { return reinterpret_cast<void*>(1); }
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t /*s*/, TickType_t /*t*/) { return pdTRUE; }
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t /*s*/) { return pdTRUE; }
inline void vSemaphoreDelete(SemaphoreHandle_t /*s*/) {}
