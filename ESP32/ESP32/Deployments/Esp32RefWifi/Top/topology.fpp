module Esp32RefWifi {

  enum Ports_RateGroups {
    rateGroup1
    rateGroup2
  }

  topology Esp32RefWifi {
    import ESP32CdhCore.Subtopology
    import ESP32ComCcsds.Subtopology

    instance posixTime
    instance rateGroup1Comp
    instance rateGroup2Comp
    instance rateGroupDriverComp
    instance systemResources
    instance freeRtosTimer
    instance wifiDriver

    command connections instance ESP32CdhCore.cmdDisp
    event connections instance ESP32CdhCore.events
    text event connections instance ESP32CdhCore.textLogger
    health connections instance ESP32CdhCore.$health
    time connections instance posixTime

    connections RateGroups {
      freeRtosTimer.CycleOut -> rateGroupDriverComp.CycleIn

      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1Comp.CycleIn
      rateGroup1Comp.RateGroupMemberOut[1] -> ESP32CdhCore.cmdDisp.run
      rateGroup1Comp.RateGroupMemberOut[2] -> ESP32ComCcsds.comQueue.run
      rateGroup1Comp.RateGroupMemberOut[4] -> systemResources.run
      rateGroup1Comp.RateGroupMemberOut[5] -> wifiDriver.run

    }

    connections Communications {
      wifiDriver.allocate         -> ESP32ComCcsds.commsBufferManager.bufferGetCallee
      wifiDriver.deallocate       -> ESP32ComCcsds.commsBufferManager.bufferSendIn

      wifiDriver.$recv                            -> ESP32ComCcsds.comStub.drvReceiveIn
      ESP32ComCcsds.comStub.drvReceiveReturnOut  -> wifiDriver.recvReturnIn

      ESP32ComCcsds.comStub.drvSendOut      -> wifiDriver.$send
      wifiDriver.ready                      -> ESP32ComCcsds.comStub.drvConnected
    }

    connections ComCcsds_CdhCore {
      ESP32ComCcsds.fprimeRouter.commandOut -> ESP32CdhCore.cmdDisp.seqCmdBuff
      ESP32CdhCore.cmdDisp.seqCmdStatus     -> ESP32ComCcsds.fprimeRouter.cmdResponseIn
    }
  }
}
