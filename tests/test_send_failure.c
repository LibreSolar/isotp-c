#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "isotp.h"

static unsigned calls;
static int can_result;
void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_ms(void) { return 0; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id; (void)data; (void)size;
    ++calls;
    return can_result;
}
int main(void) {
    uint8_t tx[32], rx[32], payload[20] = {0};
    uint8_t fc[] = {0x30, 0, 0};
    IsoTpLink link;
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    can_result = ISOTP_RET_OK;
    assert(isotp_send(&link, payload, sizeof(payload)) == ISOTP_RET_OK);
    isotp_on_can_message(&link, fc, sizeof(fc));
    can_result = ISOTP_RET_ERROR;
    isotp_poll(&link);
    assert(calls == 2);
    assert(link.send_status == ISOTP_SEND_STATUS_ERROR);
    assert(link.send_protocol_result == ISOTP_PROTOCOL_RESULT_ERROR);
    assert(link.send_offset == 6);
    isotp_poll(&link);
    assert(calls == 2);
    puts("Failed consecutive-frame send is reported and not retried.");
}
