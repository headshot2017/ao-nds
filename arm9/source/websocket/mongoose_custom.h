#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

// See https://mongoose.ws/documentation/#build-options
#define MG_ARCH MG_ARCH_CUSTOM
#define MG_ENABLE_TCPIP 1
#define MG_ENABLE_FILE 0
#define MG_IO_SIZE 256
#define MG_ENABLE_SSI 0
