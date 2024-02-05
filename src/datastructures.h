#pragma once
#include "stdint.h"

enum MessageType {PAIRING, DATA};

typedef struct struct_message {
  uint8_t msgType;
  float humidity;
} struct_message;

typedef struct struct_outgoing_message {
  uint8_t msgType;
  float minimumMoistureLevel;
  float maximumMoistureLevel;
} struct_outgoing_message;

typedef struct struct_pairing {
    uint8_t msgType;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;