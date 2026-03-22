// ======================================================================
// \title  RefTopologyDefs.hpp
// \author mstarch
// \brief required header file containing the required definitions for the topology autocoder
//
// \copyright
// Copyright 2009-2022, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
// ======================================================================
#ifndef REF_REFTOPOLOGYDEFS_HPP
#define REF_REFTOPOLOGYDEFS_HPP

#include "Ref/Top/FppConstantsAc.hpp"

#include "Svc/Subtopologies/CdhCore/SubtopologyTopologyDefs.hpp"
#include "Svc/Subtopologies/ComCcsds/SubtopologyTopologyDefs.hpp"

// Definitions are placed within a namespace named after the deployment
namespace Ref {

/**
 * \brief required type definition to carry state
 *
 * The topology autocoder requires an object that carries state with the name `Ref::TopologyState`. Only the type
 * definition is required by the autocoder and the contents of this object are otherwise opaque to the autocoder. The
 * contents are entirely up to the definition of the project. This reference application specifies hostname and port
 * fields, which are derived by command line inputs.
 */
struct TopologyState {
    const char* hostname;                 //!< Hostname for TCP communication
    U16 port;                             //!< Port for TCP communication
    CdhCore::SubtopologyState cdhCore;    //!< Subtopology state for CdhCore
    ComCcsds::SubtopologyState comCcsds;  //!< Subtopology state for ComCcsds
};
}  // namespace Ref
#endif
