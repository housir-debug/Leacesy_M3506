#pragma once
#include <QString>

namespace JV6xxx {
    constexpr int MAX_QUEUE_CNT  = 9999;
    constexpr int Thread_CHSUM   = 0x09;

    constexpr quint8 HEADER_HIGH = 0xAA;
    constexpr quint8 HEADER_LOW  = 0x55;
    constexpr quint8 END_MARKER  = 0xEE;

    enum UartCMD : quint16 {
        WRITE           = 0X0000,
        READ            = 0X0080,
        // OUTP
        OUTP_OFF        = 0X0100,
        OUTP_ON         = 0X0101,
        OUTPMODE        = 0X0102,
        OUTPBAND        = 0X0108,
        OUTPCOMP        = 0X0109,
        // SETTING
        VOLT            = 0X0200,
        CURR            = 0X0201,
        IMP             = 0X0202,
        OVP             = 0X0203,
        OTP             = 0X0204,
        SetLoadCurr     = 0X0205,
        SetPowOPP       = 0X0209,
        // Control
        EnLOAD          = 0X0300,
        EnOVP           = 0X0301,
        EnOCP           = 0X0302,
        EnOTP           = 0X0303,
        EnIMPE          = 0X0304,
        SetPON          = 0X0305,
        SavPON          = 0X0306,
        RecallPON       = 0X0307,
        LoadINDE        = 0X0308,
        LoadOCP         = 0X0309,
        LoadCurrSta     = 0X030A,
        SetOvpMod       = 0X030B,
        PowOPPMod       = 0X030C, //250630 over power protcet
        VoltTime        = 0X0311,
        CurrTime        = 0X0312,
        TherTime        = 0X0313,
        // Measurement
        RVoltage        = 0X0400,
        RCurrent        = 0X0401,
        RSCurrent       = 0X0402,
        RBT             = 0X0403,
        RHT             = 0X0404,
        RDVMACDC        = 0X0405,
        RDVMDC          = 0X0406,
        RFSpeed         = 0X0407,
        RDVMAC          = 0X0409,
        TMP1            = 0X040A,
        TMP2            = 0X040B,
        NPLC            = 0X040C,
        TMP3            = 0X040D,
        CurrRANG        = 0X040E,
        AverCount       = 0X040F,
        SetFUNC         = 0X0410,
        PCurrHIGH       = 0X0411,
        PCurrLOW        = 0X0412,
        PCurrMAX        = 0X0413,
        PCurrMIN        = 0X0414,
        PDvmHIGH        = 0X0415,
        PDvmLOW         = 0X0416,
        PDvmMAX         = 0X0417,
        PDvmMIN         = 0X0418,
        PVoltHIGH       = 0X0419,
        PVoltLOW        = 0X041A,
        PVoltMAX        = 0X041B,
        PVoltMIN        = 0X041C,
        SwePOIN         = 0X041D, // repeat cmd -> 0x0427
        SweTINT         = 0X041E,
        CurrRangTIM     = 0X041F,
        RArrVOLT        = 0X0420,
        RArrCURR        = 0X0421,
        RArrDVM         = 0X0422,
        SweOffPin       = 0X0423,
        RFanDuty        = 0X0427, // -
        RVoltADC        = 0X0430, //250113 adcoffset
        RCurrADC        = 0X0431, //250113 adcoffset
        RSCurrADC       = 0X0432, //250113 adcoffset
        RDvmADC         = 0X0436, //250113 adcoffset
        // Register
        StaREG          = 0X0500,
        EnREG           = 0X0501,
        CondREG         = 0X0502,
        ERR             = 0X0503,
        SV              = 0X0504,
        HV              = 0X0505,
        CONF            = 0X0506,
        COMREG          = 0X057F,
        // Calibrate
        CalExit         = 0X0600,
        CalInit         = 0X0601,
        CalRestore      = 0X0602,
        CalSave         = 0X0603,
        CalStarALL      = 0X0604,
        CalStarADC      = 0X0605,
        CalStarDAC      = 0X0606,
        CalStarENAB     = 0X0607,
        CalStarIMP      = 0X0608,
        CalStarDCP      = 0X0610,
        CalStarDCN      = 0X0611,
        CalCTC          = 0X0616,
        CalSTC          = 0X0617,
        CalUPTC         = 0X0618,
        // Calibration
        CalStepOffset   = 0X0700,
        CalStepItemMax  = 0X7F,
        // Trigger
        ABOR            = 0X0800,
        INIT            = 0X0801,
        CONT            = 0X0802,
        SEQ1TRIG        = 0X0803,
        SEQ2TRIG        = 0X0804,
        ACQTRIGSOUR     = 0X0805,
        COUN_COMM       = 0X0806,
        HYST_COMM       = 0X0807,
        LEV_COMM        = 0X0808,
        SLOP_COMM       = 0X0809,
        SOUR_VOLT_TRIG  = 0X080A,
        SOUR_CURR_TRIG  = 0X080B,
        SOUR_RES_TRIG   = 0X080C,
        DCQTRIGSOUR     = 0X080D,
        COUN_VOLT       = 0X080E,
        COUN_DVM        = 0X080F,
        HYST_CURR       = 0X0810,
        // ISP
        UPG51_ENISP     = 0X0900,
        UPG51_DISAISP   = 0X0901,
        UPG51_RSTISP    = 0X0902,
        // Battery
        MODE            = 0X0A00,
        STATVOLT        = 0X0A01,
        INCVOLT         = 0X0A02,
        ENDVOLT         = 0X0A03,
        OUTCURR         = 0X0A04,
        INCURR          = 0X0A05,
        STOPCURR        = 0X0A06,
        STOPMODE        = 0X0A07,
        INCTIME         = 0X0A08,
        RES             = 0X0A09,
        // SOC
        FILE            = 0X0B00,
        LENG            = 0X0B01,
        STEP            = 0X0B02,
        EDIT_Q          = 0X0B03,
        EDIT_VOIT       = 0X0B04,
        EDIT_OUTCURR    = 0X0B05,
        EDIT_INCURR     = 0X0B06,
        EDIT_RES        = 0X0B07,
        RUN_STEP        = 0X0B08,
        RUN_Q           = 0X0B09,
        FILE_SAVE       = 0X0B0A,
        FILE_CLEAR      = 0X0B0B,
        // Sequence
        DIT_FILE        = 0X0C00,
        DIT_LENG        = 0X0C01,
        DIT_CYCL        = 0X0C02,
        DIT_STEP        = 0X0C03,
        DIT_VOIT        = 0X0C04,
        DIT_OUTCURR     = 0X0C05,
        DIT_INCURR      = 0X0C06,
        DIT_RES         = 0X0C07,
        DIT_RUNT        = 0X0C08,
        DIT_LINKS       = 0X0C09,
        DIT_LINKE       = 0X0C0A,
        DIT_LINKC       = 0X0C0B,
        SQRUN_TIME      = 0X0C0C,
        SQRUN_STEP      = 0X0C0D,
        SQFILE_SAVE     = 0X0C0E,
        SQFILE_CLEAR    = 0X0C0F,
        // SN
        SN1             = 0X1001,
        SN2             = 0X1002,
        SN3             = 0X1003,
        SN4             = 0X1004,
        // ID
        IDBASE          = 0X1100,
        ID1             = 0X1101,
        ID2             = 0X1102,
        ID3             = 0X1103,
        ID4             = 0X1104,
        // TEST
        TEST_EXIT       = 0XD000,
        TEST_STAR       = 0XD001,
        TEST_SetFanDuty = 0XD002,
        TEST_SetFanSpeed= 0XD003,
        TEST_SetGPIOLow = 0XD004,
        TEST_SetGPIOHigh= 0XD005,
        // DEBUG
        DEBUG_EEP       = 0XDB00,
        DEBUG_DATA      = 0XDB01,
    };
    namespace Err {
        constexpr quint8 Error                       = 0xFF;
        constexpr quint8 CheckSum_Error              = 0x00;
        constexpr quint8 Unknow_Command              = 0x01;
        constexpr quint8 Unknow_Function             = 0x02;
        constexpr quint8 Error_Length                = 0x03;
        constexpr quint8 Invalid_Parameter           = 0x04;
        constexpr quint8 Illegal_Command             = 0x05;
        constexpr quint8 Unsupported_Command         = 0x06;
    }

    namespace Param {
        constexpr quint8 A            = 0x00;
        constexpr quint8 mA           = 0x01;
        constexpr quint8 Auto         = 0x10;
    }
}

constexpr quint16 QCmd(quint16 cmds) {
    return cmds | JV6xxx::UartCMD::READ;
}
