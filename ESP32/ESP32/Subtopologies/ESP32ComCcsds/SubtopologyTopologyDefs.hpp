#ifndef COMCCSDSSUBTOPOLOGY_DEFS_HPP
#define COMCCSDSSUBTOPOLOGY_DEFS_HPP

#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <Svc/FrameAccumulator/FrameDetector/CcsdsTcFrameDetector.hpp>
#include "ESP32ComCcsdsConfig/ESP32ComCcsdsSubtopologyConfig.hpp"
#include "ESP32/Subtopologies/ESP32ComCcsds/ESP32ComCcsdsConfig/FppConstantsAc.hpp"

namespace ESP32ComCcsds {
struct SubtopologyState {
    // Empty - no external state needed for ESP32ComCcsds subtopology
};

struct TopologyState {
    SubtopologyState comCcsds;
};
}  // namespace ESP32ComCcsds

#endif
