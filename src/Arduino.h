#pragma once

// Minimal stub so JTEncode.h will compile on desktop
#include <cstdint>

// On AVR these expand to attributes, so here we just make them no-ops:
#define PROGMEM
#define PGM_P const char*
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))

