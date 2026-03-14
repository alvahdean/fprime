module STM32Minimal {

  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 4096
  }

  # ----------------------------------------------------------------------
  # Active components
  # ----------------------------------------------------------------------

  instance rateGroup1Comp: Svc.ActiveRateGroup base id 0x20000000 \
    queue size Default.QUEUE_SIZE \
    stack size Default.STACK_SIZE \
    priority 45

  # ----------------------------------------------------------------------
  # Passive components
  # ----------------------------------------------------------------------

  instance rateGroupDriverComp: Svc.RateGroupDriver base id 0x20001000

  instance osTime: Svc.OsTime base id 0x20002000

  instance uartDriver: Drv.STM32UartDriver base id 0x20003000

}
