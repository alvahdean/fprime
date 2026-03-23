module Esp32RefUart {

  enum Ports_RateGroups {
    rateGroup1
    rateGroup2
    rateGroup3
  }

  topology Esp32RefUart {
    import ESP32CdhCore.Subtopology
    import ESP32ComCcsds.Subtopology

    instance posixTime
    instance rateGroup1Comp
    instance rateGroup2Comp
    instance rateGroup3Comp
    instance rateGroupDriverComp
    instance systemResources
    instance freeRtosTimer
    instance comDriver
    instance ledController
    instance ledGpioDriver

    command connections instance ESP32CdhCore.cmdDisp
    event connections instance ESP32CdhCore.events
    telemetry connections instance ESP32CdhCore.tlmSend
    text event connections instance ESP32CdhCore.textLogger
    health connections instance ESP32CdhCore.$health
    time connections instance posixTime

    connections RateGroups {
      freeRtosTimer.CycleOut -> rateGroupDriverComp.CycleIn

      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1Comp.CycleIn
      rateGroup1Comp.RateGroupMemberOut[0] -> ESP32CdhCore.tlmSend.Run
      rateGroup1Comp.RateGroupMemberOut[1] -> ESP32CdhCore.cmdDisp.run
      rateGroup1Comp.RateGroupMemberOut[2] -> ESP32ComCcsds.comQueue.run
      rateGroup1Comp.RateGroupMemberOut[4] -> systemResources.run
      rateGroup1Comp.RateGroupMemberOut[5] -> comDriver.run

      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup2] -> rateGroup2Comp.CycleIn

      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup3] -> rateGroup3Comp.CycleIn
      rateGroup3Comp.RateGroupMemberOut[0] -> ESP32CdhCore.$health.Run
    }

    connections Communications {
      comDriver.allocate         -> ESP32ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate       -> ESP32ComCcsds.commsBufferManager.bufferSendIn

      comDriver.$recv                             -> ESP32ComCcsds.comStub.drvReceiveIn
      ESP32ComCcsds.comStub.drvReceiveReturnOut  -> comDriver.recvReturnIn

      ESP32ComCcsds.comStub.drvSendOut      -> comDriver.$send
      comDriver.ready                       -> ESP32ComCcsds.comStub.drvConnected
    }

    connections Led {
      ledController.gpioWrite -> ledGpioDriver.gpioWrite
      ledController.gpioRead  -> ledGpioDriver.gpioRead
    }

    connections ComCcsds_CdhCore {
      ESP32CdhCore.events.PktSend     -> ESP32ComCcsds.comQueue.comPacketQueueIn[ESP32ComCcsds.Ports_ComPacketQueue.EVENTS]
      ESP32CdhCore.tlmSend.PktSend    -> ESP32ComCcsds.comQueue.comPacketQueueIn[ESP32ComCcsds.Ports_ComPacketQueue.TELEMETRY]

      ESP32ComCcsds.fprimeRouter.commandOut -> ESP32CdhCore.cmdDisp.seqCmdBuff
      ESP32CdhCore.cmdDisp.seqCmdStatus     -> ESP32ComCcsds.fprimeRouter.cmdResponseIn
    }

    connections Commands {
      ledController.CmdReg                  -> ESP32CdhCore.cmdDisp.compCmdReg[7]
      ESP32CdhCore.cmdDisp.compCmdSend[7]   -> ledController.CmdDisp
      ledController.CmdStatus               -> ESP32CdhCore.cmdDisp.compCmdStat
    }

    connections Events {
      ledController.Log     -> ESP32CdhCore.events.LogRecv
      ledController.LogText -> ESP32CdhCore.textLogger.TextLogger
    }
  }
}
