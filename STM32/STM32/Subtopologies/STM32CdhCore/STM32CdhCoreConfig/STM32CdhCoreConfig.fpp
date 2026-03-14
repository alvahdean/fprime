module STM32CdhCoreConfig {
    constant BASE_ID = 0x01000000

    module QueueSizes {
        constant cmdDisp = 10
        constant events = 10
        constant tlmSend = 10
        constant $health = 10
    }

    module StackSizes {
        constant cmdDisp = 2048
        constant events = 2048
        constant tlmSend = 2048
    }

    module Priorities {
        constant cmdDisp = 35
        constant $health = 24
        constant events = 23
        constant tlmSend = 22
    }
}
