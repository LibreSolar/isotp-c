#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "isotp.h"
static uint32_t now;
static unsigned calls;
void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_ms(void) { return now; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id; (void)data; (void)size; ++calls;
    return ISOTP_RET_OK;
}
static void check(uint8_t stmin, uint8_t expected) {
    uint8_t tx[32], rx[32], payload[20] = {0};
    uint8_t flow[] = {0x30, 0, stmin};
    IsoTpLink link;
    now = 0; calls = 0;
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    assert(isotp_send(&link, payload, sizeof(payload)) == ISOTP_RET_OK);
    isotp_on_can_message(&link, flow, sizeof(flow));
    assert(link.send_st_min == expected);
    now = 1; isotp_poll(&link);
    assert(calls == 2);
    now = 2; isotp_poll(&link);
    assert(calls == (expected == 0 ? 3U : 2U));
}
int main(void) {
    check(0, 0); check(10, 10); check(0x7F, 127);
    check(0xF1, 1); check(0xF9, 1);
    check(0x80, 127); check(0xF0, 127); check(0xFA, 127); check(0xFF, 127);
    puts("Nine separation-time boundary and reserved-value cases passed.");
}
