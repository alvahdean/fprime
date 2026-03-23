module Esp32RefUart {
  module Default {
    constant QUEUE_SIZE = 6
    constant STACK_SIZE = 8 * 1024
  }

  instance rateGroup1Comp: Svc.ActiveRateGroup base id 0x20001000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 23

  instance rateGroup2Comp: Svc.ActiveRateGroup base id 0x20002000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 22

  instance rateGroup3Comp: Svc.ActiveRateGroup base id 0x20003000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 21

  instance posixTime: Svc.OsTime base id 0x20004000

  instance rateGroupDriverComp: Svc.RateGroupDriver base id 0x20005000

  instance systemResources: Svc.SystemResources base id 0x20006000

  instance freeRtosTimer: Svc.FreeRtosTimer base id 0x20007000

  instance comDriver: Drv.Esp32UartDriver base id 0x20008000

  instance ledController: Svc.LedController base id 0x20009000

  instance ledGpioDriver: Drv.Esp32GpioDriver base id 0x2000A000
}
