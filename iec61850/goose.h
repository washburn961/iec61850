#pragma once

#include <stdint.h>
#include "iec_time.h"

#define APP_ID_SIZE 2
#define RESERVED_SIZE 2
#define ETHER_TYPE_SIZE 2
#define MAC_ADDRESS_SIZE 6

typedef struct
{
	uint8_t source_mac_address[MAC_ADDRESS_SIZE];
	uint8_t destination_mac_address[MAC_ADDRESS_SIZE];
	uint8_t ether_type[ETHER_TYPE_SIZE];
	uint8_t app_id[APP_ID_SIZE];
	uint16_t len;
	uint8_t reserved_1[RESERVED_SIZE];
	uint8_t reserved_2[RESERVED_SIZE];
	goose_pdu pdu;
} goose_frame;

typedef struct
{
	uint8_t* gocbref;
	uint32_t time_allowed_to_live;
	uint8_t* dataset;
	uint8_t* go_id;
	iec_time t;
	uint32_t st_num;
	uint32_t sq_num;
	uint8_t simulation;
	uint8_t conf_rev;
	uint8_t nds_com;
	uint16_t num_dataset_entries;
	goose_all_data all_data;
} goose_pdu;

typedef struct
{
	goose_data_entry* entries;
} goose_all_data;

typedef struct
{
	uint8_t type;
	uint32_t length;
	uint8_t* value;
} goose_data_entry;