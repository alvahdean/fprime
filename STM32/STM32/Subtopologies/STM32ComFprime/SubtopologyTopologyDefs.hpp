#ifndef STM32_STM32COMFPRIME_SUBTOPOLOGY_DEFS_HPP
#define STM32_STM32COMFPRIME_SUBTOPOLOGY_DEFS_HPP

#include <Fw/Types/MallocAllocator.hpp>
#include <Svc/BufferManager/BufferManager.hpp>
#include <Svc/FrameAccumulator/FrameDetector/FprimeFrameDetector.hpp>
#include "STM32ComFprimeConfig/STM32ComFprimeSubtopologyConfig.hpp"
#include "STM32/Subtopologies/STM32ComFprime/STM32ComFprimeConfig/FppConstantsAc.hpp"

namespace STM32ComFprime {
struct SubtopologyState {};

struct TopologyState {
    SubtopologyState comFprime;
};
}  // namespace STM32ComFprime

#endif
