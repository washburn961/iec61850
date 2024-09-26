#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <crtdbg.h>
#include "goose.h"

// Sample data from the provided GOOSE frame
void test_goose_encode() {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Initialize the source and destination MAC addresses
    uint8_t source[MAC_ADDRESS_SIZE] = { 0x00, 0x30, 0xa7, 0x03, 0xc1, 0x53 };
    uint8_t destination[MAC_ADDRESS_SIZE] = { 0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01 };

    // Initialize the appID
    uint8_t app_id[APP_ID_SIZE] = { 0x00, 0x05 };

    // Initialize the GOOSE handle
    goose_handle* handle = goose_init(source, destination, app_id);

    // Set the goosePdu fields according to the screenshot
    const char* gocbRef = "CPC UNIFEI/LLN0$GO$TestDataSet";
    const char* dataset = "CPC UNIFEI/LLN0$TestDataSet";
    const char* go_id = "CPC UNIFEI GOID";

    uint16_t time_allowed_to_live = goose_htons(2000);
    uint64_t t = goose_htonll(1695149275408396764);  // Time in the format from screenshot (UNIX epoch in nanoseconds)
    uint32_t st_num = goose_htonl(1);
    uint32_t sq_num = goose_htonl(162924);
    uint8_t simulation = 0;
    uint8_t conf_rev = 1;
    uint8_t nds_com = 0;

    // Set values using ber_set for each PDU field
    ber_set(&(handle->frame->pdu_list.gocbref), (uint8_t*)gocbRef, strlen(gocbRef));
    ber_set(&(handle->frame->pdu_list.dataset), (uint8_t*)dataset, strlen(dataset));
    ber_set(&(handle->frame->pdu_list.go_id), (uint8_t*)go_id, strlen(go_id));
    ber_set(&(handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live, sizeof(time_allowed_to_live));
    ber_set(&(handle->frame->pdu_list.t), (uint8_t*)&t, sizeof(t));
    ber_set(&(handle->frame->pdu_list.st_num), (uint8_t*)&st_num, sizeof(st_num));
    ber_set(&(handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num, sizeof(sq_num));
    ber_set(&(handle->frame->pdu_list.simulation), (uint8_t*)&simulation, sizeof(simulation));
    ber_set(&(handle->frame->pdu_list.conf_rev), (uint8_t*)&conf_rev, sizeof(conf_rev));
    ber_set(&(handle->frame->pdu_list.nds_com), (uint8_t*)&nds_com, sizeof(nds_com));
    //ber_set(&(handle->frame->pdu_list.num_dataset_entries), (uint8_t*)&num_dataset_entries, sizeof(num_dataset_entries));

    // Add the dataset entries
    uint8_t boolean_false = 0;
    for (int i = 0; i < 4; i++) {
        goose_all_data_entry_add(handle, 0x83, sizeof(boolean_false), &boolean_false);
    }

    for (size_t i = 0; i < 1000; i++)
    {
        goose_encode(handle);
    }

    // Print the encoded byte stream
    printf("Encoded GOOSE frame (length: %zu bytes):\n", handle->length);
    for (size_t i = 0; i < handle->length; i++) {
        printf("%02x ", handle->byte_stream[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    // Clean up
    goose_free(handle);

    _CrtDumpMemoryLeaks();
}

int main() {
    //_CrtSetBreakAlloc(69007);
    //_CrtSetBreakAlloc(68973);
    test_goose_encode();
    return 0;
}
