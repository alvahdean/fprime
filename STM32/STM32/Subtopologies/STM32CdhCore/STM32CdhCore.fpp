module STM32CdhCore {
    instance cmdDisp: Svc.CommandDispatcher base id STM32CdhCoreConfig.BASE_ID + 0x00000 \
        queue size STM32CdhCoreConfig.QueueSizes.cmdDisp \
        stack size STM32CdhCoreConfig.StackSizes.cmdDisp \
        priority STM32CdhCoreConfig.Priorities.cmdDisp

    instance events: Svc.EventManager base id STM32CdhCoreConfig.BASE_ID + 0x001000 \
        queue size STM32CdhCoreConfig.QueueSizes.events \
        stack size STM32CdhCoreConfig.StackSizes.events \
        priority STM32CdhCoreConfig.Priorities.events

    instance $health: Svc.Health base id STM32CdhCoreConfig.BASE_ID + 0x002000 \
        queue size STM32CdhCoreConfig.QueueSizes.$health \
    {
        phase Fpp.ToCpp.Phases.configConstants """
        enum {
            HEALTH_WATCHDOG_CODE = 0x123
        };
        """
        phase Fpp.ToCpp.Phases.configComponents """
        STM32CdhCore::health.setPingEntries(
            ConfigObjects::STM32CdhCore_health::pingEntries,
            FW_NUM_ARRAY_ELEMENTS(ConfigObjects::STM32CdhCore_health::pingEntries),
            ConfigConstants::STM32CdhCore_health::HEALTH_WATCHDOG_CODE
        );
        """
    }

    instance version: Svc.Version base id STM32CdhCoreConfig.BASE_ID + 0x003000 \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        STM32CdhCore::version.config(true);
        """
    }

    instance textLogger: Svc.PassiveTextLogger base id STM32CdhCoreConfig.BASE_ID + 0x004000

    instance fatalAdapter: Svc.AssertFatalAdapter base id STM32CdhCoreConfig.BASE_ID + 0x005000

    instance tlmSend: Svc.TlmChan base id STM32CdhCoreConfig.BASE_ID + 0x06000 \
        queue size STM32CdhCoreConfig.QueueSizes.tlmSend \
        stack size STM32CdhCoreConfig.StackSizes.tlmSend \
        priority STM32CdhCoreConfig.Priorities.tlmSend

    instance fatalHandler: Svc.FatalHandler base id STM32CdhCoreConfig.BASE_ID + 0x07000

    topology Subtopology {
        instance cmdDisp
        instance events
        instance tlmSend
        instance $health
        instance version
        instance textLogger
        instance fatalAdapter
        instance fatalHandler

        connections FaultProtection {
            events.FatalAnnounce -> fatalHandler.FatalReceive
        }
    }
}
