#include "scpi_handle.h"
#include <QtCore>

Q_LOGGING_CATEGORY(scpi, "SCPI:")

const scpi_choice_def_t ScpiManager::m_CmdparaChoices[] = {
    // --- output -
    {"HIGH",     1},
    {"LOW",      0},
    {"Llocal",   1},
    {"Lremote",  2},
    {"Hlocal",   3},
    {"Hremote",  4},
    // --- control -
    {"TRIP",     1},
    {"LIMit",    0},
    {"CC",       0},
    {"RST",      0},
    {"SAV0",     1},
    {"SAV1",     2},
    {"SAV2",     3},
    {"SAV3",     4},
    {"SAV4",     5},
    // --- measure -
    {"CURR",     0},
    {"DVM",      1},
    {"VOLT",     2},
    {"SCUR",     3},
    {"BTMP",     4},
    {"HTMP",     5},
    {"TMP1",     6},
    {"TMP2",     7},
    {"TMP3",     8},
    // --- trigger -
    {"TRANsient",0},
    {"OUTPut",   1},
    {"BUS",      1},
    {"INT",      2},
    {"EXT",      3},
    {"POSitive", 1},
    {"NEGative", 2},
    {"EITHer",   3},
    SCPI_CHOICE_LIST_END
};

scpi_result_t ScpiManager::SCPI_Query(scpi_t* context){
    auto* self = static_cast<ScpiManager*>(context->user_context);

    switch (self->m_querymode){ // Only query ,not response. Read -> response
        case 0: return sendQueryCmd(context,0x04,0x81);//CURR
        case 1: return sendQueryCmd(context,0x04,0x86);//DVM
        case 2: return sendQueryCmd(context,0x04,0x80);//VOLT
        case 3: return sendQueryCmd(context,0x04,0x82);//SCUR
        case 4: return sendQueryCmd(context,0x04,0x83);//BTMP
        case 5: return sendQueryCmd(context,0x04,0x84);//HTMP
        case 6: return sendQueryCmd(context,0x04,0x8a);//TMP1
        case 7: return sendQueryCmd(context,0x04,0x8b);//TMP2
        case 8: return sendQueryCmd(context,0x04,0x8d);//TMP3
        default:return SCPI_RES_ERR;
    }
}

// Output
scpi_result_t ScpiManager::SCPI_OutputState(scpi_t* context) {
    scpi_bool_t OpenOut;
    if(SCPI_ParamBool(context, &OpenOut,true)){
        qCDebug(scpi)<<"[SCPI_OutputState]:SCPI_Output OpenOut: "<<OpenOut;
        quint8 func = OpenOut ? 0x01 : 0x00;
        return sendQueryCmd(context,0x01,func,"");
    }

    return SCPI_RES_ERR;
}

// Measurement
scpi_result_t ScpiManager::SCPI_MeasureFunc(scpi_t* context) {
    int32_t choice;
    auto* self = static_cast<ScpiManager*>(context->user_context);
    if (SCPI_ParamChoice(context, m_CmdparaChoices, &choice, true)) {
        qCDebug(scpi) <<"[SCPI_MeasureFunc]:Param choice: "<< choice;
        QByteArray data(1, static_cast<quint8>(choice));
        self->m_querymode = choice;
        switch (choice){
            case 0: return sendQueryCmd(context,0x04,0x10,data);
            case 1: return sendQueryCmd(context,0x04,0x10,data);
            case 2: return sendQueryCmd(context,0x04,0x10,data);
            default:return SCPI_RES_OK;
        }
    }

    return SCPI_RES_ERR;
}

// Calibrate
scpi_result_t ScpiManager::SCPI_CalibrateStep(scpi_t* context) {
    int32_t step;
    float parameter;
    if (SCPI_ParamInt(context, &step, true)) {
        if (SCPI_ParamFloat(context, &parameter, true)) {
            QByteArray data(4, 0);
            qCDebug(scpi) <<"[SCPI_CalibrateStep]:step: "<<step<<" param: "<<parameter;
            qToBigEndian(parameter, reinterpret_cast<uint8_t*>(data.data()));
            return sendQueryCmd(context,0x07,step,data);
        }
    }

    return SCPI_RES_ERR;
}
// -
scpi_result_t ScpiManager::SCPI_CalibrateStepQ(scpi_t* context) {
    int value;
    if (SCPI_ParamInt(context, &value, true)) {
        qCDebug(scpi) <<"[SCPI_CalibrateStepQ]:Step Value: "<< value;
        return sendQueryCmd(context,0x07,value);
    }

    return SCPI_RES_ERR;
}

// ******************************* Basic execution ************************************

scpi_result_t ScpiManager::sendBoolCmd(scpi_t* context, quint8 cmd, quint8 func) {
    scpi_bool_t OpenOut;
    if(SCPI_ParamBool(context, &OpenOut,true)){
        qCDebug(scpi)<<"[sendBoolCmd]:SCPI_Output OpenOut: "<<OpenOut;
        quint8 swi = OpenOut ? 0x01 : 0x00;
        QByteArray data(1, swi);
        return sendQueryCmd(context,cmd,func,data);
    }

    return SCPI_RES_ERR;
}

scpi_result_t ScpiManager::sendFloatCmd(scpi_t* context, quint8 cmd, quint8 func) {
    float value;
    if (SCPI_ParamFloat(context, &value, true)) {
        qCDebug(scpi) <<"[sendFloatCmd]:Channel Set Value:"<< value;
        QByteArray data(4, 0);
        qToBigEndian(value, reinterpret_cast<uint8_t*>(data.data()));
        return sendQueryCmd(context,cmd,func,data);
    }

    return SCPI_RES_ERR;
}

scpi_result_t ScpiManager::sendChoiceCmd(scpi_t* context,quint8 cmd, quint8 func) {
    int32_t choice;
    if (SCPI_ParamChoice(context, m_CmdparaChoices, &choice, true)) {
        qCDebug(scpi) <<"[sendChoiceCmd]:Channel choice: "<< choice;
        QByteArray data(1, static_cast<quint8>(choice));
        return sendQueryCmd(context,cmd,func,data);
    }

    return SCPI_RES_ERR;
}

scpi_result_t ScpiManager::sendIntCmd(scpi_t* context, quint8 cmd, quint8 func,quint8 bytes) {
    int value;
    if (SCPI_ParamInt(context, &value, true)) {
        qCDebug(scpi) <<"[sendIntCmd]:Channel Set Value:"<< value;

        QByteArray data(bytes, 0);
        if (bytes == 1) {
            data.append(static_cast<quint8>(value));
        }else if(bytes == 2){
            quint16 val = static_cast<quint16>(value);
            qToBigEndian(val, data.data());
        }else{
            int val = static_cast<int>(value); // 32 Bytes +-
            qToBigEndian(val, data.data());
        }

        return sendQueryCmd(context,cmd,func,data);
    }

    return SCPI_RES_ERR;
}

// ********************************* Basic helper *************************************

scpi_result_t ScpiManager::sendQueryCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    std::vector<ChannelAddress> channels = parseChannelList(context);

    if (!channels.empty()) {
        return sendMultiCHCmd(context, cmd, func, data, channels);
    }else{ // Extract only the numerical parts that are filled with "#" in the command list
        if(SCPI_CommandNumbers(context, &self->m_channel, 1, -1)){ // Array 1, Default Channel -1
            if (self->m_channel == 0){
                return sendAllCHCmd(context, cmd, func, data);
            }else if(self->m_channel > 0){
                return sendSingleCHCmd(context, cmd, func, data);
            } // channel not exist of command
        } // channel is not single number
    }

    SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
    return SCPI_RES_ERR; // SCPI_RES_ERR = -1; SCPI_RES_OK = 1
}

std::vector<ChannelAddress> ScpiManager::parseChannelList(scpi_t* context) {
    std::vector<ChannelAddress> channelList;
    scpi_parameter_t channelListParam;
    const int MAX_DIM = 2; // SCPI standard Max 2 dimensions

    if (SCPI_Parameter(context, &channelListParam, false)) { // false = Unnecessary parameters
        scpi_expr_result_t result;
        scpi_bool_t isRange;
        size_t index = 0;

        int32_t valuesFrom[MAX_DIM]; // 1!1
        int32_t valuesTo[MAX_DIM];   // 2!3
        // 1!1:2!3 = [(1.1)(1.2)(1.3)(2.1)(2.2)(2.3)]
        size_t dimensions;

        do {
            result = SCPI_ExprChannelListEntry(context, &channelListParam, index,
                                               &isRange, valuesFrom, valuesTo, MAX_DIM, &dimensions);
            if (result == SCPI_EXPR_OK) {
                if (!isRange) {          // (@1)  or  (@1!2)
                    channelList.push_back({valuesFrom[0],dimensions >= 2 ? valuesFrom[1] : 0});
                } else {                 // (@1:5) or (@1,2)
                    int start_ch = valuesFrom[0];
                    int end_ch = valuesTo[0];
                    int step_ch = (start_ch <= end_ch) ? 1 : -1;

                    for (int ch = start_ch; step_ch > 0 ? ch <= end_ch : ch >= end_ch; ch += step_ch) {
                        if (dimensions >= 2) {
                            int start_sub = valuesFrom[1];
                            int end_sub = valuesTo[1];
                            int step_sub = (start_sub <= end_sub) ? 1 : -1;

                            for (int sub = start_sub; step_sub > 0 ? sub <= end_sub : sub >= end_sub; sub += step_sub) {
                                channelList.push_back({ch, sub});
                            }
                        } else {
                            channelList.push_back({ch, 0});
                        }
                    }
                }

                index++;
            } // return emtry;
        } while (result == SCPI_EXPR_OK);
    }

    return channelList; // emtry or true list
}


scpi_result_t ScpiManager::sendMultiCHCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data, const std::vector<ChannelAddress> &channels){
    auto* self = static_cast<ScpiManager*>(context->user_context);
    self->m_responseBuffer.append("[");

    for (ChannelAddress chaddress : channels) {
        self->m_channel = chaddress.channel;

        sendSingleCHCmd(context, cmd, func, data);
        self->m_responseBuffer.append(",");
    }

    self->m_responseBuffer.chop(1); // remove ","
    self->m_responseBuffer.append("]");
    return SCPI_RES_OK;
}

scpi_result_t ScpiManager::sendSingleCHCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    switch (self->m_channel){
        #define CHANNEL(n) \
            case n: \
                emit self->to_UartChannel##n(cmd,func,data,true);  \
                qCDebug(scpi)<<"[sendSingleCHCmd]: Single-Channel: "<<n; \
                if(self->m_syncCondition.wait(&self->m_syncMutex, 120) && SCPI_ReadQ(context) == SCPI_RES_OK){ \
                    return SCPI_RES_OK; \
                } \
                self->m_responseBuffer.append("Null"); \
                return SCPI_RES_ERR;

        CHANNEL_COUNT
        #undef CHANNEL
            default:return SCPI_RES_ERR;
    }
}

scpi_result_t ScpiManager::sendAllCHCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    self->m_responseBuffer.append("[");
    #define CHANNEL(n) \
        emit self->to_UartChannel##n(cmd,func,data,true); \
        if(!self->m_syncCondition.wait(&self->m_syncMutex, 120) || SCPI_ReadQ(context) == SCPI_RES_ERR){ \
            self->m_responseBuffer.append("Null"); \
        } \
        self->m_responseBuffer.append(",");

    CHANNEL_COUNT
    #undef CHANNEL
    self->m_responseBuffer.chop(1); // remove ","
    self->m_responseBuffer.append("]");
    return SCPI_RES_OK;
}

scpi_result_t ScpiManager::SCPI_ReadQ(scpi_t* context) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    switch (self->m_ReturnType){
        case 0: return SCPI_RES_OK; // emtry response
        case 1: SCPI_ResultBool(context, self->m_CHStateReturn);                        return SCPI_RES_OK;
        case 2: SCPI_ResultFloat(context, self->m_CHFloatReturn);                       return SCPI_RES_OK;
        case 3: SCPI_ResultInt(context, self->m_CHIntReturn);                           return SCPI_RES_OK;
        case 4: SCPI_ResultText(context, self->m_CHStringReturn.toUtf8().constData());  return SCPI_RES_OK;
        default:return SCPI_RES_ERR;
    }
}

// ********************************** external API **********************************

void ScpiManager::processCHVoidResponse(){
    m_syncMutex.lock();
    m_ReturnType = 0;
    m_syncMutex.unlock();
    m_syncCondition.wakeAll();
}

void ScpiManager::processCHStateResponse(bool state) {
    m_syncMutex.lock();
    m_ReturnType = 1;
    m_CHStateReturn = state;
    m_syncMutex.unlock();
    m_syncCondition.wakeAll();
}

void ScpiManager::processCHFloatResponse(float chf) {
    m_syncMutex.lock();
    m_ReturnType = 2;
    m_CHFloatReturn = chf;
    m_syncMutex.unlock();
    m_syncCondition.wakeAll();
}

void ScpiManager::processCHIntResponse(int cht) {
    m_syncMutex.lock();
    m_ReturnType = 3;
    m_CHIntReturn = cht;
    m_syncMutex.unlock();
    m_syncCondition.wakeAll();
}

void ScpiManager::processCHStringResponse(QString str) {
    m_syncMutex.lock();
    m_ReturnType = 4;
    m_CHStringReturn = str;
    m_syncMutex.unlock();
    m_syncCondition.wakeAll();
}

QByteArray ScpiManager::processCommand(const QByteArray &command) {
    QMutexLocker locker(&m_callMutex);
    m_responseBuffer.clear();

    SCPI_Input(&m_scpiContext, command.constData(), command.size());
    return m_responseBuffer; // if not query, return emtry
}

// ********************************* Initialize SCPI ********************************

const scpi_command_t ScpiManager::m_scpiCommands[] = {
     /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { "*CLS",      SCPI_CoreCls,    0 },//清除所有状态数据结构（寄存器、错误队列）
    { "*ESE",      SCPI_CoreEse,    0 },//设置标准事件状态启用寄存器
    { "*ESE?",     SCPI_CoreEseQ,   0 },//读取标准事件状态启用寄存器
    { "*ESR?",     SCPI_CoreEsrQ,   0 },//读取标准事件状态寄存器（读取后自动清零）
    { "*IDN?",     SCPI_CoreIdnQ,   0 },
    { "*OPC",      SCPI_CoreOpc,    0 },//设置OPC位
    { "*OPC?",     SCPI_CoreOpcQ,   0 },//操作完成时设置OPC位
    { "*RST",      SCPI_CoreRst,    0 },//复位设备到已知状态
    { "*SRE",      SCPI_CoreSre,    0 },//设置服务请求启用寄存器
    { "*SRE?",     SCPI_CoreSreQ,   0 },//读取服务请求启用寄存器
    { "*STB?",     SCPI_CoreStbQ,   0 },//读取状态字节寄存器
    { "*TST?",     SCPI_CoreTstQ,   0 },//自测试查询
    { "*WAI",      SCPI_CoreWai,    0 },//等待操作完成

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    { ":READ?",                                       ScpiManager::SCPI_Query,                0 },
    // --- Output -------------------------------------------------------------------------------
    { ":OUTPut#[:STATe]",                             ScpiManager::SCPI_OutputState,          0 },
    { ":OUTPut#:BANDwidth",                           ScpiManager::SCPI_OutputBand,           0 },
    { ":OUTPut#:COMPensation:MODE",                   ScpiManager::SCPI_OutputCompMode,       0 },
    // -
    { ":OUTPut#[:STATe]?",                            ScpiManager::SCPI_OutputStateQ,         0 },
    { ":OUTPut#:BANDwidth?",                          ScpiManager::SCPI_OutputBandQ,          0 },
    { ":OUTPut#:COMPensation:MODE?",                  ScpiManager::SCPI_OutputCompModeQ,      0 },
    // --- Setting ------------------------------------------------------------------------------
    { ":OUTPut#:IMPedance",                           ScpiManager::SCPI_SettingImpedance,     0 },
    { ":SOURce#:VOLTage[:LEVel][:AMPLitude]",         ScpiManager::SCPI_SettingVolt,          0 },
    { ":SOURce#:VOLTage:PROTection",                  ScpiManager::SCPI_SettingProt,          0 },
    { ":SOURce#:CURRent[:LIMit][:VALue]",             ScpiManager::SCPI_SettingCurr,          0 },
    { ":THERmal#[:PROTection][:TEMPerature]",         ScpiManager::SCPI_SettingTher,          0 },
    { ":LOAD#:CURRent[:LIMit][:VALue]",               ScpiManager::SCPI_SettingLoad,          0 },
    // -
    { ":OUTPut#:IMPedance?",                          ScpiManager::SCPI_SettingImpedanceQ,    0 },
    { ":SOURce#:VOLTage[:LEVel][:AMPLitude]?",        ScpiManager::SCPI_SettingVoltQ,         0 },
    { ":SOURce#:VOLTage:PROTection?",                 ScpiManager::SCPI_SettingProtQ,         0 },
    { ":SOURce#:CURRent[:LIMit][:VALue]?",            ScpiManager::SCPI_SettingCurrQ,         0 },
    { ":THERmal#[:PROTection][:TEMPerature]?",        ScpiManager::SCPI_SettingTherQ,         0 },
    { ":LOAD#:CURRent[:LIMit][:VALue]?",              ScpiManager::SCPI_SettingLoadQ,         0 },
    // --- Control ------------------------------------------------------------------------------
    { ":SOURce#:VOLTage:PROTection:CLAMp",            ScpiManager::SCPI_ControlClamp,         0 },
    { ":SOURce#:CURRent[:LIMit]:TYPE",                ScpiManager::SCPI_ControlCurr,          0 },
    { ":SYSTem#:POSetup",                             ScpiManager::SCPI_ControlSyst,          0 },
    { "*SAV#",                                        ScpiManager::SCPI_ControlSAV,           0 },
    { "*RCL#",                                        ScpiManager::SCPI_ControlRCL,           0 },
    { ":LOAD#:INDEpendence[:STATe]",                  ScpiManager::SCPI_ControlInde,          0 },
    { ":LOAD#:CURRent[:LIMit]:TYPE",                  ScpiManager::SCPI_ControlLoad,          0 },
    // -
    { ":SOURce#:VOLTage:PROTection:CLAMp?",           ScpiManager::SCPI_ControlClampQ,        0 },
    { ":SOURce#:CURRent[:LIMit]:TYPE?",               ScpiManager::SCPI_ControlCurrQ,         0 },
    { ":SYSTem#:POSetup?",                            ScpiManager::SCPI_ControlSystQ,         0 },
    { ":LOAD#:INDEpendence[:STATe]?",                 ScpiManager::SCPI_ControlIndeQ,         0 },
    { ":LOAD#:CURRent[:LIMit]:TYPE?",                 ScpiManager::SCPI_ControlLoadQ,         0 },
    // --- Measurement --------------------------------------------------------------------------
    { ":SENSe#:NPLCycles",                            ScpiManager::SCPI_MeasureNplc,          0 },
    { ":SENSe#:CURRent[:DC]:RANGe:TIMe",              ScpiManager::SCPI_MeasureTime,          0 },
    { ":SENSe#:CURRent[:DC]:RANGe[:UPPer]",           ScpiManager::SCPI_MeasureRang,          0 },
    { ":SENSe#:AVERage",                              ScpiManager::SCPI_MeasureAver,          0 },
    { ":SENSe#:FUNCtion",                             ScpiManager::SCPI_MeasureFunc,          0 },
    { ":SENSe#:SWEep:OFFS:POINts",                    ScpiManager::SCPI_MeasureOffs,          0 },
    { ":SENSe#:SWEep:POINts",                         ScpiManager::SCPI_MeasurePoin,          0 },
    { ":SENSe#:SWEep:TINTerval",                      ScpiManager::SCPI_MeasureTint,          0 },
    // -
    { ":MEASure#:VOLTage[:DC]?",                      ScpiManager::SCPI_MeasureVoltQ,         0 },
    { ":MEASure#:CURRent[:DC]?",                      ScpiManager::SCPI_MeasureCurrQ,         0 },
    { ":MEASure#:SCURrent[:DC]?",                     ScpiManager::SCPI_MeasureScurQ,         0 },
    { ":MEASure#:BTMPerature?",                       ScpiManager::SCPI_MeasureBtmpQ,         0 },
    { ":MEASure#:HTMPerature?",                       ScpiManager::SCPI_MeasureHtmpQ,         0 },
    { ":MEASure#:DVMeter:ACDC?",                      ScpiManager::SCPI_MeasureAcdcQ,         0 },
    { ":MEASure#:DVMeter[:DC]?",                      ScpiManager::SCPI_MeasureDvmQ,          0 },
    { ":MEASure#:DVMeter:AC?",                        ScpiManager::SCPI_MeasureDvmacQ,        0 },
    { ":MEASure#:TMP1?",                              ScpiManager::SCPI_MeasureTemp1Q,        0 },
    { ":MEASure#:TMP2?",                              ScpiManager::SCPI_MeasureTemp2Q,        0 },
    { ":MEASure#:TMP3?",                              ScpiManager::SCPI_MeasureTemp3Q,        0 },
    { ":MEASure#:ADcOFfset:VOLTage?",                 ScpiManager::SCPI_MeasureAdofVoltQ,     0 },
    { ":MEASure#:ADcOFfset:CURRent?",                 ScpiManager::SCPI_MeasureAdofCurrQ,     0 },
    { ":MEASure#:ADcOFfset:SmallCURrent?",            ScpiManager::SCPI_MeasureAdofScurQ,     0 },
    { ":MEASure#:ADcOFfset:DVMeter?",                 ScpiManager::SCPI_MeasureAdofDvmQ,      0 },
    { ":MEASure#:ARRay:CURRent[:DC]?",                ScpiManager::SCPI_MeasureArrCurrQ,      0 },
    { ":MEASure#:ARRay:VOLTage[:DC]?",                ScpiManager::SCPI_MeasureArrVoltQ,      0 },
    { ":MEASure#:ARRay:DVMeter?",                     ScpiManager::SCPI_MeasureArrDvmQ,       0 },
    { ":THERmal#[:PROTection]:FAN?",                  ScpiManager::SCPI_MeasureFanQ,          0 },
    { ":THERmal#[:PROTection]:DUTY?",                 ScpiManager::SCPI_MeasureDutyQ,         0 },
    { ":SENSe#:NPLCycles?",                           ScpiManager::SCPI_MeasureNplcQ,         0 },
    { ":SENSe#:CURRent[:DC]:RANGe:TIMe?",             ScpiManager::SCPI_MeasureTimeQ,         0 },
    { ":SENSe#:CURRent[:DC]:RANGe[:UPPer][:AUTO]?",   ScpiManager::SCPI_MeasureRangQ,         0 },
    { ":SENSe#:AVERage?",                             ScpiManager::SCPI_MeasureAverQ,         0 },
    { ":SENSe#:FUNCtion?",                            ScpiManager::SCPI_MeasureFuncQ,         0 },
    { ":SENSe#:SWEep:OFFS:POINts?",                   ScpiManager::SCPI_MeasureOffsQ,         0 },
    { ":SENSe#:SWEep:POINts?",                        ScpiManager::SCPI_MeasurePoinQ,         0 },
    { ":SENSe#:SWEep:TINTerval?",                     ScpiManager::SCPI_MeasureTintQ,         0 },
    { ":FETCh#:CURRent:HIGH?",                        ScpiManager::SCPI_MeasureCurrHighQ,     0 },
    { ":FETCh#:CURRent:LOW?",                         ScpiManager::SCPI_MeasureCurrLowQ,      0 },
    { ":FETCh#:CURRent:MAXimum?",                     ScpiManager::SCPI_MeasureCurrMaxQ,      0 },
    { ":FETCh#:CURRent:MINimum?",                     ScpiManager::SCPI_MeasureCurrMinQ,      0 },
    { ":FETCh#:DVMeter:HIGH?",                        ScpiManager::SCPI_MeasureDvmHighQ,      0 },
    { ":FETCh#:DVMeter:LOW?",                         ScpiManager::SCPI_MeasureDvmLowQ,       0 },
    { ":FETCh#:DVMeter:MAXimum?",                     ScpiManager::SCPI_MeasureDvmMaxQ,       0 },
    { ":FETCh#:DVMeter:MINimum?",                     ScpiManager::SCPI_MeasureDvmMinQ,       0 },
    { ":FETCh#:VOLTage:HIGH?",                        ScpiManager::SCPI_MeasureVoltHighQ,     0 },
    { ":FETCh#:VOLTage:LOW?",                         ScpiManager::SCPI_MeasureVoltLowQ,      0 },
    { ":FETCh#:VOLTage:MAXimum?",                     ScpiManager::SCPI_MeasureVoltMaxQ,      0 },
    { ":FETCh#:VOLTage:MINimum?",                     ScpiManager::SCPI_MeasureVoltMinQ,      0 },
    // --- Register -----------------------------------------------------------------------------
    { ":STATus#:QUEue:CLEar",                         ScpiManager::SCPI_RegisterCle,          0 },
    // -
    { ":STATus#:OPERation[:EVENt]?",                  ScpiManager::SCPI_RegisterEvenQ,        0 },
    { ":STATus#:OPERation:ENAB?",                     ScpiManager::SCPI_RegisterEnabQ,        0 },
    { ":STATus#:OPERation:CONDition?",                ScpiManager::SCPI_RegisterCondQ,        0 },
    { ":STATus#:QUEue[:NEXT]?",                       ScpiManager::SCPI_RegisterNextQ,        0 },
    // --- Calibrate ----------------------------------------------------------------------------
    { ":CALIbrate#:EXIT",                             ScpiManager::SCPI_CalibrateExit,        0 },
    { ":CALIbrate#:INITialize",                       ScpiManager::SCPI_CalibrateInit,        0 },
    { ":CALIbrate#:RESTore",                          ScpiManager::SCPI_CalibrateRest,        0 },
    { ":CALIbrate#:SAVE",                             ScpiManager::SCPI_CalibrateSave,        0 },
    { ":CALIbrate#:STARt[:ALL]",                      ScpiManager::SCPI_CalibrateAll,         0 },
    { ":CALIbrate#:STARt:ADC",                        ScpiManager::SCPI_CalibrateAdc,         0 },
    { ":CALIbrate#:STARt:DAC",                        ScpiManager::SCPI_CalibrateDac,         0 },
    { ":CALIbrate#:STARt:ENABle",                     ScpiManager::SCPI_CalibrateEnab,        0 },
    { ":CALIbrate#:STARt:IMPedance",                  ScpiManager::SCPI_CalibrateImp,         0 },
    { ":CALIbrate#:STARt:DCPositiveOffset",           ScpiManager::SCPI_CalibrateDcp,         0 },
    { ":CALIbrate#:STARt:DCNegativeOffset",           ScpiManager::SCPI_CalibrateDcn,         0 },
    { ":CALIbrate#:STEP",                             ScpiManager::SCPI_CalibrateStep,        0 },
    // -
    { ":CALIbrate#:STARt[:ALL]?",                     ScpiManager::SCPI_CalibrateAllQ,        0 },
    { ":CALIbrate#:STEP?",                            ScpiManager::SCPI_CalibrateStepQ,       0 },
    // --- Trigger ------------------------------------------------------------------------------
    { ":ABORt#",                                      ScpiManager::SCPI_TriggerAbort,         0 },
    { ":INITiate#[:IMMediate]:SEQuence",              ScpiManager::SCPI_TriggerSequene,       0 },
    { ":INITiate#:CONTinuous:SEQuence1",              ScpiManager::SCPI_TriggerCont,          0 },
    { ":INITiate#:CONTinuous:NAME",                   ScpiManager::SCPI_TriggerContname,      0 },
    { ":TRIGger#[:SEQ1][:IMMediate]",                 ScpiManager::SCPI_TriggerSeq1,          0 },
    { ":TRIGger#:SEQ2[:IMMediate]",                   ScpiManager::SCPI_TriggerSeq2,          0 },
    { ":TRIGger#:SEQ2:SOURce",                        ScpiManager::SCPI_TriggerSeq2So,        0 },
    { ":TRIGger#:SEQ2:COUNt[:CURR][:DVM][:VOLT]",     ScpiManager::SCPI_TriggerSeq2Co,        0 },
    { ":TRIGger#:SEQ2:HYSTer[:CURR][:DVM][:VOLT]",    ScpiManager::SCPI_TriggerSeq2Hy,        0 },
    { ":TRIGger#:SEQ2:LEVel[:CURR][:DVM][:VOLT]",     ScpiManager::SCPI_TriggerSeq2Le,        0 },
    { ":TRIGger#:SEQ2:SLOPe[:CURR][:DVM][:VOLT]",     ScpiManager::SCPI_TriggerSeq2Sl,        0 },
    { ":SOURce#:VOLTage:AMPL:TRIG",                   ScpiManager::SCPI_TriggerAmpl,          0 },
    { ":SOURce#:CURRent:TRIG",                        ScpiManager::SCPI_TriggerCurr,          0 },
    { ":SOURce#:RES:TRIG",                            ScpiManager::SCPI_TriggerRes,           0 },
    // -
    { ":TRIGger#:SEQ2:SOURce?",                       ScpiManager::SCPI_TriggerSeq2SoQ,       0 },
    { ":TRIGger#:SEQ2:COUNt[:CURR][:DVM][:VOLT]?",    ScpiManager::SCPI_TriggerSeq2CoQ,       0 },
    { ":TRIGger#:SEQ2:HYSTer[:CURR][:DVM][:VOLT]?",   ScpiManager::SCPI_TriggerSeq2HyQ,       0 },
    { ":TRIGger#:SEQ2:LEVel[:CURR][:DVM][:VOLT]?",    ScpiManager::SCPI_TriggerSeq2LeQ,       0 },
    { ":TRIGger#:SEQ2:SLOPe[:CURR][:DVM][:VOLT]?",    ScpiManager::SCPI_TriggerSeq2SlQ,       0 },
    { ":SOURce#:VOLTage:AMPL:TRIG?",                  ScpiManager::SCPI_TriggerAmplQ,         0 },
    { ":SOURce#:CURRent:TRIG?",                       ScpiManager::SCPI_TriggerCurrQ,         0 },
    { ":SOURce#:RES:TRIG?",                           ScpiManager::SCPI_TriggerResQ,          0 },

    SCPI_CMD_LIST_END // Empty sentry post
};

const scpi_unit_def_t ScpiManager::m_scpi_units[] = {
    {"V",    SCPI_UNIT_VOLT,     1},       // 伏特
    {"MV",   SCPI_UNIT_VOLT,     1e-3},    // 毫伏
    {"UV",   SCPI_UNIT_VOLT,     1e-6},    // 微伏
    {"A",    SCPI_UNIT_AMPER,    1},       // 安培
    {"MA",   SCPI_UNIT_AMPER,    1e-3},    // 毫安
    {"UA",   SCPI_UNIT_AMPER,    1e-6},    // 微安
    {"S",    SCPI_UNIT_SECOND,   1},       // 秒
    {"MS",   SCPI_UNIT_SECOND,   1e-3},    // 毫秒
    {"US",   SCPI_UNIT_SECOND,   1e-6},    // 微秒
    {"HZ",   SCPI_UNIT_HERTZ,    1},       // 赫兹
    {"KHZ",  SCPI_UNIT_HERTZ,    1e3},     // 千赫兹
    {"MHZ",  SCPI_UNIT_HERTZ,    1e6},     // 兆赫兹
    {"OHM",  SCPI_UNIT_OHM,      1},       // 欧姆
    {"KOHM", SCPI_UNIT_OHM,      1e3},     // 千欧姆
    {"MOHM", SCPI_UNIT_OHM,      1e6},     // 兆欧姆
    SCPI_UNITS_LIST_END
};

ScpiManager::ScpiManager(QObject *parent) : QObject(parent) {
    // QByteArray <- QString
    m_idnManufacturer = ConfigManager::s_manufacturer.toUtf8();
    m_idnModel        = ConfigManager::s_model.toUtf8();
    m_idnSerialNumber = ConfigManager::s_serialNumber.toUtf8();
    m_idnVersion      = ConfigManager::s_firmwareVersion.toUtf8();

    m_interface.flush   = staticFlush;
    m_interface.reset   = staticReset;
    m_interface.error   = staticError;
    m_interface.write   = staticWrite;
    m_interface.control = staticControl;

    SCPI_Init(&m_scpiContext,
              m_scpiCommands,
              &m_interface,
              m_scpi_units,
              m_idnManufacturer.constData(),
              m_idnModel.constData(),
              m_idnSerialNumber.constData(),
              m_idnVersion.constData(),
              m_inputBuffer,
              sizeof(m_inputBuffer),
              m_errorQueue,
              sizeof(m_errorQueue) / sizeof(scpi_error_t));

    m_scpiContext.user_context = this;
}

scpi_result_t ScpiManager::staticFlush(scpi_t* context) {
    // Send immediately, no need for forceful pressure
    Q_UNUSED(context);
    return SCPI_RES_OK;
}

scpi_result_t ScpiManager::staticReset(scpi_t* context) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    qCDebug(scpi) <<"[staticReset]:SCPI and channel Reset";

    #define CHANNEL(n) \
        emit self->to_UartChannel##n##Reset();

    CHANNEL_COUNT
    #undef CHANNEL

    memset(self->m_inputBuffer, 0, sizeof(self->m_inputBuffer));
    memset(self->m_errorQueue, 0, sizeof(self->m_errorQueue));
    self->m_responseBuffer.clear();

    return SCPI_RES_OK;
}

int ScpiManager::staticError(scpi_t* context, int_fast16_t err) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    QString errorMsg = QString("{%1,'%2'}").arg(err).arg(SCPI_ErrorTranslate(err));
    qCDebug(scpi)<<"[staticError]:SCPI Error:"<< errorMsg;
    self->m_responseBuffer.append(errorMsg.toUtf8());
    return 0;
}

size_t ScpiManager::staticWrite(scpi_t* context, const char* data, size_t len) {
    auto* self = static_cast<ScpiManager*>(context->user_context);
    qCDebug(scpi)<<"[staticWrite]:SCPI Response: "<< data;
    self->m_responseBuffer.append(data, len);
    return len; // Automatically add \r\n
}

scpi_result_t ScpiManager::staticControl(scpi_t* context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    // To accommodate the hardware interface, it is not supported.
    Q_UNUSED(context);Q_UNUSED(ctrl);Q_UNUSED(val);
    return SCPI_RES_OK;
}
