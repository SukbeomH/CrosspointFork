#pragma once
// ---------------------------------------------------------------
// Logging.h mock — all log macros expand to no-ops.
// ---------------------------------------------------------------

#include <cstdio>
#include <string>

#define LOG_DBG(origin, format, ...)
#define LOG_ERR(origin, format, ...)
#define LOG_INF(origin, format, ...)

inline std::string getLastLogs() { return ""; }
inline void clearLastLogs() {}
inline bool sanitizeLogHead() { return false; }
