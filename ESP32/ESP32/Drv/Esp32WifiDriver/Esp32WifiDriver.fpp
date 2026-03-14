module Drv {
  passive component Esp32WifiDriver {
    import ByteStreamDriver

    sync input port run: Svc.Sched

    output port allocate: Fw.BufferGet
    output port deallocate: Fw.BufferSend
  }
}
