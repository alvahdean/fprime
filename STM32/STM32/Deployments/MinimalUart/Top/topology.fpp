module STM32Minimal {

  enum Ports_RateGroups {
    rateGroup1
  }

  topology STM32Minimal {
    import STM32CdhCore.Subtopology
    import STM32ComFprime.Subtopology

    instance rateGroup1Comp
    instance rateGroupDriverComp
    instance osTime
    instance uartDriver

    command connections instance STM32CdhCore.cmdDisp
    event connections instance STM32CdhCore.events
    telemetry connections instance STM32CdhCore.tlmSend
    text event connections instance STM32CdhCore.textLogger
    health connections instance STM32CdhCore.$health
    time connections instance osTime

    connections RateGroups {
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1Comp.CycleIn

      rateGroup1Comp.RateGroupMemberOut[0] -> STM32CdhCore.tlmSend.Run
      rateGroup1Comp.RateGroupMemberOut[1] -> STM32CdhCore.$health.Run
      rateGroup1Comp.RateGroupMemberOut[2] -> STM32ComFprime.comQueue.run
      rateGroup1Comp.RateGroupMemberOut[3] -> uartDriver.run
    }

    connections Communications {
      uartDriver.allocate                     -> STM32ComFprime.commsBufferManager.bufferGetCallee
      uartDriver.deallocate                   -> STM32ComFprime.commsBufferManager.bufferSendIn
      uartDriver.$recv                        -> STM32ComFprime.comStub.drvReceiveIn
      STM32ComFprime.comStub.drvReceiveReturnOut   -> uartDriver.recvReturnIn
      STM32ComFprime.comStub.drvSendOut            -> uartDriver.$send
      uartDriver.ready                        -> STM32ComFprime.comStub.drvConnected
    }

    connections ComFprime_CdhCore {
      STM32CdhCore.events.PktSend                  -> STM32ComFprime.comQueue.comPacketQueueIn[STM32ComFprime.Ports_ComPacketQueue.EVENTS]
      STM32CdhCore.tlmSend.PktSend                 -> STM32ComFprime.comQueue.comPacketQueueIn[STM32ComFprime.Ports_ComPacketQueue.TELEMETRY]

      STM32ComFprime.fprimeRouter.commandOut       -> STM32CdhCore.cmdDisp.seqCmdBuff
      STM32CdhCore.cmdDisp.seqCmdStatus            -> STM32ComFprime.fprimeRouter.cmdResponseIn
    }
  }

}
