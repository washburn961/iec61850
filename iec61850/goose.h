#pragma once

#include <stdint.h>
#include "ber.h"

#define APP_ID_SIZE 2
#define RESERVED_SIZE 2
#define ETHERTYPE_SIZE 2
#define MAC_ADDRESS_SIZE 6
#define GOOSE_PDU_FIELD_COUNT 12

#define GOOSE_ETHERTYPE_0 0x88
#define GOOSE_ETHERTYPE_1 0xb8

#define TAG_PDU 0x61
#define TAG_GOCBREF 0x80
#define TAG_TIME_ALLOWED_TO_LIVE 0x81
#define TAG_DATASET 0x82
#define TAG_GO_ID 0x83
#define TAG_T 0x84
#define TAG_ST_NUM 0x85
#define TAG_SQ_NUM 0x86
#define TAG_SIMULATION 0x87
#define TAG_CONF_REV 0x88
#define TAG_NDS_COM 0x89
#define TAG_NUM_DATASET_ENTRIES 0x8a
#define TAG_ALL_DATA 0xab

#define GOOSE_MULTICAST_ADDRESS(last_byte0, last_byte1) { 0x01, 0x0C, 0xCD, 0x01, last_byte0, last_byte1 }

#define MAX_NUM_DATASET_ENTRIES 16

typedef struct
{
	ber entries[MAX_NUM_DATASET_ENTRIES];
	size_t entry_count;
} goose_all_data;

typedef struct
{
	ber gocbref;
	ber time_allowed_to_live;
	ber dataset;
	ber go_id;
	ber t;
	ber st_num;
	ber sq_num;
	ber simulation;
	ber conf_rev;
	ber nds_com;
	ber num_dataset_entries;
	ber all_data;
	goose_all_data all_data_list;
} goose_pdu;

typedef struct
{
	uint8_t destination[MAC_ADDRESS_SIZE];
	uint8_t source[MAC_ADDRESS_SIZE];
	uint8_t ethertype[ETHERTYPE_SIZE];
	uint8_t app_id[APP_ID_SIZE];
	uint16_t len;
	uint8_t reserved_1[RESERVED_SIZE];
	uint8_t reserved_2[RESERVED_SIZE];
	ber pdu;
	goose_pdu pdu_list;
} goose_frame;

typedef struct {
	goose_frame* frame;
	uint8_t byte_stream[1524];
	size_t length;
} goose_handle;

goose_handle* goose_init(uint8_t source[MAC_ADDRESS_SIZE], uint8_t destination[MAC_ADDRESS_SIZE], uint8_t app_id[APP_ID_SIZE]);
void goose_all_data_entry_add(goose_handle* handle, uint8_t type, size_t length, uint8_t* value);
void goose_all_data_entry_modify(goose_handle* handle, size_t index, uint8_t new_type, size_t new_length, uint8_t* new_value);
void goose_all_data_entry_remove(goose_handle* handle, size_t index);
void goose_encode(goose_handle* handle);
void goose_free(goose_handle* handle);
uint16_t goose_htons(uint16_t hostshort);
uint32_t goose_htonl(uint32_t hostlong);
uint64_t goose_htonll(uint64_t hostlonglong);