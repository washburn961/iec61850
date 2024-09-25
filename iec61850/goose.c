#include "goose.h"
#include "ber.h"

#define MAX_GOOSE_FRAMES 16

static goose_handle handle_list[MAX_GOOSE_FRAMES];
static size_t handles_in_use = 0;

goose_handle* goose_init(uint8_t source[MAC_ADDRESS_SIZE], uint8_t destination[MAC_ADDRESS_SIZE], uint8_t app_id[APP_ID_SIZE])
{
	if (handles_in_use == MAX_GOOSE_FRAMES - 1) return NULL;

	goose_handle* handle = &handle_list[handles_in_use];

	handle->frame = (goose_frame*)malloc(sizeof(goose_frame));
	if (!handle->frame)
	{
		return NULL;
	}

	handles_in_use++;

	memcpy(handle->frame->source, source, MAC_ADDRESS_SIZE);
	memcpy(handle->frame->destination, destination, MAC_ADDRESS_SIZE);
	handle->frame->ethertype[0] = GOOSE_ETHERTYPE_0;
	handle->frame->ethertype[1] = GOOSE_ETHERTYPE_1;
	handle->frame->app_id[0] = 0x0;
	handle->frame->app_id[1] = 0x0;
	handle->frame->len = 0x0;
	handle->frame->reserved_1[0] = 0x0;
	handle->frame->reserved_1[1] = 0x0;
	handle->frame->reserved_2[0] = 0x0;
	handle->frame->reserved_2[1] = 0x0;

	handle->frame->pdu.pdu_fields[PDU_GOCBREF].tag = TAG_GOCBREF;
	handle->frame->pdu.pdu_fields[PDU_TIME_ALLOWED_TO_LIVE].tag = TAG_TIME_ALLOWED_TO_LIVE;
	handle->frame->pdu.pdu_fields[PDU_DATASET].tag = TAG_DATASET;
	handle->frame->pdu.pdu_fields[PDU_GO_ID].tag = TAG_GO_ID;
	handle->frame->pdu.pdu_fields[PDU_T].tag = TAG_T;
	handle->frame->pdu.pdu_fields[PDU_ST_NUM].tag = TAG_ST_NUM;
	handle->frame->pdu.pdu_fields[PDU_SQ_NUM].tag = TAG_SQ_NUM;
	handle->frame->pdu.pdu_fields[PDU_SIMULATION].tag = TAG_SIMULATION;
	handle->frame->pdu.pdu_fields[PDU_CONF_REV].tag = TAG_CONF_REV;
	handle->frame->pdu.pdu_fields[PDU_NDS_COM].tag = TAG_NDS_COM;
	handle->frame->pdu.pdu_fields[PDU_NUM_DATASET_ENTRIES].tag = TAG_NUM_DATASET_ENTRIES;
	handle->frame->pdu.pdu_fields[PDU_ALL_DATA].tag = TAG_ALL_DATA;

	return handle;
}

void goose_encode(goose_handle* handle)
{

}

void goose_free(goose_handle* handle)
{
	if (!handle->frame)
	{
		return;
	}

	for (size_t i = 0; i < GOOSE_PDU_FIELD_COUNT; i++)
	{
		if (handle->frame->pdu.pdu_fields[i].value)
		{
			free(handle->frame->pdu.pdu_fields[i].value);
		}
	}

	if (handle->byte_stream)
	{
		free(handle->byte_stream);
	}

	free(handle->frame);

	handle->frame = NULL;
	handle->byte_stream = NULL;
	handle->length = 0;
}