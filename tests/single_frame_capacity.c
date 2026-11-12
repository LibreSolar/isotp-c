#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "isotp.h"

void isotp_user_debug(const char *message, ...) { (void)message; }
int isotp_user_send_can(uint32_t id, const uint8_t *data, uint8_t size) {
    (void)id; (void)data; (void)size;
    /* Single-frame receptions must not generate flow-control messages. */
    assert(0);
    return ISOTP_RET_OK;
}
uint32_t isotp_user_get_ms(void) { return 0; }

int main(void) {
    for (uint16_t capacity = 0; capacity <= 8; capacity++) {
        for (uint8_t size = 1; size <= 7; size++) {
            IsoTpLink link;
            uint8_t sendbuf[8];
            uint8_t *recvbuf = malloc(capacity ? capacity : 1);
            uint8_t frame[] = {size, 1, 2, 3, 4, 5, 6, 7};
            uint8_t payload[8];
            uint16_t out_size = 0;
            assert(recvbuf != NULL);
            memset(recvbuf, 0xA5, capacity ? capacity : 1);
            isotp_init_link(&link, 0x123, sendbuf, sizeof(sendbuf), recvbuf, capacity);
            isotp_on_can_message(&link, frame, size + 1);
            if (size > capacity) {
                assert(link.receive_protocol_result == ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW);
                assert(link.receive_status == ISOTP_RECEIVE_STATUS_IDLE);
                assert(isotp_receive(&link, payload, sizeof(payload), &out_size) == ISOTP_RET_NO_DATA);
                for (uint16_t i = 0; i < capacity; i++) assert(recvbuf[i] == 0xA5);
                /* A later frame that fits must still be receivable. */
                if (capacity) {
                    frame[0] = 1;
                    isotp_on_can_message(&link, frame, 2);
                    assert(isotp_receive(&link, payload, sizeof(payload), &out_size) == ISOTP_RET_OK);
                    assert(out_size == 1 && payload[0] == 1);
                    assert(link.receive_protocol_result == ISOTP_PROTOCOL_RESULT_OK);
                }
            } else {
                assert(link.receive_status == ISOTP_RECEIVE_STATUS_FULL);
                assert(isotp_receive(&link, payload, sizeof(payload), &out_size) == ISOTP_RET_OK);
                assert(out_size == size);
                assert(memcmp(payload, frame + 1, size) == 0);
            }
            free(recvbuf);
        }
    }
    return 0;
}
