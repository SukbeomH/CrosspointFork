#pragma once
// Minimal oflag_t definitions for host mock
#include <cstdint>

using oflag_t = uint8_t;

#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_RDWR 0x02
#define O_CREAT 0x10
#define O_TRUNC 0x20
