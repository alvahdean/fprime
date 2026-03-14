module Drv {

  @ STM32 UART byte-stream driver skeleton for FreeRTOS targets.
  passive component STM32UartDriver {

    import ByteStreamDriver

    @ Allocation port used for receive-path buffer emission.
    output port allocate: Fw.BufferGet

    @ Deallocation of buffers returned from downstream components.
    output port deallocate: Fw.BufferSend

    @ Rate group input used to process deferred ISR work.
    sync input port run: Svc.Sched
  }

}
