#pragma once

#include <stdint.h>

typedef struct {
	uint32_t seconds;
	uint32_t nanoseconds;
} iec_time;