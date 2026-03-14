// ======================================================================
// \title  STM32MinimalTopology.hpp
// \brief  Public topology lifecycle functions
// ======================================================================
#ifndef STM32_DEPLOYMENTS_MINIMALUART_STM32MINIMALTOPOLOGY_HPP
#define STM32_DEPLOYMENTS_MINIMALUART_STM32MINIMALTOPOLOGY_HPP

#include <STM32/Deployments/MinimalUart/Top/STM32MinimalTopologyDefs.hpp>

namespace STM32Minimal {

void setupTopology(const TopologyState& state);
void runMainLoop(const TopologyState& state);
void teardownTopology(const TopologyState& state);

}  // namespace STM32Minimal

#endif
