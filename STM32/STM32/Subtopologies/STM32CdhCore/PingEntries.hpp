#ifndef STM32_STM32CDHCORE_PINGENTRIES_HPP
#define STM32_STM32CDHCORE_PINGENTRIES_HPP

namespace PingEntries {
struct STM32CdhCore_cmdDisp {
    enum { WARN = 3, FATAL = 5 };
};
struct STM32CdhCore_events {
    enum { WARN = 3, FATAL = 5 };
};
struct STM32CdhCore_tlmSend {
    enum { WARN = 3, FATAL = 5 };
};
}  // namespace PingEntries

#endif
