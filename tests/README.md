# Host send-length regression test

Run from the repository root:

```sh
cc -std=c99 -Wall -Wextra -Wno-unused-parameter -I. isotp.c tests/test_send_length.c -o /tmp/isotp-send-length
/tmp/isotp-send-length
```

Optional sanitizers: add `-fsanitize=address,undefined -fno-omit-frame-pointer`.

The in-memory CAN shim exercises the real library. It checks single-frame lengths
1 and 7, multi-frame lengths 8 and 4095 (including completion), rejection of 4096
and 65535, rejection through the explicit-ID entry point, and rejection when a
valid protocol length exceeds the caller's send buffer. Rejected calls must not
send a frame, change link state or modify the send buffer. No CAN hardware is used.
