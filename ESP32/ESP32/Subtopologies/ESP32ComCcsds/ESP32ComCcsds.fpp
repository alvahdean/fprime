module ESP32ComCcsds {

    # ComPacket Queue enum for queue types
    enum Ports_ComPacketQueue : U8 {
        EVENTS,
        TELEMETRY 
    }

    enum Ports_ComBufferQueue : U8 {
        FILE
    }

    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance comQueue: Svc.ComQueue base id ESP32ComCcsdsConfig.BASE_ID + 0x00000 \
        queue size ESP32ComCcsdsConfig.QueueSizes.comQueue \
        stack size ESP32ComCcsdsConfig.StackSizes.comQueue \
        priority ESP32ComCcsdsConfig.Priorities.comQueue \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        using namespace ESP32ComCcsds;
        Svc::ComQueue::QueueConfigurationTable configurationTable;

        // Events (highest-priority)
        configurationTable.entries[Ports_ComPacketQueue::EVENTS].depth = ESP32ComCcsdsConfig::QueueDepths::events;
        configurationTable.entries[Ports_ComPacketQueue::EVENTS].priority = ESP32ComCcsdsConfig::QueuePriorities::events;

        // Telemetry
        configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].depth = ESP32ComCcsdsConfig::QueueDepths::tlm;
        configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].priority = ESP32ComCcsdsConfig::QueuePriorities::tlm;

        // File Downlink Queue (buffer queue using NUM_CONSTANTS offset)
        configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS + Ports_ComBufferQueue::FILE].depth = ESP32ComCcsdsConfig::QueueDepths::file;
        configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS + Ports_ComBufferQueue::FILE].priority = ESP32ComCcsdsConfig::QueuePriorities::file;

        // Allocation identifier is 0 as the MallocAllocator discards it
        ESP32ComCcsds::comQueue.configure(configurationTable, 0, ESP32ComCcsds::Allocation::memAllocator);
        """
        phase Fpp.ToCpp.Phases.tearDownComponents """
        ESP32ComCcsds::comQueue.cleanup();
        """
    }

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance frameAccumulator: Svc.FrameAccumulator base id ESP32ComCcsdsConfig.BASE_ID + 0x01000 \ 
    {

        phase Fpp.ToCpp.Phases.configObjects """
        Svc::FrameDetectors::CcsdsTcFrameDetector frameDetector;
        """
        phase Fpp.ToCpp.Phases.configComponents """
        ESP32ComCcsds::frameAccumulator.configure(
            ConfigObjects::ESP32ComCcsds_frameAccumulator::frameDetector,
            1,
            ESP32ComCcsds::Allocation::memAllocator,
            ESP32ComCcsdsConfig::BuffMgr::frameAccumulatorSize
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ESP32ComCcsds::frameAccumulator.cleanup();
        """
    }

    instance commsBufferManager: Svc.BufferManager base id ESP32ComCcsdsConfig.BASE_ID + 0x02000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::ESP32ComCcsds_commsBufferManager::bins, 0, sizeof(ConfigObjects::ESP32ComCcsds_commsBufferManager::bins));
        ConfigObjects::ESP32ComCcsds_commsBufferManager::bins.bins[0].bufferSize = ESP32ComCcsdsConfig::BuffMgr::commsBuffSize;
        ConfigObjects::ESP32ComCcsds_commsBufferManager::bins.bins[0].numBuffers = ESP32ComCcsdsConfig::BuffMgr::commsBuffCount;
        ConfigObjects::ESP32ComCcsds_commsBufferManager::bins.bins[1].bufferSize = ESP32ComCcsdsConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::ESP32ComCcsds_commsBufferManager::bins.bins[1].numBuffers = ESP32ComCcsdsConfig::BuffMgr::commsFileBuffCount;
        ESP32ComCcsds::commsBufferManager.setup(
            ESP32ComCcsdsConfig::BuffMgr::commsBuffMgrId,
            0,
            ESP32ComCcsds::Allocation::memAllocator,
            ConfigObjects::ESP32ComCcsds_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        ESP32ComCcsds::commsBufferManager.cleanup();
        """
    }

    instance fprimeRouter: Svc.FprimeRouter base id ESP32ComCcsdsConfig.BASE_ID + 0x03000

    instance tcDeframer: Svc.Ccsds.TcDeframer base id ESP32ComCcsdsConfig.BASE_ID + 0x04000

    instance spacePacketDeframer: Svc.Ccsds.SpacePacketDeframer base id ESP32ComCcsdsConfig.BASE_ID + 0x05000

    instance aggregator: Svc.ComAggregator base id ESP32ComCcsdsConfig.BASE_ID + 0x06000 \
        queue size ESP32ComCcsdsConfig.QueueSizes.aggregator \
        stack size ESP32ComCcsdsConfig.StackSizes.aggregator \
        priority ESP32ComCcsdsConfig.Priorities.aggregator

    # NOTE: name 'framer' is used for the framer that connects to the Com Adapter Interface for better subtopology interoperability
    instance framer: Svc.Ccsds.TmFramer base id ESP32ComCcsdsConfig.BASE_ID + 0x07000

    instance spacePacketFramer: Svc.Ccsds.SpacePacketFramer base id ESP32ComCcsdsConfig.BASE_ID + 0x08000

    instance apidManager: Svc.Ccsds.ApidManager base id ESP32ComCcsdsConfig.BASE_ID + 0x09000

    instance comStub: Svc.ComStub base id ESP32ComCcsdsConfig.BASE_ID + 0x0A000

    topology FramingSubtopology {
        # Usage Note:
        #
        # When importing this subtopology, users shall establish 5 port connections with a component implementing
        # the Svc.Com (Svc/Interfaces/Com.fpp) interface. They are as follows:
        #
        # 1) Outputs:
        #     - ESP32ComCcsds.framer.dataOut                 -> [Svc.Com].dataIn
        #     - ESP32ComCcsds.frameAccumulator.dataReturnOut -> [Svc.Com].dataReturnIn
        # 2) Inputs:
        #     - [Svc.Com].dataReturnOut -> ESP32ComCcsds.framer.dataReturnIn
        #     - [Svc.Com].comStatusOut  -> ESP32ComCcsds.framer.comStatusIn
        #     - [Svc.Com].dataOut       -> ESP32ComCcsds.frameAccumulator.dataIn


        # Active Components
        instance comQueue

        # Passive Components
        instance commsBufferManager
        instance frameAccumulator
        instance fprimeRouter
        instance tcDeframer
        instance spacePacketDeframer
        instance framer
        instance spacePacketFramer
        instance apidManager

        connections Downlink {
            # ComQueue <-> SpacePacketFramer
            comQueue.dataOut                -> spacePacketFramer.dataIn
            spacePacketFramer.dataReturnOut -> comQueue.dataReturnIn
            # SpacePacketFramer buffer and APID management
            spacePacketFramer.bufferAllocate   -> commsBufferManager.bufferGetCallee
            spacePacketFramer.bufferDeallocate -> commsBufferManager.bufferSendIn
            spacePacketFramer.getApidSeqCount  -> apidManager.getApidSeqCountIn
            # SpacePacketFramer <-> TmFramer
            spacePacketFramer.dataOut   -> framer.dataIn
            framer.dataReturnOut        -> spacePacketFramer.dataReturnIn

            # ComStatus
            framer.comStatusOut            -> spacePacketFramer.comStatusIn
            spacePacketFramer.comStatusOut -> comQueue.comStatusIn
            # (Outgoing) Framer <-> ComInterface connections shall be established by the user
        }

        connections Uplink {
            # (Incoming) ComInterface <-> FrameAccumulator connections shall be established by the user
            # FrameAccumulator buffer allocations
            frameAccumulator.bufferDeallocate -> commsBufferManager.bufferSendIn
            frameAccumulator.bufferAllocate   -> commsBufferManager.bufferGetCallee
            # FrameAccumulator <-> TcDeframer
            frameAccumulator.dataOut -> tcDeframer.dataIn
            tcDeframer.dataReturnOut -> frameAccumulator.dataReturnIn
            # TcDeframer <-> SpacePacketDeframer
            tcDeframer.dataOut                -> spacePacketDeframer.dataIn
            spacePacketDeframer.dataReturnOut -> tcDeframer.dataReturnIn
            # SpacePacketDeframer APID validation
            spacePacketDeframer.validateApidSeqCount -> apidManager.validateApidSeqCountIn
            # SpacePacketDeframer <-> Router
            spacePacketDeframer.dataOut -> fprimeRouter.dataIn
            fprimeRouter.dataReturnOut  -> spacePacketDeframer.dataReturnIn
            # Router buffer allocations
            fprimeRouter.bufferAllocate   -> commsBufferManager.bufferGetCallee
            fprimeRouter.bufferDeallocate -> commsBufferManager.bufferSendIn
        }
    } # end FramingSubtopology

    # This subtopology uses FramingSubtopology with a ComStub component for Com Interface
    topology Subtopology {
        import FramingSubtopology

        instance comStub

        connections ComStub {
            # Framer <-> ComStub (Downlink)
            ESP32ComCcsds.framer.dataOut -> comStub.dataIn
            comStub.dataReturnOut   -> ESP32ComCcsds.framer.dataReturnIn
            comStub.comStatusOut    -> ESP32ComCcsds.framer.comStatusIn

            # ComStub <-> FrameAccumulator (Uplink)
            comStub.dataOut -> ESP32ComCcsds.frameAccumulator.dataIn
            ESP32ComCcsds.frameAccumulator.dataReturnOut -> comStub.dataReturnIn
        }
    } # end Subtopology

} # end ESP32ComCcsds
