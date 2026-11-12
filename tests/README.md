Run the single-frame receive-capacity regression test from the repository root:

```sh
cc -std=c99 -g -fsanitize=address,undefined -I. \
    isotp.c tests/single_frame_capacity.c -o /tmp/isotp-capacity-test
/tmp/isotp-capacity-test
```

The test exercises payload sizes 1–7 and receive-buffer capacities 0–8. It
checks rejection without buffer modification, exact-fit and larger-buffer
reception, and recovery after an oversized frame. The CAN and clock callbacks
are simulated, so no CAN hardware is required.
