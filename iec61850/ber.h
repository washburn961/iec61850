#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t tag;
    size_t length;
    uint8_t* value;
} ber;

ber* ber_decode(uint8_t* bytes, size_t len);
ber* ber_decode_many(uint8_t* bytes, size_t len, size_t count);
size_t ber_encode(ber* obj, uint8_t** out_bytes);
size_t ber_encode_many(ber* obj, size_t count, uint8_t** out_bytes);