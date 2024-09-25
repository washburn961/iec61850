#include <stdio.h>
#include <string.h>
#include "ber.h"
#include "goose.h"

goose_handle* handle;

int main() {

    uint8_t src[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    uint8_t dst[] = GOOSE_MULTICAST_ADDRESS(0x0, 0x0);
    uint8_t app_id[] = { 0x0, 0x0 };
    handle = goose_init(src, dst, app_id);
    return 0;
}
