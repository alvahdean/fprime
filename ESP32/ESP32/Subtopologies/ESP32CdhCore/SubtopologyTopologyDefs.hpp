#ifndef CDHCORESUBTOPOLOGY_DEFS_HPP
#define CDHCORESUBTOPOLOGY_DEFS_HPP

#include "ESP32/Subtopologies/ESP32CdhCore/ESP32CdhCoreConfig/FppConstantsAc.hpp"

namespace ESP32CdhCore {
// State for topology construction
struct SubtopologyState {
    // Empty - no external state needed for ESP32CdhCore subtopology
};

struct TopologyState {
    SubtopologyState cdhCore;
};
}  // namespace ESP32CdhCore

#endif
