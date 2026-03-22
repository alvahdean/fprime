module Ref {
  topology Ref {
    import ComCcsds.Subtopology

    instance CdhCore.cmdDisp
    instance comDriver

    command connections instance CdhCore.cmdDisp

    connections Communications {
      # ComDriver buffer allocations
      comDriver.allocate      -> ComCcsds.commsBufferManager.bufferGetCallee
      comDriver.deallocate    -> ComCcsds.commsBufferManager.bufferSendIn
      
      # ComDriver <-> ComStub (Uplink)
      comDriver.$recv                     -> ComCcsds.comStub.drvReceiveIn
      ComCcsds.comStub.drvReceiveReturnOut -> comDriver.recvReturnIn
      
      # ComStub <-> ComDriver (Downlink)
      ComCcsds.comStub.drvSendOut      -> comDriver.$send
      comDriver.ready         -> ComCcsds.comStub.drvConnected
    }

    connections ComCcsds_CdhCore {
      # Router <-> CmdDispatcher
      ComCcsds.fprimeRouter.commandOut  -> CdhCore.cmdDisp.seqCmdBuff
      CdhCore.cmdDisp.seqCmdStatus     -> ComCcsds.fprimeRouter.cmdResponseIn
    }
  }

}
