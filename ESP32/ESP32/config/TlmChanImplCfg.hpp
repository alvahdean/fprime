/**
 * \file
 * \brief ESP32-specific telemetry channel sizing overrides.
 *
 * These values intentionally reduce static RAM usage for ESP32-WROOM-32.
 */

#ifndef TLMCHANIMPLCFG_HPP_
#define TLMCHANIMPLCFG_HPP_

namespace {

enum {
    // Keep slot count close to component count producing telemetry.
    TLMCHAN_NUM_TLM_HASH_SLOTS = 15,
    // Spread IDs across hash slots similarly to framework defaults.
    TLMCHAN_HASH_MOD_VALUE = 99,
    // ESP32 DRAM is limited; size for current Esp32RefUart telemetry set.
    TLMCHAN_HASH_BUCKETS = 64
};

}

#endif  // TLMCHANIMPLCFG_HPP_
