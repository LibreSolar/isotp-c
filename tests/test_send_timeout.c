#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "isotp.h"

static unsigned calls;
static uint32_t now;
void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_ms(void) { return now; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id; (void)data; (void)size;
    ++calls;
    return ISOTP_RET_OK;
}
static void run_case(uint32_t start) {
    uint8_t tx[32], rx[32], payload[20] = {0};
    IsoTpLink link;
    calls = 0;
    now = start;
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    assert(isotp_send(&link, payload, sizeof(payload)) == ISOTP_RET_OK);
    now += ISO_TP_DEFAULT_RESPONSE_TIMEOUT + 1;
    isotp_poll(&link);
    assert(calls == 1);
    assert(link.send_status == ISOTP_SEND_STATUS_ERROR);
    assert(link.send_protocol_result == ISOTP_PROTOCOL_RESULT_TIMEOUT_BS);
    assert(link.send_offset == 6);
}
static void timely_send(void) {
    uint8_t tx[32], rx[32], payload[20] = {0};
    uint8_t fc[] = {0x30, 0, 0};
    IsoTpLink link;
    calls = 0;
    now = 0;
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    assert(isotp_send(&link, payload, sizeof(payload)) == ISOTP_RET_OK);
    isotp_on_can_message(&link, fc, sizeof(fc));
    now = 1;
    isotp_poll(&link);
    now = 2;
    isotp_poll(&link);
    assert(calls == 3);
    assert(link.send_status == ISOTP_SEND_STATUS_IDLE);
    assert(link.send_protocol_result == ISOTP_PROTOCOL_RESULT_OK);
    now = 1000;
    isotp_poll(&link);
    assert(link.send_status == ISOTP_SEND_STATUS_IDLE);
}
static void slow_transfer(uint32_t start) {
    uint8_t tx[32], rx[32], payload[20] = {0};
    uint8_t fc[] = {0x30, 0, 127};
    IsoTpLink link;
    calls = 0;
    now = start;
    isotp_init_link(&link, 0x123, tx, sizeof(tx), rx, sizeof(rx));
    assert(isotp_send(&link, payload, sizeof(payload)) == ISOTP_RET_OK);
    isotp_on_can_message(&link, fc, sizeof(fc));
    now += 101;
    isotp_poll(&link);
    assert(link.send_status == ISOTP_SEND_STATUS_INPROGRESS);
    now = start + 128;
    isotp_poll(&link);
    now += 128;
    isotp_poll(&link);
    assert(calls == 3);
    assert(link.send_status == ISOTP_SEND_STATUS_IDLE);
}
int main(void) {
    slow_transfer(0);
    slow_transfer(UINT32_MAX - 50);
    timely_send();
    run_case(0);
    run_case(UINT32_MAX - 100);
    puts("Flow-control waits expire, but valid slow transfers finish, including clock rollover.");
}
