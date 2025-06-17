#include <stdint.h>
#include <inttypes.h>
#include "esp_timer.h"
#include "esp_cpu.h"
#include "esp_log.h"

#ifndef TIMER_TAG
    #define TIMER_TAG "timer"
#endif

#if defined(__GNUC__)
#  define STATIC_INLINE_UNUSED static inline __attribute__((unused))
#else
#  define STATIC_INLINE_UNUSED static inline
#endif


typedef struct {
    int64_t   us;
    uint32_t  cycles;
} timing_t;

// Call to start timing
STATIC_INLINE_UNUSED void timing_start(timing_t *t)
{
    t->us     = esp_timer_get_time();
    t->cycles = esp_cpu_get_cycle_count();
}

// Recieve current delta us and cycles
STATIC_INLINE_UNUSED timing_t timing_elapsed(const timing_t *t)
{
    timing_t d;
    d.us     = esp_timer_get_time()      - t->us;
    d.cycles = esp_cpu_get_cycle_count() - t->cycles;
    return d;
}

// Print current delta us and cycles at log level parameter
STATIC_INLINE_UNUSED void timing_log_level(const timing_t *t, const char   *label, esp_log_level_t level)
{
    timing_t d = timing_elapsed(t);
    esp_log_write(level, "timer", 
        "%s: %" PRId64 "µs (%" PRId64 "ms), %" PRIu32 "cycles",
        label, d.us, d.us/1000, d.cycles);
}

// Print current delta us and cycles at log level error
STATIC_INLINE_UNUSED void timing_log_error(const timing_t *t, const char *label)
{
    timing_log_level(t, label, ESP_LOG_ERROR);
}

// Print current delta us and cycles at log level warning
STATIC_INLINE_UNUSED void timing_log_warn(const timing_t *t, const char *label)
{
    timing_log_level(t, label, ESP_LOG_WARN);
}


// Print current delta us and cycles at log level info
STATIC_INLINE_UNUSED void timing_log_info(const timing_t *t, const char *label)
{
    timing_log_level(t, label, ESP_LOG_INFO);
}

// Print current delta us and cycles at log level debug
STATIC_INLINE_UNUSED void timing_log_debug(const timing_t *t, const char *label)
{
    timing_log_level(t, label, ESP_LOG_DEBUG);
}

// Print current delta us and cycles at log level verbose
STATIC_INLINE_UNUSED void timing_log_verbose(const timing_t *t, const char *label)
{
    timing_log_level(t, label, ESP_LOG_VERBOSE);
}
