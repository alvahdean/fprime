#include "ESP32/Svc/LedController/LedController.hpp"

namespace {

Fw::Logic logicFromState(const Fw::On ledState) {
    return ledState == Fw::On::ON ? Fw::Logic::HIGH : Fw::Logic::LOW;
}

Fw::On stateFromLogic(const Fw::Logic logicState) {
    return logicState == Fw::Logic::HIGH ? Fw::On::ON : Fw::On::OFF;
}

}  // namespace

namespace Svc {

LedController::LedController(const char* compName) : LedControllerComponentBase(compName) {}

LedController::~LedController() = default;

void LedController::GET_STATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    Fw::On ledState = Fw::On::OFF;
    if (!this->readLedState(ledState)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }
    this->log_ACTIVITY_HI_LED_STATE(ledState);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void LedController::SET_STATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On ledState) {
    if (!this->writeLedState(ledState)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }
    this->log_ACTIVITY_HI_LED_STATE(ledState);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void LedController::TOGGLE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    Fw::On ledState = Fw::On::OFF;
    if (!this->readLedState(ledState)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }
    const Fw::On toggled = (ledState == Fw::On::ON) ? Fw::On::OFF : Fw::On::ON;
    if (!this->writeLedState(toggled)) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }
    this->log_ACTIVITY_HI_LED_STATE(toggled);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

bool LedController::readLedState(Fw::On& ledState) {
    if (!this->isConnected_gpioRead_OutputPort(0)) {
        this->log_WARNING_HI_LED_READ_ERROR(Drv::GpioStatus::UNKNOWN_ERROR);
        return false;
    }
    Fw::Logic logic = Fw::Logic::LOW;
    const Drv::GpioStatus status = this->gpioRead_out(0, logic);
    if (status != Drv::GpioStatus::OP_OK) {
        this->log_WARNING_HI_LED_READ_ERROR(status);
        return false;
    }
    ledState = stateFromLogic(logic);
    return true;
}

bool LedController::writeLedState(Fw::On ledState) {
    if (!this->isConnected_gpioWrite_OutputPort(0)) {
        this->log_WARNING_HI_LED_WRITE_ERROR(Drv::GpioStatus::UNKNOWN_ERROR);
        return false;
    }
    const Drv::GpioStatus status = this->gpioWrite_out(0, logicFromState(ledState));
    if (status != Drv::GpioStatus::OP_OK) {
        this->log_WARNING_HI_LED_WRITE_ERROR(status);
        return false;
    }
    return true;
}

}  // namespace Svc
