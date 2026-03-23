module Svc {

  passive component LedController {
    @ Command registration port
    command reg port CmdReg

    @ Command received port
    command recv port CmdDisp

    @ Command response port
    command resp port CmdStatus

    @ Event port
    event port Log

    @ Text event port
    text event port LogText

    @ Time get port
    time get port Time

    @ GPIO write port
    output port gpioWrite: Drv.GpioWrite

    @ GPIO read port
    output port gpioRead: Drv.GpioRead

    @ Report the current LED state through an event
    guarded command GET_STATE opcode 0

    @ Set the LED to a specific state
    guarded command SET_STATE(
      @ State to drive to the board LED
      ledState: Fw.On
    ) \
      opcode 1

    @ Toggle the current LED state
    guarded command TOGGLE opcode 2

    @ LED state report
    event LED_STATE(
      ledState: Fw.On
    ) \
      severity activity high id 0 \
      format "LED state={}"

    @ GPIO read failed
    event LED_READ_ERROR(status: Drv.GpioStatus) \
      severity warning high id 1 \
      format "LED read failed: {}"

    @ GPIO write failed
    event LED_WRITE_ERROR(status: Drv.GpioStatus) \
      severity warning high id 2 \
      format "LED write failed: {}"
  }

}
