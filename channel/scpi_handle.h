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
#include "uart_protocol.h"

class ScpiManager : public QObject
{
    Q_OBJECT

private:   // auto inline
    static scpi_result_t SCPI_NETInform(scpi_t* context);
    static scpi_result_t SET_NETWork(scpi_t* context,int staticmode,bool isstatic);
    static scpi_result_t SCPI_NETDHCP(scpi_t* context){return SET_NETWork(context,0,false);};
    static scpi_result_t SCPI_NETIPADress(scpi_t* context){return SET_NETWork(context,1,true);};
    static scpi_result_t SCPI_NETSUBNetmask(scpi_t* context){return SET_NETWork(context,2,true);};
    static scpi_result_t SCPI_NETGATeway(scpi_t* context){return SET_NETWork(context,3,true);};

    int32_t m_querymode{0};
    static scpi_result_t SCPI_Query(scpi_t* context);
    // --- Output -------------------------------------------------------------------------------
    static scpi_result_t SCPI_OutputState(scpi_t* context);
    static scpi_result_t SCPI_OutputBand(scpi_t* context)            {return sendChoiceCmd(context,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPBAND);};
    static scpi_result_t SCPI_OutputCompMode(scpi_t* context)        {return sendChoiceCmd(context,JV6xxx::OUTPUT,JV6xxx::Output::SET_OUTPCOMP_MODE);};
    // -
    static scpi_result_t SCPI_OutputStateQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::OUTPUT,JV6xxx::Output::QUERY_OUTPSTATUS);};
    static scpi_result_t SCPI_OutputBandQ(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::OUTPUT,JV6xxx::Output::QUERY_OUTPBAND);};
    static scpi_result_t SCPI_OutputCompModeQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::OUTPUT,JV6xxx::Output::QUERY_OUTPCOMP_MODE);};
    // --- Setting ------------------------------------------------------------------------------
    static scpi_result_t SCPI_SettingVolt(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT);};
    static scpi_result_t SCPI_SettingCurr(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURCURR);};
    static scpi_result_t SCPI_SettingImpedance(scpi_t* context)      {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURIMP);};
    static scpi_result_t SCPI_SettingProt(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_SOURVOLT_PROT);};
    static scpi_result_t SCPI_SettingTher(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_THERPROT);};
    static scpi_result_t SCPI_SettingLoad(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::SETTING,JV6xxx::Setting::SET_LOADCURR);};
    // -
    static scpi_result_t SCPI_SettingVoltQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_SOURVOLT);};
    static scpi_result_t SCPI_SettingCurrQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_SOURCURR);};
    static scpi_result_t SCPI_SettingImpedanceQ(scpi_t* context)     {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_SOURIMP);};
    static scpi_result_t SCPI_SettingProtQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_SOURVOLT_PROT);};
    static scpi_result_t SCPI_SettingTherQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_THERPROT);};
    static scpi_result_t SCPI_SettingLoadQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::SETTING,JV6xxx::Setting::QUERY_LOADCURR);};
    // --- Control ------------------------------------------------------------------------------
    static scpi_result_t SCPI_ControlClamp(scpi_t* context)          {return sendBoolCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_SOURVOLT_CLAM);};
    static scpi_result_t SCPI_ControlCurr(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_SOURCURR_TYPE);};
    static scpi_result_t SCPI_ControlSyst(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_SYSTPOS);};
    static scpi_result_t SCPI_ControlSAV(scpi_t* context)            {return sendIntCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_SAV,1);};
    static scpi_result_t SCPI_ControlRCL(scpi_t* context)            {return sendIntCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_RCL,1);};
    static scpi_result_t SCPI_ControlInde(scpi_t* context)           {return sendBoolCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_LOADINDE_STAT);};
    static scpi_result_t SCPI_ControlLoad(scpi_t* context)           {return sendChoiceCmd(context,JV6xxx::CONTROL,JV6xxx::Control::SET_LOADCURR_TYPE);};
    // -
    static scpi_result_t SCPI_ControlClampQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CONTROL,JV6xxx::Control::QUERY_SOURVOLT_CLAM);};
    static scpi_result_t SCPI_ControlCurrQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CONTROL,JV6xxx::Control::QUERY_SOURCURR_TYPE);};
    static scpi_result_t SCPI_ControlSystQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CONTROL,JV6xxx::Control::QUERY_SYSTPOS);};
    static scpi_result_t SCPI_ControlIndeQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CONTROL,JV6xxx::Control::QUERY_LOADINDE_STAT);};
    static scpi_result_t SCPI_ControlLoadQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CONTROL,JV6xxx::Control::QUERY_LOADCURR_TYPE);};
    // --- Measurement --------------------------------------------------------------------------
    static scpi_result_t SCPI_MeasureNplc(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSNPLC);};
    static scpi_result_t SCPI_MeasureRang(scpi_t* context)           {return sendIntCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE,1);};
    static scpi_result_t SCPI_MeasureAver(scpi_t* context)           {return sendIntCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSAVER,1);};
    static scpi_result_t SCPI_MeasureFunc(scpi_t* context);
    static scpi_result_t SCPI_MeasurePoin(scpi_t* context)           {return sendIntCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSSWE_POIN,2);};
    static scpi_result_t SCPI_MeasureTint(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSSWE_TINT);};
    static scpi_result_t SCPI_MeasureTime(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSCURR_RANGE_TIM);};
    static scpi_result_t SCPI_MeasureOffs(scpi_t* context)           {return sendIntCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::SET_SENSSWE_OFFS_POIN,4);};
    // -
    static scpi_result_t SCPI_MeasureVoltQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASVOLT);};
    static scpi_result_t SCPI_MeasureCurrQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASCURR);};
    static scpi_result_t SCPI_MeasureScurQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASSCUR);};
    static scpi_result_t SCPI_MeasureBtmpQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASBTMP);};
    static scpi_result_t SCPI_MeasureHtmpQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASHTMP);};
    static scpi_result_t SCPI_MeasureAcdcQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASDVM_ACDC);};
    static scpi_result_t SCPI_MeasureDvmQ(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASDVM);};
    static scpi_result_t SCPI_MeasureFanQ(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_THERFAN);};
    static scpi_result_t SCPI_MeasureDvmacQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASDVM_AC);};
    static scpi_result_t SCPI_MeasureTemp1Q(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASTMP1);};
    static scpi_result_t SCPI_MeasureTemp2Q(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASTMP2);};
    static scpi_result_t SCPI_MeasureNplcQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSNPLC);};
    static scpi_result_t SCPI_MeasureTemp3Q(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASTMP3);};
    static scpi_result_t SCPI_MeasureRangQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSCURR_RANGE);};
    static scpi_result_t SCPI_MeasureAverQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSAVER);};
    static scpi_result_t SCPI_MeasureFuncQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSFUNC);};
    static scpi_result_t SCPI_MeasureCurrHighQ(scpi_t* context)      {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCCURR_HIGH);};
    static scpi_result_t SCPI_MeasureCurrLowQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCCURR_LOW);};
    static scpi_result_t SCPI_MeasureCurrMaxQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCCURR_MAX);};
    static scpi_result_t SCPI_MeasureCurrMinQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCCURR_MIN);};
    static scpi_result_t SCPI_MeasureDvmHighQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCDVM_HIGH);};
    static scpi_result_t SCPI_MeasureDvmLowQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCDVM_LOW);};
    static scpi_result_t SCPI_MeasureDvmMaxQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCDVM_MAX);};
    static scpi_result_t SCPI_MeasureDvmMinQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCDVM_MIN);};
    static scpi_result_t SCPI_MeasureVoltHighQ(scpi_t* context)      {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCVOLT_HIGH);};
    static scpi_result_t SCPI_MeasureVoltLowQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCVOLT_LOW);};
    static scpi_result_t SCPI_MeasureVoltMaxQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCVOLT_MAX);};
    static scpi_result_t SCPI_MeasureVoltMinQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_FETCVOLT_MIN);};
    static scpi_result_t SCPI_MeasurePoinQ(scpi_t* context)          {return SCPI_MeasureDutyQ(context);}; //  repeat 9d-func
    static scpi_result_t SCPI_MeasureDutyQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_THERDUTY);};
    static scpi_result_t SCPI_MeasureTintQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSSWE_TINT);};
    static scpi_result_t SCPI_MeasureTimeQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSCURR_RANGE_TIME);};
    static scpi_result_t SCPI_MeasureArrVoltQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASARR_VOLT);};
    static scpi_result_t SCPI_MeasureArrCurrQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASARR_CURR);};
    static scpi_result_t SCPI_MeasureArrDvmQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASARR_DVM);};
    static scpi_result_t SCPI_MeasureOffsQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_SENSSWE_OFFS_POIN);};
    static scpi_result_t SCPI_MeasureAdofVoltQ(scpi_t* context)      {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASADC_VOLT);};
    static scpi_result_t SCPI_MeasureAdofCurrQ(scpi_t* context)      {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASADC_CURR);};
    static scpi_result_t SCPI_MeasureAdofScurQ(scpi_t* context)      {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASADC_SCUR);};
    static scpi_result_t SCPI_MeasureAdofDvmQ(scpi_t* context)       {return sendQueryCmd(context,JV6xxx::MEASUREMENT,JV6xxx::Measurement::QUERY_MEASADC_DVM);};
    // --- Register -----------------------------------------------------------------------------
    static scpi_result_t SCPI_RegisterCle(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::REGISTER,JV6xxx::Register::SET_STATQUE_CLEAR);};
    // -
    static scpi_result_t SCPI_RegisterEvenQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::REGISTER,JV6xxx::Register::QUERY_STATOPER_EVEN);};
    static scpi_result_t SCPI_RegisterEnabQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::REGISTER,JV6xxx::Register::QUERY_SYSTENAB);};
    static scpi_result_t SCPI_RegisterCondQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::REGISTER,JV6xxx::Register::QUERY_STATOPER_COND);};
    static scpi_result_t SCPI_RegisterNextQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::REGISTER,JV6xxx::Register::QUERY_STATQUE_NEXT);};
    // --- Calibrate ----------------------------------------------------------------------------
    static scpi_result_t SCPI_CalibrateExit(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALIEXIT);};
    static scpi_result_t SCPI_CalibrateInit(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALIINIT);};
    static scpi_result_t SCPI_CalibrateRest(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALIREST);};
    static scpi_result_t SCPI_CalibrateSave(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISAVE);};
    static scpi_result_t SCPI_CalibrateAll(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_ALL);};
    static scpi_result_t SCPI_CalibrateAdc(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_ADC);};
    static scpi_result_t SCPI_CalibrateDac(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_DAC);};
    static scpi_result_t SCPI_CalibrateEnab(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALITART_ENAB);};
    static scpi_result_t SCPI_CalibrateImp(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_IMP);};
    static scpi_result_t SCPI_CalibrateDcp(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_DCP);};
    static scpi_result_t SCPI_CalibrateDcn(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::SET_CALISTART_DCN);};
    static scpi_result_t SCPI_CalibrateStep(scpi_t* context);
    // -
    static scpi_result_t SCPI_CalibrateAllQ(scpi_t* context)         {return sendQueryCmd(context,JV6xxx::CALIBRATE,JV6xxx::Calibrate::QUERY_CALISTAR_ALL);};
    static scpi_result_t SCPI_CalibrateStepQ(scpi_t* context);
    // --- Trigger ------------------------------------------------------------------------------
    static scpi_result_t SCPI_TriggerAbort(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_ABORT);};
    static scpi_result_t SCPI_TriggerSequene(scpi_t* context)        {return sendIntCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_INIT_IMM,1);};
    static scpi_result_t SCPI_TriggerCont(scpi_t* context)           {return sendBoolCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_INIT_CONT);};
    static scpi_result_t SCPI_TriggerContname(scpi_t* context)       {return sendChoiceCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_INIT_CONT);};
    static scpi_result_t SCPI_TriggerSeq1(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ1);};
    static scpi_result_t SCPI_TriggerSeq2(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2);};
    static scpi_result_t SCPI_TriggerSeq2So(scpi_t* context)         {return sendChoiceCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2_SOUR);};
    static scpi_result_t SCPI_TriggerSeq2Co(scpi_t* context)         {return sendIntCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2_COUN,2);};
    static scpi_result_t SCPI_TriggerSeq2Hy(scpi_t* context)         {return sendFloatCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2_HYST);};
    static scpi_result_t SCPI_TriggerSeq2Le(scpi_t* context)         {return sendFloatCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2_LEV);};
    static scpi_result_t SCPI_TriggerSeq2Sl(scpi_t* context)         {return sendChoiceCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_TRIGSEQ2_SLOP);};
    static scpi_result_t SCPI_TriggerAmpl(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_SOURVOLT_AMPL_TRIG);};
    static scpi_result_t SCPI_TriggerCurr(scpi_t* context)           {return sendFloatCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_SOURCURR_TRIG);};
    static scpi_result_t SCPI_TriggerRes(scpi_t* context)            {return sendFloatCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::SET_SOURRES_TRIG);};
    // -
    static scpi_result_t SCPI_TriggerSeq2SoQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_TRIGSEQ2_SOUR);};
    static scpi_result_t SCPI_TriggerSeq2CoQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_TRIGSEQ2_COUN);};
    static scpi_result_t SCPI_TriggerSeq2HyQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_TRIGSEQ2_HYST);};
    static scpi_result_t SCPI_TriggerSeq2LeQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_TRIGSEQ2_LEV);};
    static scpi_result_t SCPI_TriggerSeq2SlQ(scpi_t* context)        {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_TRIGSEQ2_SLOP);};
    static scpi_result_t SCPI_TriggerAmplQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_SOURVOLT_AMPL_TRIG);};
    static scpi_result_t SCPI_TriggerCurrQ(scpi_t* context)          {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_SOURCURR_TRIG);};
    static scpi_result_t SCPI_TriggerResQ(scpi_t* context)           {return sendQueryCmd(context,JV6xxx::TRIGGER,JV6xxx::Trigger::QUERY_SOURRES_TRIG);};

    /* Basic execution */
    static scpi_result_t sendBoolCmd(scpi_t* context, quint8 cmd, quint8 func);
    static scpi_result_t sendFloatCmd(scpi_t* context, quint8 cmd, quint8 func);
    static const scpi_choice_def_t m_CmdparaChoices[];
    static scpi_result_t sendChoiceCmd(scpi_t* context, quint8 cmd, quint8 func);
    static scpi_result_t sendIntCmd(scpi_t* context, quint8 cmd, quint8 func,quint8 bytes);

    /* Basic helper */
    static scpi_result_t sendQueryCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data = "");
    struct ChannelAddress
    {
        int channel;
        int subAddr;
    };
    static std::vector<ChannelAddress> allChannelList();
    static std::vector<ChannelAddress> parseChannelList(scpi_t* context);
    static scpi_result_t sendMultiCHCmd(scpi_t* context, quint8 cmd, quint8 func, const QByteArray &data, const std::vector<ChannelAddress> &channels);
    static scpi_result_t SCPI_ReadQ(scpi_t* context);

public:
    static const scpi_command_t m_scpiCommands[];
    static const scpi_unit_def_t m_scpi_units[];
    static ScpiManager& instance();

    // Channel -> this
    void processCHVoidResponse();
    void processCHBoolResponse(bool state);
    void processCHFloatResponse(float value);
    void processCHIntResponse(int value);
    void processCHStringResponse(const QString& value);
    // Control -> this
    QByteArray processCommand(const QByteArray& command);

private:
    int m_ReType{0};
    QMutex m_syncMutex;
    QWaitCondition m_syncCondition;

    int m_CHIntReturn{0};
    bool m_CHStateReturn{false};
    float m_CHFloatReturn{0.0f};
    QString m_CHStringReturn{""};

    QMutex m_callMutex;
    QByteArray m_responseBuffer;

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
