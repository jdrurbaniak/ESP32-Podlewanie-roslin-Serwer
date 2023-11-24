#pragma once
#include "stdint.h"

enum MessageType {PAIRING, DATA,};
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  float humidity;
  unsigned int readingId;
} struct_message;

typedef struct struct_incoming_message {
  uint8_t msgType;
  uint8_t id;
  float humidity;
  unsigned int readingId;
} struct_incoming_message;

typedef struct struct_outgoing_message {
  uint8_t msgType;
  float minimumMoistureLevel;
  float maximumMoistureLevel;
} struct_outgoing_message;

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;