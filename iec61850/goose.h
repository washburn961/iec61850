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

#define PDU_GOCBREF 0x0
#define PDU_TIME_ALLOWED_TO_LIVE 0x1
#define PDU_DATASET 0x2
#define PDU_GO_ID 0x3
#define PDU_T 0x4
#define PDU_ST_NUM 0x5
#define PDU_SQ_NUM 0x6
#define PDU_SIMULATION 0x7
#define PDU_CONF_REV 0x8
#define PDU_NDS_COM 0x9
#define PDU_NUM_DATASET_ENTRIES 0xa
#define PDU_ALL_DATA 0xb

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

typedef struct
{
	ber pdu_fields[GOOSE_PDU_FIELD_COUNT];
} goose_pdu;

typedef struct
{
	uint8_t source[MAC_ADDRESS_SIZE];
	uint8_t destination[MAC_ADDRESS_SIZE];
	uint8_t ethertype[ETHERTYPE_SIZE];
	uint8_t app_id[APP_ID_SIZE];
	uint16_t len;
	uint8_t reserved_1[RESERVED_SIZE];
	uint8_t reserved_2[RESERVED_SIZE];
	goose_pdu pdu;
} goose_frame;

typedef struct {
	goose_frame* frame;
	uint8_t* byte_stream;
	size_t length;
} goose_handle;

goose_handle* goose_init(uint8_t source[MAC_ADDRESS_SIZE], uint8_t destination[MAC_ADDRESS_SIZE], uint8_t app_id[APP_ID_SIZE]);
void goose_encode(goose_handle* handle);
void goose_free(goose_handle* handle);