module ESP32CdhCore {
    # ----------------------------------------------------------------------
    # Active Components
    # ----------------------------------------------------------------------
    instance cmdDisp: Svc.CommandDispatcher base id ESP32CdhCoreConfig.BASE_ID + 0x00000 \
        queue size ESP32CdhCoreConfig.QueueSizes.cmdDisp \
        stack size ESP32CdhCoreConfig.StackSizes.cmdDisp \
        priority ESP32CdhCoreConfig.Priorities.cmdDisp

    instance events: Svc.EventManager base id ESP32CdhCoreConfig.BASE_ID + 0x001000 \
        queue size ESP32CdhCoreConfig.QueueSizes.events \
        stack size ESP32CdhCoreConfig.StackSizes.events \
        priority ESP32CdhCoreConfig.Priorities.events

    # ----------------------------------------------------------------------
    # Queued Components
    # ----------------------------------------------------------------------
    instance $health: Svc.Health base id ESP32CdhCoreConfig.BASE_ID + 0x002000 \
        queue size ESP32CdhCoreConfig.QueueSizes.$health \
    {
        phase Fpp.ToCpp.Phases.configConstants """
        enum {
            HEALTH_WATCHDOG_CODE = 0x123
        };
        """
        phase Fpp.ToCpp.Phases.configComponents """
        // Health is supplied a set of ping entires.
        ESP32CdhCore::health.setPingEntries(
            ConfigObjects::ESP32CdhCore_health::pingEntries,
            FW_NUM_ARRAY_ELEMENTS(ConfigObjects::ESP32CdhCore_health::pingEntries),
            ConfigConstants::ESP32CdhCore_health::HEALTH_WATCHDOG_CODE
        );
        """
    }

    # ----------------------------------------------------------------------
    # Passive Components
    # ----------------------------------------------------------------------
    instance version: Svc.Version base id ESP32CdhCoreConfig.BASE_ID + 0x003000 \
    {
        phase Fpp.ToCpp.Phases.configComponents """
        // Startup TLM and Config verbosity for Versions
        ESP32CdhCore::version.config(true);
        """
    }

    instance textLogger: Svc.PassiveTextLogger base id ESP32CdhCoreConfig.BASE_ID + 0x004000

    instance fatalAdapter: Svc.AssertFatalAdapter base id ESP32CdhCoreConfig.BASE_ID + 0x005000

    topology Subtopology {
        #Active Components
        instance cmdDisp
        instance events
        instance tlmSend

        #Queued Components
        instance $health

        #Passive Components
        instance version
        instance textLogger
        instance fatalAdapter
        instance fatalHandler

        connections FaultProtection {
            events.FatalAnnounce -> fatalHandler.FatalReceive
        }
        
    } # end topology
} # end ESP32CdhCore Subtopology
