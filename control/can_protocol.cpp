#include "channel/uart_protocol.h"
#include "can_server.h"

const QHash<quint32, quint16> CanServerManager::m_canToUart =
{
    // output
    {0x036001, QCmd(JV6xxx::UartCMD::OUTP_OFF)},
    {0x106001, JV6xxx::UartCMD::OUTP_OFF},
    {0x107006, JV6xxx::UartCMD::OUTPBAND},
    {0x037006, QCmd(JV6xxx::UartCMD::OUTPBAND)},
    {0x107007, JV6xxx::UartCMD::OUTPCOMP},
    {0x037007, QCmd(JV6xxx::UartCMD::OUTPCOMP)},
    // setting
    {0x036002, QCmd(JV6xxx::UartCMD::VOLT)},
    {0x016002, JV6xxx::UartCMD::VOLT},
    {0x036003, QCmd(JV6xxx::UartCMD::CURR)},
    {0x106003, JV6xxx::UartCMD::CURR},
    {0x036004, QCmd(JV6xxx::UartCMD::IMP)},
    {0x106004, JV6xxx::UartCMD::IMP},
    {0x036005, QCmd(JV6xxx::UartCMD::OVP)},
    {0x106005, JV6xxx::UartCMD::OVP},
    {0x036006, QCmd(JV6xxx::UartCMD::OTP)},
    {0x106006, JV6xxx::UartCMD::OTP},
    {0x036040, QCmd(JV6xxx::UartCMD::SetLoadCurr)},
    {0x106040, JV6xxx::UartCMD::SetLoadCurr},
    // control
    {0x036007, QCmd(JV6xxx::UartCMD::EnLOAD)},
    {0x106007, JV6xxx::UartCMD::EnLOAD},
    {0x036008, QCmd(JV6xxx::UartCMD::EnOVP)},
    {0x106008, JV6xxx::UartCMD::EnOVP},
    {0x036009, QCmd(JV6xxx::UartCMD::EnOCP)},
    {0x106009, JV6xxx::UartCMD::EnOCP},
    {0x03600A, QCmd(JV6xxx::UartCMD::EnOTP)},
    {0x10600A, JV6xxx::UartCMD::EnOTP},
    {0x03600B, QCmd(JV6xxx::UartCMD::EnIMPE)},
    {0x10600B, JV6xxx::UartCMD::EnIMPE},
    {0x03600C, QCmd(JV6xxx::UartCMD::SetPON)},
    {0x10600C, JV6xxx::UartCMD::SetPON},
    {0x036041, QCmd(JV6xxx::UartCMD::LoadINDE)},
    {0x106041, JV6xxx::UartCMD::LoadINDE},
    {0x036042, QCmd(JV6xxx::UartCMD::LoadOCP)},
    {0x106042, JV6xxx::UartCMD::LoadOCP},
    // measurement
    {0x03600D, QCmd(JV6xxx::UartCMD::AverCount)},
    {0x10600D, JV6xxx::UartCMD::AverCount},

    {0x03600E, QCmd(JV6xxx::UartCMD::RVoltage)},
    {0x03600F, QCmd(JV6xxx::UartCMD::RCurrent)},
    {0x036010, QCmd(JV6xxx::UartCMD::RSCurrent)},
    {0x036011, QCmd(JV6xxx::UartCMD::RBT)},
    {0x036012, QCmd(JV6xxx::UartCMD::RHT)},
    {0x036013, QCmd(JV6xxx::UartCMD::RDVMACDC)},
    {0x036014, QCmd(JV6xxx::UartCMD::RDVMDC)},
    {0x036015, QCmd(JV6xxx::UartCMD::RFSpeed)},

    {0x036029, QCmd(JV6xxx::UartCMD::TMP1)},
    {0x03602A, QCmd(JV6xxx::UartCMD::TMP2)},
    {0x03602B, QCmd(JV6xxx::UartCMD::TMP3)},

    {0x03601E, QCmd(JV6xxx::UartCMD::RDVMAC)},

    {0x106030, JV6xxx::UartCMD::SetFUNC},
    {0x036031, QCmd(JV6xxx::UartCMD::PCurrHIGH)},
    {0x036032, QCmd(JV6xxx::UartCMD::PCurrLOW)},
    {0x036033, QCmd(JV6xxx::UartCMD::PCurrMAX)},
    {0x036034, QCmd(JV6xxx::UartCMD::PCurrMIN)},
    {0x036035, QCmd(JV6xxx::UartCMD::PDvmHIGH)},
    {0x036036, QCmd(JV6xxx::UartCMD::PDvmLOW)},
    {0x036037, QCmd(JV6xxx::UartCMD::PDvmMAX)},
    {0x036038, QCmd(JV6xxx::UartCMD::PDvmMIN)},
    {0x036039, QCmd(JV6xxx::UartCMD::PVoltHIGH)},
    {0x03603a, QCmd(JV6xxx::UartCMD::PVoltLOW)},
    {0x03603b, QCmd(JV6xxx::UartCMD::PVoltMAX)},
    {0x03603c, QCmd(JV6xxx::UartCMD::PVoltMIN)},
    // register
    {0x036016, QCmd(JV6xxx::UartCMD::StaREG)},
    {0x036017, QCmd(JV6xxx::UartCMD::EnREG)},
    {0x036018, QCmd(JV6xxx::UartCMD::CondREG)},
    {0x036019, QCmd(JV6xxx::UartCMD::ERR)},
    {0x10601A, JV6xxx::UartCMD::ERR},
    {0x03601B, QCmd(JV6xxx::UartCMD::SV)},
    {0x03601C, QCmd(JV6xxx::UartCMD::HV)},
    // trigger
    {0x107000, JV6xxx::UartCMD::ABOR},
    {0x107001, JV6xxx::UartCMD::INIT},
    {0x107002, JV6xxx::UartCMD::CONT},
    {0x107003, JV6xxx::UartCMD::SEQ1TRIG},
    {0x107004, JV6xxx::UartCMD::SEQ2TRIG},
    {0x107005, JV6xxx::UartCMD::ACQTRIGSOUR},
    {0x037005, QCmd(JV6xxx::UartCMD::ACQTRIGSOUR)},
    {0x107011, JV6xxx::UartCMD::COUN_COMM},
    {0x037011, QCmd(JV6xxx::UartCMD::COUN_COMM)},
    {0x107021, JV6xxx::UartCMD::HYST_COMM},
    {0x037021, QCmd(JV6xxx::UartCMD::HYST_COMM)},
    {0x107031, JV6xxx::UartCMD::LEV_COMM},
    {0x037031, QCmd(JV6xxx::UartCMD::LEV_COMM)},
    {0x107041, JV6xxx::UartCMD::SLOP_COMM},
    {0x037041, QCmd(JV6xxx::UartCMD::SLOP_COMM)},
    {0x037050, QCmd(JV6xxx::UartCMD::SOUR_VOLT_TRIG)},
    {0x107050, JV6xxx::UartCMD::SOUR_VOLT_TRIG},
    {0x037051, QCmd(JV6xxx::UartCMD::SOUR_CURR_TRIG)},
    {0x107051, JV6xxx::UartCMD::SOUR_CURR_TRIG},
    {0x037052, QCmd(JV6xxx::UartCMD::SOUR_RES_TRIG)},
    {0x107052, JV6xxx::UartCMD::SOUR_RES_TRIG},
    // calibrate
    {0x106102, JV6xxx::UartCMD::CalInit},
    {0x106103, JV6xxx::UartCMD::CalRestore},
    {0x106104, JV6xxx::UartCMD::CalSave},
    {0x036105, QCmd(JV6xxx::UartCMD::CalStarALL)},
    {0x106106, JV6xxx::UartCMD::CalStarALL},
    {0x106107, JV6xxx::UartCMD::CalStarADC},
    {0x106108, JV6xxx::UartCMD::CalStarDAC},
    {0x1062,   JV6xxx::UartCMD::CalStepOffset},
    // error
    {0xffffff,0xffff},
};
const QHash<quint16, quint32> CanServerManager::m_uartToCan = []
{
    QHash<quint16, quint32> reverse;
    for (auto it = m_canToUart.constBegin(); it != m_canToUart.constEnd(); ++it) {
        reverse.insert(it.value(), it.key());
    }
    return reverse;
}();
