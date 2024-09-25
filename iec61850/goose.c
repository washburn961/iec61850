#include "goose.h"
#include "ber.h"

goose_handle* goose_init(uint8_t source[MAC_ADDRESS_SIZE], uint8_t destination[MAC_ADDRESS_SIZE], uint8_t app_id[APP_ID_SIZE])
{
	goose_handle* handle = (goose_handle*)malloc(sizeof(goose_handle));
	if (!handle)
	{
		return NULL;
	}

	handle->frame = (goose_frame*)malloc(sizeof(goose_frame));
	if (!handle->frame)
	{
		return NULL;
	}

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

	ber_init(&(handle->frame->pdu.gocbref), TAG_GOCBREF);
	ber_init(&(handle->frame->pdu.time_allowed_to_live), TAG_TIME_ALLOWED_TO_LIVE);
	ber_init(&(handle->frame->pdu.dataset), TAG_DATASET);
	ber_init(&(handle->frame->pdu.go_id), TAG_GO_ID);
	ber_init(&(handle->frame->pdu.t), TAG_T);
	ber_init(&(handle->frame->pdu.st_num), TAG_ST_NUM);
	ber_init(&(handle->frame->pdu.sq_num), TAG_SQ_NUM);
	ber_init(&(handle->frame->pdu.simulation), TAG_SIMULATION);
	ber_init(&(handle->frame->pdu.conf_rev), TAG_CONF_REV);
	ber_init(&(handle->frame->pdu.nds_com), TAG_NDS_COM);
	ber_init(&(handle->frame->pdu.num_dataset_entries), TAG_NUM_DATASET_ENTRIES);

	handle->frame->pdu.all_data.tag = TAG_ALL_DATA;
	for (size_t i = 0; i < MAX_NUM_DATASET_ENTRIES; i++)
	{
		handle->frame->pdu.all_data.entries[i].name = "";
		ber_init(&(handle->frame->pdu.all_data.entries[i].data), 0x0);
	}
	handle->frame->pdu.all_data.entry_count = 0x0;

	handle->byte_stream = NULL;
	handle->length = 0x0;

	return handle;
}

// Function to add a new entry to all_data
void goose_all_data_entry_add(goose_handle* handle, const char* name, uint8_t type, size_t length, uint8_t* value)
{
	if (!handle || handle->frame->pdu.all_data.entry_count >= MAX_NUM_DATASET_ENTRIES)
		return;

	goose_all_data* all_data = &handle->frame->pdu.all_data;

	// Add new entry at the next available index
	goose_all_data_entry* new_entry = &all_data->entries[all_data->entry_count];

	new_entry->name = name;  // Assign name

	// Initialize the ber field
	ber_init(&new_entry->data, type);
	new_entry->data.length = length;
	new_entry->data.value = (uint8_t*)malloc(length);
	if (!new_entry->data.value)
		return;

	memcpy(new_entry->data.value, value, length);  // Copy the value

	all_data->entry_count++;  // Increment entry count
}

// Function to modify an existing entry in all_data
void goose_all_data_entry_modify(goose_handle* handle, const char* name, uint8_t new_type, size_t new_length, uint8_t* new_value)
{
	if (!handle)
		return;

	goose_all_data* all_data = &handle->frame->pdu.all_data;

	// Find the entry by name
	for (size_t i = 0; i < all_data->entry_count; i++)
	{
		goose_all_data_entry* entry = &all_data->entries[i];
		if (strcmp(entry->name, name) == 0)
		{
			// Modify the existing entry
			free(entry->data.value);  // Free the old value

			entry->data.tag = new_type;
			entry->data.length = new_length;
			entry->data.value = (uint8_t*)malloc(new_length);
			if (!entry->data.value)
				return;

			memcpy(entry->data.value, new_value, new_length);  // Copy new value
			return;
		}
	}
}

// Function to remove an entry by name from all_data
void goose_all_data_entry_remove(goose_handle* handle, const char* name)
{
	if (!handle)
		return;

	goose_all_data* all_data = &handle->frame->pdu.all_data;

	// Find the entry by name
	for (size_t i = 0; i < all_data->entry_count; i++)
	{
		goose_all_data_entry* entry = &all_data->entries[i];
		if (strcmp(entry->name, name) == 0)
		{
			// Free the entry's ber value
			free(entry->data.value);

			// Shift the remaining entries in the array to fill the gap
			for (size_t j = i; j < all_data->entry_count - 1; j++)
			{
				all_data->entries[j] = all_data->entries[j + 1];
			}

			all_data->entry_count--;  // Decrement entry count
			return;
		}
	}
}

void goose_encode(goose_handle* handle)
{

}

void goose_free(goose_handle* handle)
{
	if (!handle) return;  // If handle is NULL, nothing to free
	if (!handle->frame)   // If frame is NULL, free the handle and return
	{
		free(handle);
		return;
	}

	// Free each PDU field if it was allocated
	free(handle->frame->pdu.gocbref.value);
	free(handle->frame->pdu.time_allowed_to_live.value);
	free(handle->frame->pdu.dataset.value);
	free(handle->frame->pdu.go_id.value);
	free(handle->frame->pdu.t.value);
	free(handle->frame->pdu.st_num.value);
	free(handle->frame->pdu.sq_num.value);
	free(handle->frame->pdu.simulation.value);
	free(handle->frame->pdu.conf_rev.value);
	free(handle->frame->pdu.nds_com.value);
	free(handle->frame->pdu.num_dataset_entries.value);

	for (size_t i = 0; i < handle->frame->pdu.all_data.entry_count; i++)
	{
		if (!handle->frame->pdu.all_data.entries[i].data.value) continue;
		free(handle->frame->pdu.all_data.entries[i].data.value);
	}

	// Free the byte stream if allocated
	free(handle->byte_stream);

	// Free the frame and handle itself
	free(handle->frame);
	free(handle);
}
