module ESP32CdhCoreConfig {
    #Base ID for the ESP32CdhCore Subtopology, all components are offsets from this base ID
    constant BASE_ID = 0x01000000
    
    module QueueSizes {
        constant cmdDisp     = 10
        constant events      = 10
        constant tlmSend     = 10
        constant $health     = 25
    }
    
    # Reduced substantially for lower-RAM embedded targets (e.g. ESP32)
    module StackSizes {
        constant cmdDisp     = 8 * 1024
        constant events      = 4 * 1024
        constant tlmSend     = 4 * 1024
    }

    # Reduced to stay under the ESP-IDF priority max (25)
    module Priorities {
        constant cmdDisp     = 24  # 35 => 24
        constant $health     = 23  # 24 => 23
        constant events      = 22  # 23 => 22
        constant tlmSend     = 21  # 22 => 21
    }
}
