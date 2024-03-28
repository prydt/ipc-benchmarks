#include "ipc_runner.h"

void check(int ret, const char *errormsg) {
    if (ret != 0) {
        fprintf(stderr, "ERROR %s, ERR: %s\n", errormsg, strerror(errno));
        exit(-1);
    }
}

char int_to_byte(int value, int index) {
    return (value >> (8 * index)) & 0xFF;
}

int byte_to_int(int value, int index) {
    return value << 8 * index;
}