#include "goose.h"
#include "ber.h"
#include <string.h>

uint16_t goose_htons(uint16_t hostshort)
{
	return (hostshort >> 8) | (hostshort << 8);
}

// Convert 32-bit unsigned integer to big-endian
uint32_t goose_htonl(uint32_t hostlong)
{
	return ((hostlong >> 24) & 0x000000FF) |
		((hostlong >> 8) & 0x0000FF00) |
		((hostlong << 8) & 0x00FF0000) |
		((hostlong << 24) & 0xFF000000);
}

// Convert 64-bit unsigned integer to big-endian
uint64_t goose_htonll(uint64_t hostlonglong)
{
	return ((hostlonglong >> 56) & 0x00000000000000FFULL) |
		((hostlonglong >> 40) & 0x000000000000FF00ULL) |
		((hostlonglong >> 24) & 0x0000000000FF0000ULL) |
		((hostlonglong >> 8) & 0x00000000FF000000ULL) |
		((hostlonglong << 8) & 0x000000FF00000000ULL) |
		((hostlonglong << 24) & 0x0000FF0000000000ULL) |
		((hostlonglong << 40) & 0x00FF000000000000ULL) |
		((hostlonglong << 56) & 0xFF00000000000000ULL);
}

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

	ber_init(&(handle->frame->pdu_list.gocbref), TAG_GOCBREF);
	ber_init(&(handle->frame->pdu_list.time_allowed_to_live), TAG_TIME_ALLOWED_TO_LIVE);
	ber_init(&(handle->frame->pdu_list.dataset), TAG_DATASET);
	ber_init(&(handle->frame->pdu_list.go_id), TAG_GO_ID);
	ber_init(&(handle->frame->pdu_list.t), TAG_T);
	ber_init(&(handle->frame->pdu_list.st_num), TAG_ST_NUM);
	ber_init(&(handle->frame->pdu_list.sq_num), TAG_SQ_NUM);
	ber_init(&(handle->frame->pdu_list.simulation), TAG_SIMULATION);
	ber_init(&(handle->frame->pdu_list.conf_rev), TAG_CONF_REV);
	ber_init(&(handle->frame->pdu_list.nds_com), TAG_NDS_COM);
	ber_init(&(handle->frame->pdu_list.num_dataset_entries), TAG_NUM_DATASET_ENTRIES);
	ber_init(&(handle->frame->pdu_list.all_data), TAG_ALL_DATA);
	ber_init(&(handle->frame->pdu), TAG_PDU);

	for (size_t i = 0; i < MAX_NUM_DATASET_ENTRIES; i++)
	{
		ber_init(&(handle->frame->pdu_list.all_data_list.entries[i]), 0x0);
	}
	handle->frame->pdu_list.all_data_list.entry_count = 0x0;
	memset(&(handle->byte_stream), 0x0, sizeof(handle->byte_stream));
	handle->length = 0x0;

	return handle;
}

// Function to add a new entry to all_data_list by type and value
void goose_all_data_entry_add(goose_handle* handle, uint8_t type, size_t length, uint8_t* value)
{
	if (!handle || handle->frame->pdu_list.all_data_list.entry_count >= MAX_NUM_DATASET_ENTRIES)
		return;

	goose_all_data* all_data_list = &handle->frame->pdu_list.all_data_list;

	// Add new entry at the next available index
	ber* new_entry_data = &all_data_list->entries[all_data_list->entry_count];

	// Initialize the BER data with the type
	ber_init(new_entry_data, type);
	new_entry_data->length = length;
	new_entry_data->value = (uint8_t*)malloc(length);

	if (!new_entry_data->value)
		return;  // Handle memory allocation failure

	// Copy the provided value into the new entry
	memcpy(new_entry_data->value, value, length);

	all_data_list->entry_count++;  // Increment the entry count
}

// Function to modify an existing entry in all_data_list by index
void goose_all_data_entry_modify(goose_handle* handle, size_t index, uint8_t new_type, size_t new_length, uint8_t* new_value)
{
	if (!handle || index >= handle->frame->pdu_list.all_data_list.entry_count)
		return;

	goose_all_data* all_data_list = &handle->frame->pdu_list.all_data_list;

	// Get the entry at the specified index
	ber* entry_data = &all_data_list->entries[index];

	// Free the old value before modifying
	free(entry_data->value);

	// Update the tag and reallocate for the new value
	ber_init(entry_data, new_type);
	entry_data->length = new_length;
	entry_data->value = (uint8_t*)malloc(new_length);
	if (!entry_data->value)
		return;  // Handle memory allocation failure

	// Copy the new value into the entry
	memcpy(entry_data->value, new_value, new_length);
}

// Function to remove an entry by index from all_data_list
void goose_all_data_entry_remove(goose_handle* handle, size_t index)
{
	if (!handle || index >= handle->frame->pdu_list.all_data_list.entry_count)
		return;

	goose_all_data* all_data_list = &handle->frame->pdu_list.all_data_list;

	// Free the memory allocated for the value in the BER entry
	free(all_data_list->entries[index].value);

	// Shift the remaining entries in the array to fill the gap
	for (size_t j = index; j < all_data_list->entry_count - 1; j++)
	{
		all_data_list->entries[j] = all_data_list->entries[j + 1];
	}

	all_data_list->entry_count--;  // Decrement entry count
}

void goose_encode(goose_handle* handle)
{
	uint8_t* temp_bytes = NULL;
	size_t temp_bytes_len = 0;
	size_t offset = 0;

	// Reset length and start serializing into the static byte stream
	handle->length = 0;

	// Calculate the base frame size and ensure it fits in the byte stream
	size_t header_size = MAC_ADDRESS_SIZE * 2 + ETHERTYPE_SIZE + APP_ID_SIZE + sizeof(handle->frame->len) + 2 * RESERVED_SIZE;

	// Encode all_data entries and PDU fields
	handle->frame->pdu_list.all_data.length = ber_encode_many(
		handle->frame->pdu_list.all_data_list.entries,
		handle->frame->pdu_list.all_data_list.entry_count,
		&(handle->frame->pdu_list.all_data.value)
	);

	uint16_t num_dataset_entries = goose_htons(handle->frame->pdu_list.all_data_list.entry_count);
	ber_set(&(handle->frame->pdu_list.num_dataset_entries), (uint8_t*)&num_dataset_entries, sizeof(num_dataset_entries));

	handle->frame->pdu.length = ber_encode_many( //TO CHATGPT: appears there is something wrong with this function. the pdu_fields list i pass to it is not the same i see when i step in the function...
		&(handle->frame->pdu_list),
		GOOSE_PDU_FIELD_COUNT,
		&(handle->frame->pdu.value)
	);

	// Now encode the final PDU
	temp_bytes_len = ber_encode(&(handle->frame->pdu), &temp_bytes);

	// Ensure the PDU fits in the static byte stream
	if (offset + temp_bytes_len > sizeof(handle->byte_stream))
	{
		// Data exceeds byte stream size
		handle->length = 0;
		free(temp_bytes);
		return;
	}

	uint16_t total_goose_len = temp_bytes_len + header_size - (MAC_ADDRESS_SIZE * 2 + ETHERTYPE_SIZE);
	handle->frame->len = goose_htons(total_goose_len);

	// Manually serialize the frame fields
	memcpy(handle->byte_stream, handle->frame->destination, MAC_ADDRESS_SIZE);
	offset += MAC_ADDRESS_SIZE;

	memcpy(&(handle->byte_stream[offset]), handle->frame->source, MAC_ADDRESS_SIZE);
	offset += MAC_ADDRESS_SIZE;

	memcpy(&(handle->byte_stream[offset]), handle->frame->ethertype, ETHERTYPE_SIZE);
	offset += ETHERTYPE_SIZE;

	memcpy(&(handle->byte_stream[offset]), handle->frame->app_id, APP_ID_SIZE);
	offset += APP_ID_SIZE;

	memcpy(&(handle->byte_stream[offset]), &(handle->frame->len), sizeof(handle->frame->len));
	offset += sizeof(handle->frame->len);

	memcpy(&(handle->byte_stream[offset]), handle->frame->reserved_1, RESERVED_SIZE);
	offset += RESERVED_SIZE;

	memcpy(&(handle->byte_stream[offset]), handle->frame->reserved_2, RESERVED_SIZE);
	offset += RESERVED_SIZE;

	// Copy the encoded PDU into the byte stream
	memcpy(&(handle->byte_stream[offset]), temp_bytes, temp_bytes_len);

	// Update the total length
	handle->length = offset + temp_bytes_len;

	// Free temporary buffer
	free(temp_bytes);
}



void goose_free(goose_handle* handle)
{
	if (!handle) return;  // If handle is NULL, nothing to free

	if (handle->frame)
	{
		// Free each PDU field if it was allocated
		free(handle->frame->pdu_list.gocbref.value);
		free(handle->frame->pdu_list.time_allowed_to_live.value);
		free(handle->frame->pdu_list.dataset.value);
		free(handle->frame->pdu_list.go_id.value);
		free(handle->frame->pdu_list.t.value);
		free(handle->frame->pdu_list.st_num.value);
		free(handle->frame->pdu_list.sq_num.value);
		free(handle->frame->pdu_list.simulation.value);
		free(handle->frame->pdu_list.conf_rev.value);
		free(handle->frame->pdu_list.nds_com.value);
		free(handle->frame->pdu_list.num_dataset_entries.value);
		free(handle->frame->pdu_list.all_data.value);
		free(handle->frame->pdu.value);

		// Free the all_data_list entries
		for (size_t i = 0; i < handle->frame->pdu_list.all_data_list.entry_count; i++)
		{
			// Free the value associated with each BER entry
			free(handle->frame->pdu_list.all_data_list.entries[i].value);
		}

		// Free the frame structure
		free(handle->frame);
	}

	// Finally, free the handle itself
	free(handle);
}

