#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "isotp.h"

static unsigned calls;
void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_ms(void) { return 0; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id; (void)data; (void)size;
    ++calls;
    return ISOTP_RET_OK;
}
int main(int argc, char **argv) {
    uint8_t tx[32], rx[32], payload[8] = {1};
    IsoTpLink link;
    assert(argc == 2);
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    switch (argv[1][0]) {
    case 'n':
        assert(isotp_send(NULL, payload, 1) == ISOTP_RET_ERROR);
        assert(isotp_send_with_id(NULL, 0x123, payload, 1) == ISOTP_RET_ERROR);
        break;
    case 'p':
        assert(isotp_send(&link, NULL, 1) == ISOTP_RET_ERROR);
        assert(isotp_send_with_id(&link, 0x123, NULL, 1) == ISOTP_RET_ERROR);
        break;
    case 'z':
        assert(isotp_send(&link, payload, 0) == ISOTP_RET_LENGTH);
        assert(isotp_send_with_id(&link, 0x123, payload, 0) == ISOTP_RET_LENGTH);
        break;
    default:
        assert(isotp_send(&link, payload, 1) == ISOTP_RET_OK);
        assert(isotp_send_with_id(&link, 0x456, payload, 7) == ISOTP_RET_OK);
        assert(calls == 2);
        puts("Valid sends passed.");
        return 0;
    }
    assert(calls == 0);
    assert(link.send_status == ISOTP_SEND_STATUS_IDLE);
    assert(link.send_size == 0);
    puts("Invalid send rejected before changing state or calling CAN.");
    return 0;
}
