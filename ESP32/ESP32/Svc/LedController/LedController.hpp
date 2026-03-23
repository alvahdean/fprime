#ifndef ESP32_SVC_LEDCONTROLLER_HPP
#define ESP32_SVC_LEDCONTROLLER_HPP

#include "ESP32/Svc/LedController/LedControllerComponentAc.hpp"

namespace Svc {

class LedController final : public LedControllerComponentBase {
  public:
    explicit LedController(const char* compName);
    ~LedController() override;

  private:
    void GET_STATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void SET_STATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On ledState) override;
    void TOGGLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    bool readLedState(Fw::On& ledState);
    bool writeLedState(Fw::On ledState);
};

}  // namespace Svc

#endif
