module Ref {

  module Default {
    constant QUEUE_SIZE = 10
    constant STACK_SIZE = 64 * 1024
  }

  instance comDriver: Drv.TcpClient base id 0x10025000

}
