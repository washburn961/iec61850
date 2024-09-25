#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "goose.h"

// Sample data from the provided GOOSE frame
void test_goose_encode() {
    // Initialize the source and destination MAC addresses
    uint8_t source[MAC_ADDRESS_SIZE] = { 0x00, 0xa7, 0x03, 0xc1, 0x53, 0xb8 };
    uint8_t destination[MAC_ADDRESS_SIZE] = { 0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01 };

    // Initialize the appID
    uint8_t app_id[APP_ID_SIZE] = { 0x00, 0x05 };

    // Initialize the GOOSE handle
    goose_handle* handle = goose_init(source, destination, app_id);

    // Set the goosePdu fields according to the screenshot
    const char* gocbRef = "SE01100CS01CFG/LLN0$GO$AA3SE01100CS01G5";
    const char* dataset = "SE01100CS01CFG/LLN0$AA3SE01100CS01G5";
    const char* go_id = "AA3AL20101$SE01100CS01G5";
    uint32_t time_allowed_to_live = 2000;
    uint64_t t = 1695149275408396764;  // Time in the format from screenshot (UNIX epoch in nanoseconds)
    uint32_t st_num = 1;
    uint32_t sq_num = 162924;
    uint8_t simulation = 0;
    uint8_t conf_rev = 1;
    uint8_t nds_com = 0;
    uint16_t num_dataset_entries = 4;

    // Add these fields to the GOOSE PDU
    goose_all_data_entry_add(handle, TAG_GOCBREF, strlen(gocbRef), (uint8_t*)gocbRef);
    goose_all_data_entry_add(handle, TAG_DATASET, strlen(dataset), (uint8_t*)dataset);
    goose_all_data_entry_add(handle, TAG_GO_ID, strlen(go_id), (uint8_t*)go_id);
    goose_all_data_entry_add(handle, TAG_TIME_ALLOWED_TO_LIVE, sizeof(time_allowed_to_live), (uint8_t*)&time_allowed_to_live);
    goose_all_data_entry_add(handle, TAG_T, sizeof(t), (uint8_t*)&t);
    goose_all_data_entry_add(handle, TAG_ST_NUM, sizeof(st_num), (uint8_t*)&st_num);
    goose_all_data_entry_add(handle, TAG_SQ_NUM, sizeof(sq_num), (uint8_t*)&sq_num);
    goose_all_data_entry_add(handle, TAG_SIMULATION, sizeof(simulation), &simulation);
    goose_all_data_entry_add(handle, TAG_CONF_REV, sizeof(conf_rev), &conf_rev);
    goose_all_data_entry_add(handle, TAG_NDS_COM, sizeof(nds_com), &nds_com);
    goose_all_data_entry_add(handle, TAG_NUM_DATASET_ENTRIES, sizeof(num_dataset_entries), (uint8_t*)&num_dataset_entries);

    // Add the dataset entries
    uint8_t boolean_false = 0;
    for (int i = 0; i < 4; i++) {
        goose_all_data_entry_add(handle, TAG_ALL_DATA, sizeof(boolean_false), &boolean_false);
    }

    // Encode the frame
    goose_encode(handle);

    // Print the encoded byte stream
    printf("Encoded GOOSE frame (length: %zu bytes):\n", handle->length);
    for (size_t i = 0; i < handle->length; i++) {
        printf("%02x ", handle->byte_stream[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    // Clean up
    goose_free(handle);
}

int main() {
    test_goose_encode();
    return 0;
}
