#include "ber.h"
#include <string.h>

void ber_init(ber* obj, uint8_t tag)
{
    if (!obj) return;

    obj->tag = tag;
    obj->length = 0x0;
    obj->value = NULL;
}

static inline size_t parse_length(uint8_t* all_bytes, size_t len, size_t* length_bytes_count)
{
    if (len == 0) {
        return 0; // Invalid input, no bytes available
    }

    size_t length = 0;
    all_bytes++;

    if (all_bytes[0] & 0x80) { // Long form (first bit is 1)
        // Get the number of bytes used for encoding the length
        uint8_t numLengthBytes = all_bytes[0] & 0x7F;

        if (numLengthBytes > sizeof(uint32_t) || numLengthBytes >= len) {
            return 0; // Invalid length field or too large to fit in uint32_t
        }

        // Decode the long form length
        for (size_t i = 0; i < numLengthBytes; i++) {
            length = (length << 8) | all_bytes[i + 1];  // Shift and add next byte
        }

        if (length_bytes_count != NULL)
        {
            *length_bytes_count = numLengthBytes + 1;
        }
    }
    else { // Short form (first bit is 0)
        length = all_bytes[0] & 0x7F;  // Extract length from remaining 7 bits

        if (length_bytes_count != NULL)
        {
            *length_bytes_count = 1;
        }
    }

    return length;
}

static inline size_t encode_length(size_t len, uint8_t** out_length_bytes)
{
    if (len >= 0 && len <= 127)
    {
        // Short form: 1 byte for lengths 0 to 127
        *out_length_bytes = (uint8_t*)malloc(1);
        if (!*out_length_bytes) return 0;
        **out_length_bytes = len;  // Directly assign the length
        return 1;
    }
    else
    {
        // Long form: allocate enough bytes for the length
        size_t num_bytes = 0;
        size_t tmp_len = len;

        // Determine how many bytes are needed to represent the length
        while (tmp_len > 0)
        {
            tmp_len >>= 8;  // Shift by 8 bits (1 byte) to count required bytes
            num_bytes++;
        }

        // Allocate memory: 1 byte for the header + num_bytes for the actual length
        *out_length_bytes = (uint8_t*)malloc(1 + num_bytes);
        if (!*out_length_bytes) return 0;

        // Set the first byte to indicate long form and the number of length bytes
        **out_length_bytes = 0x80 | num_bytes;  // 0x80 means "long form" and OR with num_bytes

        // Encode the length into the following bytes
        for (size_t i = 0; i < num_bytes; i++)
        {
            (*out_length_bytes)[1 + num_bytes - 1 - i] = (uint8_t)(len & 0xFF);  // Store the last byte of len
            len >>= 8;  // Shift to the next byte
        }

        return 1 + num_bytes;  // Total bytes used: 1 header byte + num_bytes for the length
    }
}



ber* ber_decode(uint8_t* bytes, size_t len)
{
    if (len < 2) return NULL;

    ber* element = (ber*)malloc(sizeof(ber));
    if (!element) return NULL;

    element->tag = bytes[0];

    element->length = parse_length(bytes, len, NULL);
    if (element->length == 0)
    {
        element->value = NULL;
        return element;
    }


    element->value = (uint8_t*)malloc(element->length);

    if (!element->value)
    {
        free(element);
        return NULL;
    }

    memcpy(element->value, &bytes[len - element->length], element->length);

    return element;
}

size_t ber_encode(ber* obj, uint8_t** out_bytes)
{
    if (*out_bytes != NULL)
    {
        free(*out_bytes);  // Free the previously allocated memory
        *out_bytes = NULL; // Reset the pointer to avoid dangling references
    }

    uint8_t* length_bytes;
    size_t length_bytes_len = encode_length(obj->length, &length_bytes);
    size_t total_length = 1 + length_bytes_len + obj->length;

    *out_bytes = (uint8_t*)malloc(total_length);
    if (!*out_bytes)
    {
        free(length_bytes);
        return 0;
    }

    (*out_bytes)[0] = obj->tag;
    memcpy(&(*out_bytes)[1], length_bytes, length_bytes_len);
    memcpy(&(*out_bytes)[1 + length_bytes_len], obj->value, obj->length);

    free(length_bytes);
    return total_length;
}

// Function to decode multiple BER objects from a byte stream
ber* ber_decode_many(uint8_t* bytes, size_t len, size_t count) {
    size_t offset = 0;
    ber* decoded_objects = (ber*)malloc(count * sizeof(ber));
    if (!decoded_objects) {
        return NULL; // Memory allocation failed
    }

    for (size_t i = 0; i < count; i++) {
        if (offset >= len) {
            // Free any previously allocated objects on failure
            for (size_t j = 0; j < i; j++) {
                free(decoded_objects[j].value);
            }
            free(decoded_objects);
            return NULL;
        }

        // Read the tag
        uint8_t tag = bytes[offset];
        offset += 1;

        // Parse the length field using the parse_length function
        size_t length_bytes_count = 0;
        size_t length = parse_length(bytes + offset - 1, len - offset + 1, &length_bytes_count);
        if (length == 0 && length_bytes_count == 0) {
            // Free any previously allocated objects on failure
            for (size_t j = 0; j < i; j++) {
                free(decoded_objects[j].value);
            }
            free(decoded_objects);
            return NULL;
        }

        // Move past the length field
        offset += length_bytes_count;

        // Decode the value
        uint8_t* value = (uint8_t*)malloc(length);
        if (!value) {
            // Free previously allocated memory
            for (size_t j = 0; j < i; j++) {
                free(decoded_objects[j].value);
            }
            free(decoded_objects);
            return NULL;
        }
        memcpy(value, bytes + offset, length);
        offset += length;  // Move past the value field

        // Store the decoded BER object
        decoded_objects[i].tag = tag;
        decoded_objects[i].length = length;
        decoded_objects[i].value = value;
    }

    return decoded_objects;  // Return the array of decoded objects
}


// Function to encode multiple BER objects into a single byte stream
size_t ber_encode_many(ber* obj_array, size_t count, uint8_t** out_bytes) {
    size_t total_length = 0;

    // First, calculate the total length required for the output byte array
    for (size_t i = 0; i < count; i++) {
        uint8_t* temp_encoded = NULL;
        size_t temp_len = ber_encode(&obj_array[i], &temp_encoded);
        if (temp_len == 0) {
            free(temp_encoded); // Free here in case of failure
            return 0;  // Encoding failed
        }
        total_length += temp_len;
        free(temp_encoded); // Only free once after calculating total length
    }

    // Allocate memory for the output byte array
    *out_bytes = (uint8_t*)malloc(total_length);
    if (!*out_bytes) return 0;

    // Now encode each BER object into the output array
    size_t offset = 0;
    for (size_t i = 0; i < count; i++) {
        uint8_t* temp_encoded = NULL;
        size_t temp_len = ber_encode(&obj_array[i], &temp_encoded);
        if (temp_len == 0) {
            free(temp_encoded);
            free(*out_bytes);  // Clean up the entire output buffer on failure
            return 0;  // Encoding failed
        }

        // Copy the encoded object into the output array
        memcpy(*out_bytes + offset, temp_encoded, temp_len);
        offset += temp_len;
        free(temp_encoded);  // Free the temporary buffer after use
    }

    return total_length;  // Return the total length of the encoded byte stream
}

void ber_free(ber* obj)
{
    if (obj)
    {
        if (obj->value)
        {
            free(obj->value);  // Free the value buffer
        }
        free(obj);  // Free the ber structure itself
    }
}

void ber_free_many(ber* obj, size_t count)
{
    if (obj)
    {
        for (size_t i = 0; i < count; i++)
        {
            if (obj[i].value)
            {
                free(obj[i].value);  // Free the value field for each `ber`
            }
        }
        free(obj);  // Free the array of `ber` structures
    }
}