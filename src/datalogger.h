#pragma once
#include "stdint.h"
#include "time.h"
#include "FS.h"
#include <sstream>
#include "datastructures.h"
#include "fileoperations.h"

void saveReading(fs::FS &fs, const uint8_t *mac_addr, struct_message &incomingReadings);