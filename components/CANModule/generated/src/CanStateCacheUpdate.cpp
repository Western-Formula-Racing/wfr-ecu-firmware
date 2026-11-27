#include "can_gen/CanStateCacheUpdate.hpp"
#include "can_gen/CanRegistry.hpp"
#include "can_gen/CanCodec.hpp"

bool updateStateCacheFromFrame(const CanFrame& frame, CanStateCache& stateCache, uint32_t nowMs) {
    const MessageDefinition* def = getMessageDefinitionByCanId(frame.id);
    if (def == nullptr) {
        return false;
    }

    switch (def->msg) {
        case CanMessageId::CAN_MSG_VCU_PDM_REAR_CMD: {
            VCUPDMRearCmdMessage decoded{};
            if (decodeVCUPDMRearCmdMessage(frame, decoded)) {
                stateCache.vCUPDMRearCmd = decoded;
                stateCache.vCUPDMRearCmdValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M173_MODULATION_AND_FLUX_INFO: {
            M173ModulationAndFluxInfoMessage decoded{};
            if (decodeM173ModulationAndFluxInfoMessage(frame, decoded)) {
                stateCache.m173ModulationAndFluxInfo = decoded;
                stateCache.m173ModulationAndFluxInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M172_TORQUE_AND_TIMER_INFO: {
            M172TorqueAndTimerInfoMessage decoded{};
            if (decodeM172TorqueAndTimerInfoMessage(frame, decoded)) {
                stateCache.m172TorqueAndTimerInfo = decoded;
                stateCache.m172TorqueAndTimerInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M194_READ_WRITE_PARAM_RESPONSE: {
            M194ReadWriteParamResponseMessage decoded{};
            if (decodeM194ReadWriteParamResponseMessage(frame, decoded)) {
                stateCache.m194ReadWriteParamResponse = decoded;
                stateCache.m194ReadWriteParamResponseValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M193_READ_WRITE_PARAM_COMMAND: {
            M193ReadWriteParamCommandMessage decoded{};
            if (decodeM193ReadWriteParamCommandMessage(frame, decoded)) {
                stateCache.m193ReadWriteParamCommand = decoded;
                stateCache.m193ReadWriteParamCommandValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M192_COMMAND_MESSAGE: {
            M192CommandMessage decoded{};
            if (decodeM192CommandMessage(frame, decoded)) {
                stateCache.m192CommandMessage = decoded;
                stateCache.m192CommandMessageValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M171_FAULT_CODES: {
            M171FaultCodesMessage decoded{};
            if (decodeM171FaultCodesMessage(frame, decoded)) {
                stateCache.m171FaultCodes = decoded;
                stateCache.m171FaultCodesValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M170_INTERNAL_STATES: {
            M170InternalStatesMessage decoded{};
            if (decodeM170InternalStatesMessage(frame, decoded)) {
                stateCache.m170InternalStates = decoded;
                stateCache.m170InternalStatesValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M169_INTERNAL_VOLTAGES: {
            M169InternalVoltagesMessage decoded{};
            if (decodeM169InternalVoltagesMessage(frame, decoded)) {
                stateCache.m169InternalVoltages = decoded;
                stateCache.m169InternalVoltagesValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M168_FLUX_ID_IQ_INFO: {
            M168FluxIDIQInfoMessage decoded{};
            if (decodeM168FluxIDIQInfoMessage(frame, decoded)) {
                stateCache.m168FluxIDIQInfo = decoded;
                stateCache.m168FluxIDIQInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M167_VOLTAGE_INFO: {
            M167VoltageInfoMessage decoded{};
            if (decodeM167VoltageInfoMessage(frame, decoded)) {
                stateCache.m167VoltageInfo = decoded;
                stateCache.m167VoltageInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M166_CURRENT_INFO: {
            M166CurrentInfoMessage decoded{};
            if (decodeM166CurrentInfoMessage(frame, decoded)) {
                stateCache.m166CurrentInfo = decoded;
                stateCache.m166CurrentInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M165_MOTOR_POSITION_INFO: {
            M165MotorPositionInfoMessage decoded{};
            if (decodeM165MotorPositionInfoMessage(frame, decoded)) {
                stateCache.m165MotorPositionInfo = decoded;
                stateCache.m165MotorPositionInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M164_DIGITAL_INPUT_STATUS: {
            M164DigitalInputStatusMessage decoded{};
            if (decodeM164DigitalInputStatusMessage(frame, decoded)) {
                stateCache.m164DigitalInputStatus = decoded;
                stateCache.m164DigitalInputStatusValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M163_ANALOG_INPUT_VOLTAGES: {
            M163AnalogInputVoltagesMessage decoded{};
            if (decodeM163AnalogInputVoltagesMessage(frame, decoded)) {
                stateCache.m163AnalogInputVoltages = decoded;
                stateCache.m163AnalogInputVoltagesValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M162_TEMPERATURE_SET_3: {
            M162TemperatureSet3Message decoded{};
            if (decodeM162TemperatureSet3Message(frame, decoded)) {
                stateCache.m162TemperatureSet3 = decoded;
                stateCache.m162TemperatureSet3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M161_TEMPERATURE_SET_2: {
            M161TemperatureSet2Message decoded{};
            if (decodeM161TemperatureSet2Message(frame, decoded)) {
                stateCache.m161TemperatureSet2 = decoded;
                stateCache.m161TemperatureSet2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M160_TEMPERATURE_SET_1: {
            M160TemperatureSet1Message decoded{};
            if (decodeM160TemperatureSet1Message(frame, decoded)) {
                stateCache.m160TemperatureSet1 = decoded;
                stateCache.m160TemperatureSet1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M174_FIRMWARE_INFO: {
            M174FirmwareInfoMessage decoded{};
            if (decodeM174FirmwareInfoMessage(frame, decoded)) {
                stateCache.m174FirmwareInfo = decoded;
                stateCache.m174FirmwareInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M175_DIAG_DATA_MESSAGE: {
            M175DiagDataMessage decoded{};
            if (decodeM175DiagDataMessage(frame, decoded)) {
                stateCache.m175DiagDataMessage = decoded;
                stateCache.m175DiagDataMessageValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_BMS_CURRENT_LIMIT: {
            MOBOBMSCurrentLimitMessage decoded{};
            if (decodeMOBOBMSCurrentLimitMessage(frame, decoded)) {
                stateCache.mOBOBMSCurrentLimit = decoded;
                stateCache.mOBOBMSCurrentLimitValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M176_FAST_INFO: {
            M176FastInfoMessage decoded{};
            if (decodeM176FastInfoMessage(frame, decoded)) {
                stateCache.m176FastInfo = decoded;
                stateCache.m176FastInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_PEDAL_INFO: {
            VCUPedalInfoMessage decoded{};
            if (decodeVCUPedalInfoMessage(frame, decoded)) {
                stateCache.vCUPedalInfo = decoded;
                stateCache.vCUPedalInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_PDM_FRONT: {
            VCUPDMFrontMessage decoded{};
            if (decodeVCUPDMFrontMessage(frame, decoded)) {
                stateCache.vCUPDMFront = decoded;
                stateCache.vCUPDMFrontValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_STATE_INFO: {
            VCUStateInfoMessage decoded{};
            if (decodeVCUStateInfoMessage(frame, decoded)) {
                stateCache.vCUStateInfo = decoded;
                stateCache.vCUStateInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_PDM_REAR: {
            VCUPDMRearMessage decoded{};
            if (decodeVCUPDMRearMessage(frame, decoded)) {
                stateCache.vCUPDMRear = decoded;
                stateCache.vCUPDMRearValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_M177_TORQUE_CAPABILITY: {
            M177TorqueCapabilityMessage decoded{};
            if (decodeM177TorqueCapabilityMessage(frame, decoded)) {
                stateCache.m177TorqueCapability = decoded;
                stateCache.m177TorqueCapabilityValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_ELCON_LIMITS: {
            MOBOELCONLIMITSMessage decoded{};
            if (decodeMOBOELCONLIMITSMessage(frame, decoded)) {
                stateCache.mOBOELCONLIMITS = decoded;
                stateCache.mOBOELCONLIMITSValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_ELCON_STATUS: {
            ELCONSTATUSMessage decoded{};
            if (decodeELCONSTATUSMessage(frame, decoded)) {
                stateCache.eLCONSTATUS = decoded;
                stateCache.eLCONSTATUSValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_SENSORS_1: {
            VCUFrontSensors1Message decoded{};
            if (decodeVCUFrontSensors1Message(frame, decoded)) {
                stateCache.vCUFrontSensors1 = decoded;
                stateCache.vCUFrontSensors1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_SENSORS_2: {
            VCUFrontSensors2Message decoded{};
            if (decodeVCUFrontSensors2Message(frame, decoded)) {
                stateCache.vCUFrontSensors2 = decoded;
                stateCache.vCUFrontSensors2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_SENSORS_3: {
            VCUFrontSensors3Message decoded{};
            if (decodeVCUFrontSensors3Message(frame, decoded)) {
                stateCache.vCUFrontSensors3 = decoded;
                stateCache.vCUFrontSensors3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_SENSORS_4: {
            VCUFrontSensors4Message decoded{};
            if (decodeVCUFrontSensors4Message(frame, decoded)) {
                stateCache.vCUFrontSensors4 = decoded;
                stateCache.vCUFrontSensors4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_SENSORS_1: {
            VCURearSensors1Message decoded{};
            if (decodeVCURearSensors1Message(frame, decoded)) {
                stateCache.vCURearSensors1 = decoded;
                stateCache.vCURearSensors1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_SENSORS_2: {
            VCURearSensors2Message decoded{};
            if (decodeVCURearSensors2Message(frame, decoded)) {
                stateCache.vCURearSensors2 = decoded;
                stateCache.vCURearSensors2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_SENSORS_3: {
            VCURearSensors3Message decoded{};
            if (decodeVCURearSensors3Message(frame, decoded)) {
                stateCache.vCURearSensors3 = decoded;
                stateCache.vCURearSensors3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_SENSORS_4: {
            VCURearSensors4Message decoded{};
            if (decodeVCURearSensors4Message(frame, decoded)) {
                stateCache.vCURearSensors4 = decoded;
                stateCache.vCURearSensors4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_APPS_CALIBRATION: {
            VCUAPPSCalibrationMessage decoded{};
            if (decodeVCUAPPSCalibrationMessage(frame, decoded)) {
                stateCache.vCUAPPSCalibration = decoded;
                stateCache.vCUAPPSCalibrationValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_APPS_AUTOCALIBRATE: {
            VCUAPPSAUTOCALIBRATEMessage decoded{};
            if (decodeVCUAPPSAUTOCALIBRATEMessage(frame, decoded)) {
                stateCache.vCUAPPSAUTOCALIBRATE = decoded;
                stateCache.vCUAPPSAUTOCALIBRATEValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_IMU_1: {
            VCUFrontIMU1Message decoded{};
            if (decodeVCUFrontIMU1Message(frame, decoded)) {
                stateCache.vCUFrontIMU1 = decoded;
                stateCache.vCUFrontIMU1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_FRONT_IMU_2: {
            VCUFrontIMU2Message decoded{};
            if (decodeVCUFrontIMU2Message(frame, decoded)) {
                stateCache.vCUFrontIMU2 = decoded;
                stateCache.vCUFrontIMU2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_IMU_1: {
            VCURearIMU1Message decoded{};
            if (decodeVCURearIMU1Message(frame, decoded)) {
                stateCache.vCURearIMU1 = decoded;
                stateCache.vCURearIMU1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_VCU_REAR_IMU_2: {
            VCURearIMU2Message decoded{};
            if (decodeVCURearIMU2Message(frame, decoded)) {
                stateCache.vCURearIMU2 = decoded;
                stateCache.vCURearIMU2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_BASE_STATION_ALIVE: {
            BASESTATIONALIVEMessage decoded{};
            if (decodeBASESTATIONALIVEMessage(frame, decoded)) {
                stateCache.bASESTATIONALIVE = decoded;
                stateCache.bASESTATIONALIVEValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_STEERINGWHEEL: {
            SteeringWheelMessage decoded{};
            if (decodeSteeringWheelMessage(frame, decoded)) {
                stateCache.steeringWheel = decoded;
                stateCache.steeringWheelValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V1: {
            MOBOM1V1Message decoded{};
            if (decodeMOBOM1V1Message(frame, decoded)) {
                stateCache.mOBOM1V1 = decoded;
                stateCache.mOBOM1V1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V2: {
            MOBOM1V2Message decoded{};
            if (decodeMOBOM1V2Message(frame, decoded)) {
                stateCache.mOBOM1V2 = decoded;
                stateCache.mOBOM1V2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V3: {
            MOBOM1V3Message decoded{};
            if (decodeMOBOM1V3Message(frame, decoded)) {
                stateCache.mOBOM1V3 = decoded;
                stateCache.mOBOM1V3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V4: {
            MOBOM1V4Message decoded{};
            if (decodeMOBOM1V4Message(frame, decoded)) {
                stateCache.mOBOM1V4 = decoded;
                stateCache.mOBOM1V4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V5: {
            MOBOM1V5Message decoded{};
            if (decodeMOBOM1V5Message(frame, decoded)) {
                stateCache.mOBOM1V5 = decoded;
                stateCache.mOBOM1V5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V6: {
            MOBOM1V6Message decoded{};
            if (decodeMOBOM1V6Message(frame, decoded)) {
                stateCache.mOBOM1V6 = decoded;
                stateCache.mOBOM1V6Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1V7: {
            MOBOM1V7Message decoded{};
            if (decodeMOBOM1V7Message(frame, decoded)) {
                stateCache.mOBOM1V7 = decoded;
                stateCache.mOBOM1V7Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1T1: {
            MOBOM1T1Message decoded{};
            if (decodeMOBOM1T1Message(frame, decoded)) {
                stateCache.mOBOM1T1 = decoded;
                stateCache.mOBOM1T1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1T2: {
            MOBOM1T2Message decoded{};
            if (decodeMOBOM1T2Message(frame, decoded)) {
                stateCache.mOBOM1T2 = decoded;
                stateCache.mOBOM1T2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1T3: {
            MOBOM1T3Message decoded{};
            if (decodeMOBOM1T3Message(frame, decoded)) {
                stateCache.mOBOM1T3 = decoded;
                stateCache.mOBOM1T3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1T4: {
            MOBOM1T4Message decoded{};
            if (decodeMOBOM1T4Message(frame, decoded)) {
                stateCache.mOBOM1T4 = decoded;
                stateCache.mOBOM1T4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M1T5: {
            MOBOM1T5Message decoded{};
            if (decodeMOBOM1T5Message(frame, decoded)) {
                stateCache.mOBOM1T5 = decoded;
                stateCache.mOBOM1T5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V1: {
            MOBOM2V1Message decoded{};
            if (decodeMOBOM2V1Message(frame, decoded)) {
                stateCache.mOBOM2V1 = decoded;
                stateCache.mOBOM2V1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V2: {
            MOBOM2V2Message decoded{};
            if (decodeMOBOM2V2Message(frame, decoded)) {
                stateCache.mOBOM2V2 = decoded;
                stateCache.mOBOM2V2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V3: {
            MOBOM2V3Message decoded{};
            if (decodeMOBOM2V3Message(frame, decoded)) {
                stateCache.mOBOM2V3 = decoded;
                stateCache.mOBOM2V3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V4: {
            MOBOM2V4Message decoded{};
            if (decodeMOBOM2V4Message(frame, decoded)) {
                stateCache.mOBOM2V4 = decoded;
                stateCache.mOBOM2V4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V5: {
            MOBOM2V5Message decoded{};
            if (decodeMOBOM2V5Message(frame, decoded)) {
                stateCache.mOBOM2V5 = decoded;
                stateCache.mOBOM2V5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V6: {
            MOBOM2V6Message decoded{};
            if (decodeMOBOM2V6Message(frame, decoded)) {
                stateCache.mOBOM2V6 = decoded;
                stateCache.mOBOM2V6Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2V7: {
            MOBOM2V7Message decoded{};
            if (decodeMOBOM2V7Message(frame, decoded)) {
                stateCache.mOBOM2V7 = decoded;
                stateCache.mOBOM2V7Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2T1: {
            MOBOM2T1Message decoded{};
            if (decodeMOBOM2T1Message(frame, decoded)) {
                stateCache.mOBOM2T1 = decoded;
                stateCache.mOBOM2T1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2T2: {
            MOBOM2T2Message decoded{};
            if (decodeMOBOM2T2Message(frame, decoded)) {
                stateCache.mOBOM2T2 = decoded;
                stateCache.mOBOM2T2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2T3: {
            MOBOM2T3Message decoded{};
            if (decodeMOBOM2T3Message(frame, decoded)) {
                stateCache.mOBOM2T3 = decoded;
                stateCache.mOBOM2T3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2T4: {
            MOBOM2T4Message decoded{};
            if (decodeMOBOM2T4Message(frame, decoded)) {
                stateCache.mOBOM2T4 = decoded;
                stateCache.mOBOM2T4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M2T5: {
            MOBOM2T5Message decoded{};
            if (decodeMOBOM2T5Message(frame, decoded)) {
                stateCache.mOBOM2T5 = decoded;
                stateCache.mOBOM2T5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V1: {
            MOBOM3V1Message decoded{};
            if (decodeMOBOM3V1Message(frame, decoded)) {
                stateCache.mOBOM3V1 = decoded;
                stateCache.mOBOM3V1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V2: {
            MOBOM3V2Message decoded{};
            if (decodeMOBOM3V2Message(frame, decoded)) {
                stateCache.mOBOM3V2 = decoded;
                stateCache.mOBOM3V2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V3: {
            MOBOM3V3Message decoded{};
            if (decodeMOBOM3V3Message(frame, decoded)) {
                stateCache.mOBOM3V3 = decoded;
                stateCache.mOBOM3V3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V4: {
            MOBOM3V4Message decoded{};
            if (decodeMOBOM3V4Message(frame, decoded)) {
                stateCache.mOBOM3V4 = decoded;
                stateCache.mOBOM3V4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V5: {
            MOBOM3V5Message decoded{};
            if (decodeMOBOM3V5Message(frame, decoded)) {
                stateCache.mOBOM3V5 = decoded;
                stateCache.mOBOM3V5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V6: {
            MOBOM3V6Message decoded{};
            if (decodeMOBOM3V6Message(frame, decoded)) {
                stateCache.mOBOM3V6 = decoded;
                stateCache.mOBOM3V6Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3V7: {
            MOBOM3V7Message decoded{};
            if (decodeMOBOM3V7Message(frame, decoded)) {
                stateCache.mOBOM3V7 = decoded;
                stateCache.mOBOM3V7Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3T1: {
            MOBOM3T1Message decoded{};
            if (decodeMOBOM3T1Message(frame, decoded)) {
                stateCache.mOBOM3T1 = decoded;
                stateCache.mOBOM3T1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3T2: {
            MOBOM3T2Message decoded{};
            if (decodeMOBOM3T2Message(frame, decoded)) {
                stateCache.mOBOM3T2 = decoded;
                stateCache.mOBOM3T2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3T3: {
            MOBOM3T3Message decoded{};
            if (decodeMOBOM3T3Message(frame, decoded)) {
                stateCache.mOBOM3T3 = decoded;
                stateCache.mOBOM3T3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3T4: {
            MOBOM3T4Message decoded{};
            if (decodeMOBOM3T4Message(frame, decoded)) {
                stateCache.mOBOM3T4 = decoded;
                stateCache.mOBOM3T4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M3T5: {
            MOBOM3T5Message decoded{};
            if (decodeMOBOM3T5Message(frame, decoded)) {
                stateCache.mOBOM3T5 = decoded;
                stateCache.mOBOM3T5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V1: {
            MOBOM4V1Message decoded{};
            if (decodeMOBOM4V1Message(frame, decoded)) {
                stateCache.mOBOM4V1 = decoded;
                stateCache.mOBOM4V1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V2: {
            MOBOM4V2Message decoded{};
            if (decodeMOBOM4V2Message(frame, decoded)) {
                stateCache.mOBOM4V2 = decoded;
                stateCache.mOBOM4V2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V3: {
            MOBOM4V3Message decoded{};
            if (decodeMOBOM4V3Message(frame, decoded)) {
                stateCache.mOBOM4V3 = decoded;
                stateCache.mOBOM4V3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V4: {
            MOBOM4V4Message decoded{};
            if (decodeMOBOM4V4Message(frame, decoded)) {
                stateCache.mOBOM4V4 = decoded;
                stateCache.mOBOM4V4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V5: {
            MOBOM4V5Message decoded{};
            if (decodeMOBOM4V5Message(frame, decoded)) {
                stateCache.mOBOM4V5 = decoded;
                stateCache.mOBOM4V5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V6: {
            MOBOM4V6Message decoded{};
            if (decodeMOBOM4V6Message(frame, decoded)) {
                stateCache.mOBOM4V6 = decoded;
                stateCache.mOBOM4V6Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4V7: {
            MOBOM4V7Message decoded{};
            if (decodeMOBOM4V7Message(frame, decoded)) {
                stateCache.mOBOM4V7 = decoded;
                stateCache.mOBOM4V7Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4T1: {
            MOBOM4T1Message decoded{};
            if (decodeMOBOM4T1Message(frame, decoded)) {
                stateCache.mOBOM4T1 = decoded;
                stateCache.mOBOM4T1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4T2: {
            MOBOM4T2Message decoded{};
            if (decodeMOBOM4T2Message(frame, decoded)) {
                stateCache.mOBOM4T2 = decoded;
                stateCache.mOBOM4T2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4T3: {
            MOBOM4T3Message decoded{};
            if (decodeMOBOM4T3Message(frame, decoded)) {
                stateCache.mOBOM4T3 = decoded;
                stateCache.mOBOM4T3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4T4: {
            MOBOM4T4Message decoded{};
            if (decodeMOBOM4T4Message(frame, decoded)) {
                stateCache.mOBOM4T4 = decoded;
                stateCache.mOBOM4T4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M4T5: {
            MOBOM4T5Message decoded{};
            if (decodeMOBOM4T5Message(frame, decoded)) {
                stateCache.mOBOM4T5 = decoded;
                stateCache.mOBOM4T5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V1: {
            MOBOM5V1Message decoded{};
            if (decodeMOBOM5V1Message(frame, decoded)) {
                stateCache.mOBOM5V1 = decoded;
                stateCache.mOBOM5V1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V2: {
            MOBOM5V2Message decoded{};
            if (decodeMOBOM5V2Message(frame, decoded)) {
                stateCache.mOBOM5V2 = decoded;
                stateCache.mOBOM5V2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V3: {
            MOBOM5V3Message decoded{};
            if (decodeMOBOM5V3Message(frame, decoded)) {
                stateCache.mOBOM5V3 = decoded;
                stateCache.mOBOM5V3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V4: {
            MOBOM5V4Message decoded{};
            if (decodeMOBOM5V4Message(frame, decoded)) {
                stateCache.mOBOM5V4 = decoded;
                stateCache.mOBOM5V4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V5: {
            MOBOM5V5Message decoded{};
            if (decodeMOBOM5V5Message(frame, decoded)) {
                stateCache.mOBOM5V5 = decoded;
                stateCache.mOBOM5V5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V6: {
            MOBOM5V6Message decoded{};
            if (decodeMOBOM5V6Message(frame, decoded)) {
                stateCache.mOBOM5V6 = decoded;
                stateCache.mOBOM5V6Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5V7: {
            MOBOM5V7Message decoded{};
            if (decodeMOBOM5V7Message(frame, decoded)) {
                stateCache.mOBOM5V7 = decoded;
                stateCache.mOBOM5V7Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5T1: {
            MOBOM5T1Message decoded{};
            if (decodeMOBOM5T1Message(frame, decoded)) {
                stateCache.mOBOM5T1 = decoded;
                stateCache.mOBOM5T1Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5T2: {
            MOBOM5T2Message decoded{};
            if (decodeMOBOM5T2Message(frame, decoded)) {
                stateCache.mOBOM5T2 = decoded;
                stateCache.mOBOM5T2Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5T3: {
            MOBOM5T3Message decoded{};
            if (decodeMOBOM5T3Message(frame, decoded)) {
                stateCache.mOBOM5T3 = decoded;
                stateCache.mOBOM5T3Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5T4: {
            MOBOM5T4Message decoded{};
            if (decodeMOBOM5T4Message(frame, decoded)) {
                stateCache.mOBOM5T4 = decoded;
                stateCache.mOBOM5T4Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_M5T5: {
            MOBOM5T5Message decoded{};
            if (decodeMOBOM5T5Message(frame, decoded)) {
                stateCache.mOBOM5T5 = decoded;
                stateCache.mOBOM5T5Valid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_PACKSTATUS: {
            MOBOPackStatusMessage decoded{};
            if (decodeMOBOPackStatusMessage(frame, decoded)) {
                stateCache.mOBOPackStatus = decoded;
                stateCache.mOBOPackStatusValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_PACKINFO: {
            MOBOPackInfoMessage decoded{};
            if (decodeMOBOPackInfoMessage(frame, decoded)) {
                stateCache.mOBOPackInfo = decoded;
                stateCache.mOBOPackInfoValid = true;
                return true;
            }
            return false;
        }
        case CanMessageId::CAN_MSG_MOBO_ANALOG_READINGS: {
            MOBOANALOGREADINGSMessage decoded{};
            if (decodeMOBOANALOGREADINGSMessage(frame, decoded)) {
                stateCache.mOBOANALOGREADINGS = decoded;
                stateCache.mOBOANALOGREADINGSValid = true;
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}
