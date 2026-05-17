#pragma once

#include <windows.h>
#include "types.h"

#define PIPE_NAME TEXT("\\\\.\\pipe\\employee_pipe")
#define BUFFER_SIZE 512
#define RETRY_DELAY_MS 3000
#define RETRY_DELAY_SEC (RETRY_DELAY_MS / 1000)
