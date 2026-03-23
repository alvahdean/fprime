module ESP32ComCcsdsConfig {
    #Base ID for the ESP32ComCcsds Subtopology, all components are offsets from this base ID
    constant BASE_ID = 0x02000000
    
    module QueueSizes {
        constant comQueue    = 20
        constant aggregator  = 8
    }
    
    # Reduced substantially for lower-RAM embedded targets (e.g. ESP32)
    module StackSizes {
        constant comQueue     = 10 * 1024
        constant aggregator   = 4 * 1024
    }

    module Priorities {
        constant aggregator = 20
        constant comQueue   = 19
    }

    # Queue configuration constants
    # Reduced substantially for lower-RAM embedded targets (e.g. ESP32)
    module QueueDepths {
        constant events      = 16
        constant tlm         = 12
        constant file        = 1
    }

    module QueuePriorities {
        constant events      = 0                 
        constant tlm         = 0                 
        constant file        = 1                   
    }

    # Buffer management constants
    # Reduced substantially for lower-RAM embedded targets (e.g. ESP32)
    module BuffMgr {
        constant frameAccumulatorSize  = 1024  # 2048 => 1024
        constant commsBuffSize         = 1024  # Must satisfy the ESP32 UDP RX allocation size
        constant commsFileBuffSize     = 1024  # 3000 => 1024
        constant commsBuffCount        = 2
        constant commsFileBuffCount    = 1
        constant commsBuffMgrId        = 200      
    }
}
