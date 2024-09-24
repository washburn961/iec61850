#pragma once

#include <stdint.h>

typedef struct
{
	uint8_t tag;
	uint8_t* length;
	uint8_t* value;
} ber;