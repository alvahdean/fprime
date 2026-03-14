module Drv {
  passive component Esp32UartDriver {
    import ByteStreamDriver

    sync input port run: Svc.Sched

    output port allocate: Fw.BufferGet
    output port deallocate: Fw.BufferSend
  }
}
