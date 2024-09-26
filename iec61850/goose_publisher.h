#include <stdint.h>
#include "goose.h"

#define MAX_GOOSE_MESSAGES 16

typedef void (*linkoutput)(uint8_t* byte_stream, size_t length);

typedef struct
{
	const char* name;
	goose_handle* handle;
	uint16_t default_time_allowed_to_live;
	uint16_t current_time_allowed_to_live;
	uint16_t time_since_last_transmission;
	uint32_t st_num;
	uint32_t sq_num;
	size_t burst_count;
	uint8_t updated;
} goose_message_params;

typedef struct
{
	goose_message_params message_list[MAX_GOOSE_MESSAGES];
	linkoutput output;
} goose_publisher;

void goose_publisher_init(linkoutput output);
void goose_publisher_register(goose_message_params params);
void goose_publisher_deregister(const char* name);
void goose_publisher_notify(const char* name);
void goose_publisher_process(void);