#include <stdio.h>
#include <string.h>
#include "ber.h"

// Helper function to compare two ber objects
int compare_ber(ber* a, ber* b) {
    if (a->tag != b->tag) {
        printf("Tags don't match: 0x%x vs 0x%x\n", a->tag, b->tag);
        return 0;
    }
    if (a->length != b->length) {
        printf("Lengths don't match: %zu vs %zu\n", a->length, b->length);
        return 0;
    }
    if (a->length > 0) {
        printf("Comparing values for objects with tag 0x%x and length %zu\n", a->tag, a->length);
        for (size_t i = 0; i < a->length; i++) {
            if (a->value[i] != b->value[i]) {
                printf("Value mismatch at index %zu: 0x%x vs 0x%x\n", i, a->value[i], b->value[i]);
                return 0;
            }
        }
    }
    return 1;
}

void test_decode_many() {
    // Create a byte stream that contains multiple BER objects
    uint8_t stream[] = {
        0x01, 0x03, 0x10, 0x20, 0x30,  // First BER object (tag = 0x01, length = 3, value = {0x10, 0x20, 0x30})
        0x02, 0x02, 0x40, 0x50         // Second BER object (tag = 0x02, length = 2, value = {0x40, 0x50})
    };
    size_t len = sizeof(stream);
    size_t count = 2;  // We know there are 2 BER objects in the stream

    // Decode the byte stream
    ber* decoded = ber_decode_many(stream, len, count);

    if (!decoded) {
        printf("Test Failed: Decoding multiple BER objects failed.\n");
        return;
    }

    // Verify the decoded BER objects
    for (size_t i = 0; i < count; i++) {
        printf("Decoded BER object %zu: tag = 0x%x, length = %zu\n", i, decoded[i].tag, decoded[i].length);
    }

    // Clean up
    for (size_t i = 0; i < count; i++) {
        if (decoded[i].value) {
            free(decoded[i].value);
        }
    }
    free(decoded);
}


// Test case 1: Basic encoding and decoding
void test_encode_decode() {
    ber original;
    original.tag = 0x01;
    original.length = 3;
    uint8_t value[] = { 0x10, 0x20, 0x30 };
    original.value = value;

    // Encode the original object
    uint8_t* encoded_bytes = NULL;
    size_t encoded_len = ber_encode(&original, &encoded_bytes);

    if (encoded_len == 0 || !encoded_bytes) {
        printf("Test Failed: Encoding failed.\n");
        return;
    }

    // Decode the encoded bytes
    ber* decoded = ber_decode(encoded_bytes, encoded_len);
    free(encoded_bytes);  // Don't forget to free the encoded bytes after use

    if (!decoded) {
        printf("Test Failed: Decoding failed.\n");
        return;
    }

    // Compare the original and decoded objects
    if (compare_ber(&original, decoded)) {
        printf("Test Passed: Encoding and decoding are consistent.\n");
    }
    else {
        printf("Test Failed: Original and decoded objects do not match.\n");
    }

    // Clean up the decoded object
    if (decoded->value) {
        free(decoded->value);
    }
    free(decoded);
}

// Test case 2: Zero-length value
void test_zero_length_value() {
    ber original;
    original.tag = 0x02;
    original.length = 0;
    original.value = NULL;  // No value

    // Encode the original object
    uint8_t* encoded_bytes = NULL;
    size_t encoded_len = ber_encode(&original, &encoded_bytes);

    if (encoded_len == 0 || !encoded_bytes) {
        printf("Test Failed: Encoding failed (zero-length value).\n");
        return;
    }

    // Decode the encoded bytes
    ber* decoded = ber_decode(encoded_bytes, encoded_len);
    free(encoded_bytes);  // Free encoded bytes after use

    if (!decoded) {
        printf("Test Failed: Decoding failed (zero-length value).\n");
        return;
    }

    // Compare the original and decoded objects
    if (compare_ber(&original, decoded)) {
        printf("Test Passed: Zero-length value test passed.\n");
    }
    else {
        printf("Test Failed: Zero-length value test failed.\n");
    }

    // Clean up
    if (decoded->value) {
        free(decoded->value);
    }
    free(decoded);
}

// Test case 3: Long value
void test_long_value() {
    ber original;
    original.tag = 0x03;
    original.length = 256;  // A long value that requires multiple bytes for length
    original.value = (uint8_t*)malloc(original.length);
    if (!original.value) {
        printf("Test Failed: Could not allocate memory for original value.\n");
        return;
    }

    // Fill the value with incremental bytes
    for (size_t i = 0; i < original.length; i++) {
        original.value[i] = (uint8_t)(i & 0xFF);
    }

    // Encode the original object
    uint8_t* encoded_bytes = NULL;
    size_t encoded_len = ber_encode(&original, &encoded_bytes);

    if (encoded_len == 0 || !encoded_bytes) {
        printf("Test Failed: Encoding failed (long value).\n");
        free(original.value);
        return;
    }

    // Decode the encoded bytes
    ber* decoded = ber_decode(encoded_bytes, encoded_len);
    free(encoded_bytes);  // Free encoded bytes after use

    if (!decoded) {
        printf("Test Failed: Decoding failed (long value).\n");
        free(original.value);
        return;
    }

    // Compare the original and decoded objects
    if (compare_ber(&original, decoded)) {
        printf("Test Passed: Long value test passed.\n");
    }
    else {
        printf("Test Failed: Long value test failed.\n");
    }

    // Clean up
    if (decoded->value) {
        free(decoded->value);
    }
    free(decoded);
    free(original.value);
}

int main() {
    // Run all tests
    printf("Running test_encode_decode...\n");
    test_encode_decode();

    printf("Running test_zero_length_value...\n");
    test_zero_length_value();

    printf("Running test_long_value...\n");
    test_long_value();

    printf("Running test_decode_many...\n");
    test_decode_many();

    return 0;
}
