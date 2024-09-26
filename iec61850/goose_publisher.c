#include "goose_publisher.h"
#include "semaphore_interface.h"
#include <string.h>

// Static goose_publisher instance
static goose_publisher publisher;

// Static semaphore to guard access to the publisher
static semaphore_t* publisher_semaphore;

void goose_message_housekeeping(goose_message_params* params);

uint64_t time_elapsed_ns = 0;
uint32_t time_elapsed_ms = 0;

// Initialize the GOOSE publisher
void goose_publisher_init(linkoutput output)
{
    // Initialize the semaphore
    publisher_semaphore = semaphore_create();

    // Set the linkoutput function
    publisher.output = output;

    for (size_t i = 0; i < MAX_GOOSE_MESSAGES; i++)
    {
        publisher.message_list[i].name = NULL;
        publisher.message_list[i].handle = NULL;
        publisher.message_list[i].default_time_allowed_to_live = 0;
        publisher.message_list[i].burst_count = 0;
    }
}

// Register a new GOOSE message
void goose_publisher_register(goose_message_params params)
{
    semaphore_take(publisher_semaphore);

    for (size_t i = 0; i < MAX_GOOSE_MESSAGES; i++)
    {
        if (publisher.message_list[i].name == NULL)
        {
            publisher.message_list[i] = params;
            break;
        }
    }

    semaphore_release(publisher_semaphore);
}

// Deregister a GOOSE message by name
void goose_publisher_deregister(const char* name)
{
    semaphore_take(publisher_semaphore);

    for (int i = 0; i < MAX_GOOSE_MESSAGES; i++)
    {
        if (publisher.message_list[i].name && strcmp(publisher.message_list[i].name, name) == 0)
        {
            publisher.message_list[i].name = NULL;
            publisher.message_list[i].handle = NULL;
            publisher.message_list[i].default_time_allowed_to_live = 0;
            publisher.message_list[i].burst_count = 0;
            break;
        }
    }

    semaphore_release(publisher_semaphore);
}

// Notify and publish a GOOSE message by name
void goose_publisher_notify(const char* name)
{
    semaphore_take(publisher_semaphore);

    for (int i = 0; i < MAX_GOOSE_MESSAGES; i++)
    {
        if (publisher.message_list[i].name && strcmp(publisher.message_list[i].name, name) == 0)
        {

        }
    }

    semaphore_release(publisher_semaphore);
}

// Process function (can be called periodically)
void goose_publisher_process(void)
{
    semaphore_take(publisher_semaphore);

    for (size_t i = 0; i < MAX_GOOSE_MESSAGES; i++)
    {
        if (publisher.message_list[i].name)
        {
            goose_message_housekeeping(&(publisher.message_list[i]));
        }
    }

    time_elapsed_ns += 1000000000ULL;
    time_elapsed_ms += 1ULL;

    semaphore_release(publisher_semaphore);
}


void goose_message_housekeeping(goose_message_params* params)
{
    // Case 1: If message was updated, reset and transmit
    if (params->updated)
    {
        params->burst_count = 6;  // Reset burst count to 6
        params->current_time_allowed_to_live = 3;  // Set current TATL to 3
        params->time_since_last_transmission = 0;  // Reset the time since last transmission
        params->updated = 0;  // Clear the updated flag

        // Increment st_num and reset sq_num to 0
        params->st_num++;  // Use the st_num from the params struct
        params->sq_num = 0;  // Reset sq_num to 0

        // Set the time allowed to live in network byte order
        uint16_t time_allowed_to_live_net = goose_htons(params->current_time_allowed_to_live);

        // Convert st_num and sq_num to network byte order
        uint32_t st_num_net = goose_htonl(params->st_num);
        uint32_t sq_num_net = goose_htonl(params->sq_num);

        // Update the PDU with new values
        ber_set(&(params->handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live_net, sizeof(time_allowed_to_live_net));
        ber_set(&(params->handle->frame->pdu_list.st_num), (uint8_t*)&st_num_net, sizeof(st_num_net));
        ber_set(&(params->handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num_net, sizeof(sq_num_net));

        // Re-encode the GOOSE message and transmit it
        goose_encode(params->handle);
        publisher.output(params->handle->byte_stream, params->handle->length);

        return;  // Return after transmission
    }

    // Case 2: If time since last transmission is less than TATL, increment and return
    if (params->time_since_last_transmission < params->current_time_allowed_to_live)
    {
        params->time_since_last_transmission++;
        return;
    }

    // Case 3: If burst_count > 0, decrement burst count, double the TATL, increment sq_num, and transmit
    if (params->burst_count > 0)
    {
        params->burst_count--;  // Decrement burst count
        params->current_time_allowed_to_live *= 2;  // Double the current TATL

        // Increment sq_num
        params->sq_num++;

        // Convert sq_num to network byte order
        uint32_t sq_num_net = goose_htonl(params->sq_num);
        uint16_t time_allowed_to_live_net = goose_htons(params->current_time_allowed_to_live);

        // Update the PDU with the new sq_num and time allowed to live
        ber_set(&(params->handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num_net, sizeof(sq_num_net));
        ber_set(&(params->handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live_net, sizeof(time_allowed_to_live_net));

        // Re-encode the GOOSE message and transmit it
        goose_encode(params->handle);
        publisher.output(params->handle->byte_stream, params->handle->length);

        params->time_since_last_transmission = 0;  // Reset time since last transmission

        return;
    }

    // Case 4: Normal case, increment sq_num and transmit
    params->sq_num++;  // Increment sq_num in the params struct

    // Convert sq_num to network byte order
    uint32_t sq_num_net = goose_htonl(params->sq_num);

    // Update the PDU with the incremented sq_num
    ber_set(&(params->handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num_net, sizeof(sq_num_net));

    if (params->current_time_allowed_to_live != params->default_time_allowed_to_live)
    {
        params->current_time_allowed_to_live = params->default_time_allowed_to_live;
        uint16_t time_allowed_to_live_net = goose_htons(params->current_time_allowed_to_live);
        ber_set(&(params->handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live_net, sizeof(time_allowed_to_live_net));
    }

    // Re-encode the GOOSE message and transmit it
    goose_encode(params->handle);
    publisher.output(params->handle->byte_stream, params->handle->length);

    params->time_since_last_transmission = 0;  // Reset time since last transmission
}
