module STM32ComFprime {

    enum Ports_ComPacketQueue : U8 {
        EVENTS,
        TELEMETRY,
    };

    enum Ports_ComBufferQueue : U8 {
        FILE
    };

    instance comQueue: Svc.ComQueue base id STM32ComFprimeConfig.BASE_ID + 0x00000 \
        queue size STM32ComFprimeConfig.QueueSizes.comQueue \
        stack size STM32ComFprimeConfig.StackSizes.comQueue \
        priority STM32ComFprimeConfig.Priorities.comQueue \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        using namespace STM32ComFprime;
        Svc::ComQueue::QueueConfigurationTable configurationTable;

        configurationTable.entries[Ports_ComPacketQueue::EVENTS].depth = STM32ComFprimeConfig::QueueDepths::events;
        configurationTable.entries[Ports_ComPacketQueue::EVENTS].priority = STM32ComFprimeConfig::QueuePriorities::events;
        configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].depth = STM32ComFprimeConfig::QueueDepths::tlm;
        configurationTable.entries[Ports_ComPacketQueue::TELEMETRY].priority = STM32ComFprimeConfig::QueuePriorities::tlm;
        configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS + Ports_ComBufferQueue::FILE].depth = STM32ComFprimeConfig::QueueDepths::file;
        configurationTable.entries[Ports_ComPacketQueue::NUM_CONSTANTS + Ports_ComBufferQueue::FILE].priority = STM32ComFprimeConfig::QueuePriorities::file;
        STM32ComFprime::comQueue.configure(configurationTable, 0, STM32ComFprime::Allocation::memAllocator);
        """
        phase Fpp.ToCpp.Phases.tearDownComponents """
        STM32ComFprime::comQueue.cleanup();
        """
    }

    instance frameAccumulator: Svc.FrameAccumulator base id STM32ComFprimeConfig.BASE_ID + 0x01000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::FrameDetectors::FprimeFrameDetector frameDetector;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        STM32ComFprime::frameAccumulator.configure(
            ConfigObjects::STM32ComFprime_frameAccumulator::frameDetector,
            1,
            STM32ComFprime::Allocation::memAllocator,
            STM32ComFprimeConfig::BuffMgr::frameAccumulatorSize
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        STM32ComFprime::frameAccumulator.cleanup();
        """
    }

    instance commsBufferManager: Svc.BufferManager base id STM32ComFprimeConfig.BASE_ID + 0x02000 \
    {
        phase Fpp.ToCpp.Phases.configObjects """
        Svc::BufferManager::BufferBins bins;
        """

        phase Fpp.ToCpp.Phases.configComponents """
        memset(&ConfigObjects::STM32ComFprime_commsBufferManager::bins, 0, sizeof(ConfigObjects::STM32ComFprime_commsBufferManager::bins));
        ConfigObjects::STM32ComFprime_commsBufferManager::bins.bins[0].bufferSize = STM32ComFprimeConfig::BuffMgr::commsBuffSize;
        ConfigObjects::STM32ComFprime_commsBufferManager::bins.bins[0].numBuffers = STM32ComFprimeConfig::BuffMgr::commsBuffCount;
        ConfigObjects::STM32ComFprime_commsBufferManager::bins.bins[1].bufferSize = STM32ComFprimeConfig::BuffMgr::commsFileBuffSize;
        ConfigObjects::STM32ComFprime_commsBufferManager::bins.bins[1].numBuffers = STM32ComFprimeConfig::BuffMgr::commsFileBuffCount;
        STM32ComFprime::commsBufferManager.setup(
            STM32ComFprimeConfig::BuffMgr::commsBuffMgrId,
            0,
            STM32ComFprime::Allocation::memAllocator,
            ConfigObjects::STM32ComFprime_commsBufferManager::bins
        );
        """

        phase Fpp.ToCpp.Phases.tearDownComponents """
        STM32ComFprime::commsBufferManager.cleanup();
        """
    }

    instance deframer: Svc.FprimeDeframer base id STM32ComFprimeConfig.BASE_ID + 0x03000
    instance framer: Svc.FprimeFramer base id STM32ComFprimeConfig.BASE_ID + 0x04000
    instance fprimeRouter: Svc.FprimeRouter base id STM32ComFprimeConfig.BASE_ID + 0x05000
    instance comStub: Svc.ComStub base id STM32ComFprimeConfig.BASE_ID + 0x06000

    topology FramingSubtopology {
        instance comQueue
        instance commsBufferManager
        instance frameAccumulator
        instance deframer
        instance framer
        instance fprimeRouter

        connections Downlink {
            comQueue.dataOut -> framer.dataIn
            framer.dataReturnOut -> comQueue.dataReturnIn
            framer.bufferAllocate -> commsBufferManager.bufferGetCallee
            framer.bufferDeallocate -> commsBufferManager.bufferSendIn
            framer.comStatusOut -> comQueue.comStatusIn
        }

        connections Uplink {
            frameAccumulator.bufferDeallocate -> commsBufferManager.bufferSendIn
            frameAccumulator.bufferAllocate -> commsBufferManager.bufferGetCallee
            frameAccumulator.dataOut -> deframer.dataIn
            deframer.dataReturnOut -> frameAccumulator.dataReturnIn
            deframer.dataOut -> fprimeRouter.dataIn
            fprimeRouter.dataReturnOut -> deframer.dataReturnIn
            fprimeRouter.bufferAllocate -> commsBufferManager.bufferGetCallee
            fprimeRouter.bufferDeallocate -> commsBufferManager.bufferSendIn
        }
    }

    topology Subtopology {
        import FramingSubtopology
        instance comStub

        connections ComStub {
            STM32ComFprime.framer.dataOut -> comStub.dataIn
            comStub.dataReturnOut -> STM32ComFprime.framer.dataReturnIn
            comStub.comStatusOut -> STM32ComFprime.framer.comStatusIn
            comStub.dataOut -> STM32ComFprime.frameAccumulator.dataIn
            STM32ComFprime.frameAccumulator.dataReturnOut -> comStub.dataReturnIn
        }
    }
}
