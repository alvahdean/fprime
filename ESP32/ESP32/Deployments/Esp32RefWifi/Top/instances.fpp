module Esp32RefWifi {
  module Default {
    constant QUEUE_SIZE = 6
    constant STACK_SIZE = 4096
  }

  instance rateGroup1Comp: Svc.ActiveRateGroup base id 0x21001000 \
    queue size Default.QUEUE_SIZE \
    stack size 6144 \
    priority 20

  instance rateGroup2Comp: Svc.ActiveRateGroup base id 0x21002000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 19

  instance rateGroup3Comp: Svc.ActiveRateGroup base id 0x21003000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 18

  instance posixTime: Svc.OsTime base id 0x21004000

  instance rateGroupDriverComp: Svc.RateGroupDriver base id 0x21005000

  instance systemResources: Svc.SystemResources base id 0x21006000

  instance freeRtosTimer: Svc.FreeRtosTimer base id 0x21007000

  instance wifiDriver: Drv.Esp32WifiDriver base id 0x21008000
}
