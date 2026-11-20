#pragma once
#include <QMutex>
#include <QObject>
#include <QWaitCondition>
#ifdef __cplusplus
extern "C" {
#endif
#include "scpi/scpi.h"
#ifdef __cplusplus
}
#endif
#include "auxiliary/global_static.h"
#include "uart_protocol.h"

class ScpiManager : public QObject
{
    Q_OBJECT
private:
    static scpi_result_t SCPI_SERBaud(scpi_t* context);
    static scpi_result_t SCPI_SERBaudQ(scpi_t* context);
    static scpi_result_t SCPI_CANBaud(scpi_t* context);
    static scpi_result_t SCPI_CANBaudQ(scpi_t* context);
    static scpi_result_t SCPI_CANId(scpi_t* context);
    static scpi_result_t SCPI_CANIdQ(scpi_t* context);

    static scpi_result_t SCPI_LANIP(scpi_t* context){return SET_NETWork(context,NetMode::IP);};
    static scpi_result_t SCPI_LANSmask(scpi_t* context){return SET_NETWork(context,NetMode::MASK);};
    static scpi_result_t SCPI_LANGate(scpi_t* context){return SET_NETWork(context,NetMode::GATE);};
    static scpi_result_t SCPI_LANDHCP(scpi_t* context){return SET_NETWork(context,NetMode::DHCP);};
    static scpi_result_t SET_NETWork(scpi_t* context,int mode);
    static scpi_result_t SCPI_LANInfQ(scpi_t* context);
    // --- Output --------------------------------------------------------------------------------
    static scpi_result_t SCPI_OutputState(scpi_t* context);
    static scpi_result_t SCPI_OutputBand(scpi_t* context)            {return sendChoiceCmd(context,JV6xxx::UartCMD::OUTPBAND);};
    static scpi_result_t SCPI_OutputCompMode(scpi_t* context)        {return sendChoiceCmd(context,JV6xxx::UartCMD::OUTPCOMP);};
    // -
    static scpi_result_t SCPI_OutputStateQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::OUTP_OFF));};
    static scpi_result_t SCPI_OutputBandQ(scpi_t* context)           {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::OUTPBAND));};
    static scpi_result_t SCPI_OutputCompModeQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::OUTPCOMP));};
    // --- Setting -------------------------------------------------------------------------------
    static scpi_result_t SCPI_SettingVolt(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::VOLT);};
    static scpi_result_t SCPI_SettingCurr(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::CURR);};
    static scpi_result_t SCPI_SettingImpedance(scpi_t* context)      {return sendFloatCmd(context,JV6xxx::UartCMD::IMP);};
    static scpi_result_t SCPI_SettingProt(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::OVP);};
    static scpi_result_t SCPI_SettingTher(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::OTP);};
    static scpi_result_t SCPI_SettingLoad(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::SetLoadCurr);};
    // -
    static scpi_result_t SCPI_SettingVoltQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::VOLT));};
    static scpi_result_t SCPI_SettingCurrQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::CURR));};
    static scpi_result_t SCPI_SettingImpedanceQ(scpi_t* context)     {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::IMP));};
    static scpi_result_t SCPI_SettingProtQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::OVP));};
    static scpi_result_t SCPI_SettingTherQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::OTP));};
    static scpi_result_t SCPI_SettingLoadQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SetLoadCurr));};
    // --- Control -------------------------------------------------------------------------------
    static scpi_result_t SCPI_ControlClamp(scpi_t* context)          {return sendBoolCmd(context,JV6xxx::UartCMD::EnOVP);};
    static scpi_result_t SCPI_ControlCurr(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::UartCMD::EnOCP);};
    static scpi_result_t SCPI_ControlSyst(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::UartCMD::SetPON);};
    static scpi_result_t SCPI_ControlSAV(scpi_t* context)            {return sendIntCmd(context,JV6xxx::UartCMD::SavPON,1);};
    static scpi_result_t SCPI_ControlRCL(scpi_t* context)            {return sendIntCmd(context,JV6xxx::UartCMD::RecallPON,1);};
    static scpi_result_t SCPI_ControlInde(scpi_t* context)           {return sendBoolCmd(context,JV6xxx::UartCMD::LoadINDE);};
    static scpi_result_t SCPI_ControlLoad(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::UartCMD::LoadOCP);};
    // -
    static scpi_result_t SCPI_ControlClampQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::EnOVP));};
    static scpi_result_t SCPI_ControlCurrQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::EnOCP));};
    static scpi_result_t SCPI_ControlSystQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SetPON));};
    static scpi_result_t SCPI_ControlIndeQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::LoadINDE));};
    static scpi_result_t SCPI_ControlLoadQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::LoadOCP));};
    // --- Measurement ---------------------------------------------------------------------------
    static scpi_result_t SCPI_MeasureNplc(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::NPLC);};
    static scpi_result_t SCPI_MeasureRang(scpi_t* context)           {return sendIntCmd(context,JV6xxx::UartCMD::CurrRANG,1);};
    static scpi_result_t SCPI_MeasureAver(scpi_t* context)           {return sendIntCmd(context,JV6xxx::UartCMD::AverCount,1);};
    static scpi_result_t SCPI_MeasureFunc(scpi_t* context);
    static scpi_result_t SCPI_MeasurePoin(scpi_t* context)           {return sendIntCmd(context,JV6xxx::UartCMD::SwePOIN,2);};
    static scpi_result_t SCPI_MeasureTint(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::SweTINT);};
    static scpi_result_t SCPI_MeasureTime(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::CurrRangTIM);};
    static scpi_result_t SCPI_MeasureOffs(scpi_t* context)           {return sendIntCmd(context,JV6xxx::UartCMD::SweOffPin,4);};
    // -
    static scpi_result_t SCPI_MeasureVoltQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RVoltage));};
    static scpi_result_t SCPI_MeasureCurrQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RCurrent));};
    static scpi_result_t SCPI_MeasureScurQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RSCurrent));};
    static scpi_result_t SCPI_MeasureBtmpQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RBT));};
    static scpi_result_t SCPI_MeasureHtmpQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RHT));};
    static scpi_result_t SCPI_MeasureAcdcQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RDVMACDC));};
    static scpi_result_t SCPI_MeasureDvmQ(scpi_t* context)           {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RDVMDC));};
    static scpi_result_t SCPI_MeasureFanQ(scpi_t* context)           {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RFSpeed));};
    static scpi_result_t SCPI_MeasureDvmacQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RDVMAC));};
    static scpi_result_t SCPI_MeasureTemp1Q(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::TMP1));};
    static scpi_result_t SCPI_MeasureTemp2Q(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::TMP2));};
    static scpi_result_t SCPI_MeasureNplcQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::NPLC));};
    static scpi_result_t SCPI_MeasureTemp3Q(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::TMP3));};
    static scpi_result_t SCPI_MeasureRangQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::CurrRANG));};
    static scpi_result_t SCPI_MeasureAverQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::AverCount));};
    static scpi_result_t SCPI_MeasureFuncQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SetFUNC));};
    static scpi_result_t SCPI_MeasureCurrHighQ(scpi_t* context)      {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PCurrHIGH));};
    static scpi_result_t SCPI_MeasureCurrLowQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PCurrLOW));};
    static scpi_result_t SCPI_MeasureCurrMaxQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PCurrMAX));};
    static scpi_result_t SCPI_MeasureCurrMinQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PCurrMIN));};
    static scpi_result_t SCPI_MeasureDvmHighQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PDvmHIGH));};
    static scpi_result_t SCPI_MeasureDvmLowQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PDvmLOW));};
    static scpi_result_t SCPI_MeasureDvmMaxQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PDvmMAX));};
    static scpi_result_t SCPI_MeasureDvmMinQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PDvmMIN));};
    static scpi_result_t SCPI_MeasureVoltHighQ(scpi_t* context)      {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PVoltHIGH));};
    static scpi_result_t SCPI_MeasureVoltLowQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PVoltLOW));};
    static scpi_result_t SCPI_MeasureVoltMaxQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PVoltMAX));};
    static scpi_result_t SCPI_MeasureVoltMinQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::PVoltMIN));};
    static scpi_result_t SCPI_MeasurePoinQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SwePOIN));}; // repeat cmd -> 0x0427 = Duty
    static scpi_result_t SCPI_MeasureTintQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SweTINT));};
    static scpi_result_t SCPI_MeasureTimeQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::CurrRangTIM));};
    static scpi_result_t SCPI_MeasureArrVoltQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RArrVOLT));};
    static scpi_result_t SCPI_MeasureArrCurrQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RArrCURR));};
    static scpi_result_t SCPI_MeasureArrDvmQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RArrDVM));};
    static scpi_result_t SCPI_MeasureOffsQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SweOffPin));};
    static scpi_result_t SCPI_MeasureDutyQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RFanDuty));}; // -
    static scpi_result_t SCPI_MeasureAdofVoltQ(scpi_t* context)      {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RVoltADC));};
    static scpi_result_t SCPI_MeasureAdofCurrQ(scpi_t* context)      {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RCurrADC));};
    static scpi_result_t SCPI_MeasureAdofScurQ(scpi_t* context)      {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RSCurrADC));};
    static scpi_result_t SCPI_MeasureAdofDvmQ(scpi_t* context)       {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::RDvmADC));};
    // --- Register ------------------------------------------------------------------------------
    static scpi_result_t SCPI_RegisterCle(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::UartCMD::ERR);};
    // -
    static scpi_result_t SCPI_RegisterEvenQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::StaREG));};
    static scpi_result_t SCPI_RegisterEnabQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::EnREG));};
    static scpi_result_t SCPI_RegisterCondQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::CondREG));};
    static scpi_result_t SCPI_RegisterNextQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::ERR));};
    // --- Calibrate -----------------------------------------------------------------------------
    static scpi_result_t SCPI_CalibrateExit(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::UartCMD::CalExit);};
    static scpi_result_t SCPI_CalibrateInit(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::UartCMD::CalInit);};
    static scpi_result_t SCPI_CalibrateRest(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::UartCMD::CalRestore);};
    static scpi_result_t SCPI_CalibrateSave(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::UartCMD::CalSave);};
    static scpi_result_t SCPI_CalibrateAll(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarALL);};
    static scpi_result_t SCPI_CalibrateAdc(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarADC);};
    static scpi_result_t SCPI_CalibrateDac(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarDAC);};
    static scpi_result_t SCPI_CalibrateEnab(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarENAB);};
    static scpi_result_t SCPI_CalibrateImp(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarIMP);};
    static scpi_result_t SCPI_CalibrateDcp(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarDCP);};
    static scpi_result_t SCPI_CalibrateDcn(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::CalStarDCN);};
    static scpi_result_t SCPI_CalibrateStep(scpi_t* context);
    // -
    static scpi_result_t SCPI_CalibrateAllQ(scpi_t* context)         {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::CalStarALL));};
    static scpi_result_t SCPI_CalibrateStepQ(scpi_t* context);
    // --- Trigger -------------------------------------------------------------------------------
    static scpi_result_t SCPI_TriggerAbort(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::UartCMD::ABOR);};
    static scpi_result_t SCPI_TriggerSequene(scpi_t* context)        {return sendIntCmd(context,JV6xxx::UartCMD::INIT,1);};
    static scpi_result_t SCPI_TriggerCont(scpi_t* context)           {return sendBoolCmd(context,JV6xxx::UartCMD::CONT);};
    static scpi_result_t SCPI_TriggerContname(scpi_t* context)       {return sendChoiceCmd(context,JV6xxx::UartCMD::CONT);};
    static scpi_result_t SCPI_TriggerSeq1(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::UartCMD::SEQ1TRIG);};
    static scpi_result_t SCPI_TriggerSeq2(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::UartCMD::SEQ2TRIG);};
    static scpi_result_t SCPI_TriggerSeq2So(scpi_t* context)         {return sendChoiceCmd(context,JV6xxx::UartCMD::ACQTRIGSOUR);};
    static scpi_result_t SCPI_TriggerSeq2Co(scpi_t* context)         {return sendIntCmd(context,JV6xxx::UartCMD::COUN_COMM,2);};
    static scpi_result_t SCPI_TriggerSeq2Hy(scpi_t* context)         {return sendFloatCmd(context,JV6xxx::UartCMD::HYST_COMM);};
    static scpi_result_t SCPI_TriggerSeq2Le(scpi_t* context)         {return sendFloatCmd(context,JV6xxx::UartCMD::LEV_COMM);};
    static scpi_result_t SCPI_TriggerSeq2Sl(scpi_t* context)         {return sendChoiceCmd(context,JV6xxx::UartCMD::SLOP_COMM);};
    static scpi_result_t SCPI_TriggerAmpl(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::SOUR_VOLT_TRIG);};
    static scpi_result_t SCPI_TriggerCurr(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::UartCMD::SOUR_CURR_TRIG);};
    static scpi_result_t SCPI_TriggerRes(scpi_t* context)            {return sendFloatCmd(context,JV6xxx::UartCMD::SOUR_RES_TRIG);};
    // -
    static scpi_result_t SCPI_TriggerSeq2SoQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::ACQTRIGSOUR));};
    static scpi_result_t SCPI_TriggerSeq2CoQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::COUN_COMM));};
    static scpi_result_t SCPI_TriggerSeq2HyQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::HYST_COMM));};
    static scpi_result_t SCPI_TriggerSeq2LeQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::LEV_COMM));};
    static scpi_result_t SCPI_TriggerSeq2SlQ(scpi_t* context)        {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SLOP_COMM));};
    static scpi_result_t SCPI_TriggerAmplQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SOUR_VOLT_TRIG));};
    static scpi_result_t SCPI_TriggerCurrQ(scpi_t* context)          {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SOUR_CURR_TRIG));};
    static scpi_result_t SCPI_TriggerResQ(scpi_t* context)           {return sendQueryCmd(context,QCmd(JV6xxx::UartCMD::SOUR_RES_TRIG));};
    // --- Serial cmd query ----------------------------------------------------------------------
    int32_t m_querymode{0};
    static scpi_result_t SCPI_Query(scpi_t* context);

    /* Basic execution */
    static scpi_result_t sendBoolCmd(scpi_t* context, quint16 cmds);
    static scpi_result_t sendFloatCmd(scpi_t* context, quint16 cmds);
    static const scpi_choice_def_t m_CmdparaChoices[];
    static scpi_result_t sendChoiceCmd(scpi_t* context, quint16 cmds);
    static scpi_result_t sendIntCmd(scpi_t* context,quint16 cmds,quint8 bytes);

    /* Basic helper */
    static scpi_result_t sendQueryCmd(scpi_t* context,quint16 cmds, const QByteArray &data = "");
    struct ChannelAddress
    {
        int channel;
        int subAddr;
    };
    static std::vector<ChannelAddress> allChannelList();
    static std::vector<ChannelAddress> parseChannelList(scpi_t* context);
    static scpi_result_t sendMultiCHCmd(scpi_t* context,quint16 cmds, const QByteArray &data, const std::vector<ChannelAddress> &channels);
    static void SCPI_ReadQ(scpi_t* context);

public:
    static const scpi_command_t m_scpiCommands[];
    static const scpi_unit_def_t m_scpi_units[];
    static ScpiManager& instance();

    // Channel -> this
    void processCHVoidResponse(quint8 ch);
    void processCHIntResponse(quint8 ch,int value);
    void processCHBoolResponse(quint8 ch,bool state);
    void processCHFloatResponse(quint8 ch,float value);
    void processCHStringResponse(quint8 ch,const QString& value);
    // Control -> this
    QByteArray processCommand(const QByteArray& command);

private:
    QMutex m_callMutex;
    QByteArray m_responseBuffer;

    int m_ReType{0};
    quint8 m_channel{0};
    QMutex m_syncMutex;
    QWaitCondition m_syncCondition;

    int m_CHIntReturn{0};
    bool m_CHStateReturn{false};
    float m_CHFloatReturn{0.0f};
    QString m_CHStringReturn{""};

    explicit ScpiManager(QObject *parent = nullptr);
    ~ScpiManager() = default;
    Q_DISABLE_COPY(ScpiManager)

    scpi_t m_scpiContext;
    char m_inputBuffer[256];
    scpi_error_t m_errorQueue[18];
    scpi_interface_t m_interface;

    static scpi_result_t staticFlush(scpi_t* context);
    static scpi_result_t staticReset(scpi_t* context);
    static int           staticError(scpi_t* context, int_fast16_t err);
    static size_t        staticWrite(scpi_t* context, const char* data, size_t len);
    static scpi_result_t staticControl(scpi_t* context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val);
};
