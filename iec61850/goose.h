#pragma once

#include <stdint.h>
#include "ber.h"

#define APP_ID_SIZE 2
#define RESERVED_SIZE 2
#define ETHER_TYPE_SIZE 2
#define MAC_ADDRESS_SIZE 6

typedef struct
{
	ber* entries;
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
	goose_all_data all_data;
} goose_pdu;

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