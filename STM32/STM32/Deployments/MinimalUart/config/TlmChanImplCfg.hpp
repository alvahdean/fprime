#ifndef TLMCHANIMPLCFG_HPP_
#define TLMCHANIMPLCFG_HPP_

// STM32F405 has limited SRAM, so keep the telemetry hash table small for the
// MinimalUart deployment. This deployment only exposes a small telemetry set.
namespace {

enum {
    TLMCHAN_NUM_TLM_HASH_SLOTS = 8,
    TLMCHAN_HASH_MOD_VALUE = 31,
    TLMCHAN_HASH_BUCKETS = 32
};

}  // namespace

#endif
