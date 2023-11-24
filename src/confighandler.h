#pragma once
#include "stdint.h"
#include "fileoperations.h"
#include "datastructures.h"
#include <sstream>
#include <ArduinoJson.h>

struct_outgoing_message readDataToSend(fs::FS &fs, const uint8_t * mac_addr);