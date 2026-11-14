/* Host regression tests: real ISO-TP framing with an in-memory CAN shim. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "isotp.h"

static uint8_t send_buffer[UINT16_MAX];
static uint8_t receive_buffer[UINT16_MAX];
static uint8_t payload[UINT16_MAX];
static uint8_t frame[8];
static unsigned frames;
static uint32_t now;

void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_ms(void) { return now; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id;
    assert(size <= sizeof(frame));
    memcpy(frame, data, size);
    ++frames;
    return ISOTP_RET_OK;
}

static void init(IsoTpLink *link, uint16_t capacity) {
    isotp_init_link(link, 0x123, send_buffer, capacity,
                    receive_buffer, sizeof(receive_buffer));
    memset(send_buffer, 0x5a, sizeof(send_buffer));
    frames = 0;
    now = 0;
}

static void valid_size(uint16_t size) {
    IsoTpLink link;
    init(&link, sizeof(send_buffer));
    assert(isotp_send(&link, payload, size) == ISOTP_RET_OK);
    assert(frames == 1);
    if (size <= 7) {
        assert((frame[0] >> 4) == 0);
        assert((frame[0] & 0x0f) == size);
    } else {
        assert((frame[0] >> 4) == 1);
        assert((((frame[0] & 0x0f) << 8) | frame[1]) == size);
        uint8_t flow_control[] = {0x30, 0, 0};
        isotp_on_can_message(&link, flow_control, sizeof(flow_control));
        while (link.send_status == ISOTP_SEND_STATUS_INPROGRESS) {
            ++now;
            isotp_poll(&link);
        }
        assert(link.send_status == ISOTP_SEND_STATUS_IDLE);
        assert(link.send_offset == size);
    }
}

static void oversized(uint16_t size, uint16_t capacity, int custom_id) {
    IsoTpLink link, original;
    init(&link, capacity);
    original = link;
    int result = custom_id ? isotp_send_with_id(&link, 0x456, payload, size)
                           : isotp_send(&link, payload, size);
    assert(result == ISOTP_RET_OVERFLOW);
    assert(frames == 0);
    assert(memcmp(&link, &original, sizeof(link)) == 0);
    for (unsigned i = 0; i < sizeof(send_buffer); ++i)
        assert(send_buffer[i] == 0x5a);
}

int main(void) {
    valid_size(1);
    valid_size(7);
    valid_size(8);
    valid_size(4095);
    oversized(4096, sizeof(send_buffer), 0);
    oversized(UINT16_MAX, sizeof(send_buffer), 0);
    oversized(4096, sizeof(send_buffer), 1);
    oversized(8, 7, 0);
    puts("All eight send-length cases passed.");
    return 0;
}
