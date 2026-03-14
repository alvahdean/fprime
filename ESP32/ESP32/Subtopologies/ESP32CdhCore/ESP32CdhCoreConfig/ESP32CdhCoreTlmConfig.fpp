module ESP32CdhCore{

    instance tlmSend: Svc.TlmChan base id ESP32CdhCoreConfig.BASE_ID + 0x06000 \
        queue size ESP32CdhCoreConfig.QueueSizes.tlmSend \
        stack size ESP32CdhCoreConfig.StackSizes.tlmSend \
        priority ESP32CdhCoreConfig.Priorities.tlmSend \

    # Uncomment the following block and comment the above block to use TlmPacketizer instead of TlmChan
    # instance tlmSend: Svc.TlmPacketizer base id ESP32CdhCoreConfig.BASE_ID + 0x06000 \
    #    queue size ESP32CdhCoreConfig.QueueSizes.tlmSend \
    #    stack size ESP32CdhCoreConfig.StackSizes.tlmSend \
    #    priority ESP32CdhCoreConfig.Priorities.tlmSend \
    # {
    #    # NOTE: The Name Ref is specific to the Reference deployment, Ref
    #    # This name will need to be updated if wishing to use this in a custom deployment
    #    phase Fpp.ToCpp.Phases.configComponents """
    #    ESP32CdhCore::tlmSend.setPacketList(
    #        Ref::Ref_RefPacketsTlmPackets::packetList, 
    #        Ref::Ref_RefPacketsTlmPackets::omittedChannels, 
    #        1
    #    );
    #    """
    # }
}
