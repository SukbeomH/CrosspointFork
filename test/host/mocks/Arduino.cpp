#include "Arduino.h"

// Fake clock for host tests
static unsigned long fakeMillis = 0;

void setMillis(unsigned long ms) { fakeMillis = ms; }
unsigned long millis() { return fakeMillis; }

// Global Serial instance (unused in tests but needed for link)
HardwareSerial Serial;
