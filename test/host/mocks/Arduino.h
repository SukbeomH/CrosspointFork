#pragma once
// ---------------------------------------------------------------
// Arduino.h mock for host-based unit tests.
// Provides millis(), delay(), String, and Serial stubs.
// ---------------------------------------------------------------

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

// ---- millis / delay -------------------------------------------

// Controllable fake clock — tests can advance time via setMillis().
void setMillis(unsigned long ms);
unsigned long millis();
inline void delay(unsigned long /*ms*/) {}

// ---- Arduino String class (minimal mock) ----------------------

class String {
 public:
  String() = default;
  String(const char* s) : data_(s ? s : "") {}
  String(const std::string& s) : data_(s) {}

  const char* c_str() const { return data_.c_str(); }
  size_t length() const { return data_.size(); }
  bool isEmpty() const { return data_.empty(); }

  String& operator+=(const char* s) {
    data_ += s;
    return *this;
  }
  String& operator+=(const String& s) {
    data_ += s.data_;
    return *this;
  }
  friend String operator+(const String& a, const String& b) { return String(a.data_ + b.data_); }

  bool operator==(const String& o) const { return data_ == o.data_; }
  bool operator==(const char* o) const { return data_ == (o ? o : ""); }
  bool operator!=(const String& o) const { return !(*this == o); }

  char operator[](unsigned int i) const { return data_[i]; }

 private:
  std::string data_;
};

// ---- Print base class (minimal) --------------------------------

class Print {
 public:
  virtual ~Print() = default;
  virtual size_t write(uint8_t b) {
    (void)b;
    return 1;
  }
  virtual size_t write(const uint8_t* buf, size_t len) {
    (void)buf;
    return len;
  }
  virtual void flush() {}
  size_t printf(const char* fmt, ...) {
    (void)fmt;
    return 0;
  }
};

// ---- HardwareSerial / Serial stub ------------------------------

class HardwareSerial : public Print {
 public:
  void begin(unsigned long /*baud*/) {}
  operator bool() const { return true; }
  size_t write(uint8_t b) override {
    (void)b;
    return 1;
  }
};

// Using HWCDC = HardwareSerial so Logging.h compiles.
using HWCDC = HardwareSerial;

extern HardwareSerial Serial;

// ---- Misc typedefs commonly assumed by ESP-IDF headers ---------
using gpio_num_t = int;
