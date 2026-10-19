#pragma once
#include <QString>

namespace JV6xxx {
    constexpr int Thread_CHSUM   = 0x09;
    constexpr quint8 HEADER_HIGH = 0xAA;
    constexpr quint8 HEADER_LOW  = 0x55;
    constexpr quint8 END_MARKER  = 0xEE;

    enum Cmd : quint8 {
        OUTPUT      = 0x01,
        SETTING     = 0x02,
        CONTROL     = 0x03,
        MEASUREMENT = 0x04,
        REGISTER    = 0x05,
        CALIBRATE   = 0x06,
        CALIBRATION = 0x07,
        TRIGGER     = 0x08,
        ISP         = 0x09,
        BATTERY     = 0x0A,
        SOC         = 0x0B,
        SEQUENCE    = 0x0C,
        SN          = 0x10,
        ID          = 0x11,
        READ        = 0x24,
        CONFIG      = 0x12,
        TEST        = 0xd0,
        DEBUG       = 0xdb,
        ERROR       = 0xFF
    };

    namespace Output {
        constexpr quint8 QUERY_OUTPSTATUS            = 0x80;
        constexpr quint8 QUERY_OUTPMODE              = 0x82;
        constexpr quint8 QUERY_OUTPBAND              = 0x88;
        constexpr quint8 QUERY_OUTPCOMP_MODE         = 0x89;

        constexpr quint8 SET_OUTPSTATUS_OFF          = 0x00;
        constexpr quint8 SET_OUTPSTATUS_ON           = 0x01;
        constexpr quint8 SET_OUTPMODE                = 0x02;
        constexpr quint8 SET_OUTPBAND                = 0x08;
        constexpr quint8 SET_OUTPCOMP_MODE           = 0x09;
    }

    namespace Setting {
        constexpr quint8 QUERY_SOURVOLT              = 0x80;
        constexpr quint8 QUERY_SOURCURR              = 0x81;
        constexpr quint8 QUERY_SOURIMP               = 0x82;
        constexpr quint8 QUERY_SOURVOLT_PROT         = 0x83;
        constexpr quint8 QUERY_THERPROT              = 0x84;
        constexpr quint8 QUERY_LOADCURR              = 0x85;

        constexpr quint8 SET_SOURVOLT                = 0x00;
        constexpr quint8 SET_SOURCURR                = 0x01;
        constexpr quint8 SET_SOURIMP                 = 0x02;
        constexpr quint8 SET_SOURVOLT_PROT           = 0x03;
        constexpr quint8 SET_THERPROT                = 0x04;
        constexpr quint8 SET_LOADCURR                = 0x05;
    }

    namespace Control {
        constexpr quint8 QUERY_SOURVOLT_CLAM         = 0x81;
        constexpr quint8 QUERY_SOURCURR_TYPE         = 0x82;
        constexpr quint8 QUERY_SYSTPOS               = 0x85;
        constexpr quint8 QUERY_LOADINDE_STAT         = 0x88;
        constexpr quint8 QUERY_LOADCURR_TYPE         = 0x89;
        constexpr quint8 QUERY_SOURCURR_PRIO         = 0x8a;
        constexpr quint8 QUERY_SOURVOLT_ABS          = 0x8b;
        constexpr quint8 QUERY_SOURVOLT_TIME         = 0x91;
        constexpr quint8 QUERY_SOURCURR_TIME         = 0x92;
        constexpr quint8 QUERY_THERTIME              = 0x93;

        constexpr quint8 SET_SOURVOLT_CLAM           = 0x01;
        constexpr quint8 SET_SOURCURR_TYPE           = 0x02;
        constexpr quint8 SET_SYSTPOS                 = 0x05;
        constexpr quint8 SET_SAV                     = 0x06;
        constexpr quint8 SET_RCL                     = 0x07;
        constexpr quint8 SET_LOADINDE_STAT           = 0x08;
        constexpr quint8 SET_LOADCURR_TYPE           = 0x09;
        constexpr quint8 SET_SOURCURR_PRIO           = 0x0a;
        constexpr quint8 SET_SOURVOLT_ABS            = 0x0b;
        constexpr quint8 SET_SOURVOLT_TIME           = 0x11;
        constexpr quint8 SET_SOURCURR_TIME           = 0x12;
        constexpr quint8 SET_THERTIME                = 0x13;
    }

    namespace Measurement { // not match new protocol
        constexpr quint8 QUERY_MEASVOLT              = 0x80;
        constexpr quint8 QUERY_MEASCURR              = 0x81;
        constexpr quint8 QUERY_MEASSCUR              = 0x82;
        constexpr quint8 QUERY_MEASBTMP              = 0x83;
        constexpr quint8 QUERY_MEASHTMP              = 0x84;
        constexpr quint8 QUERY_MEASDVM_ACDC          = 0x85;
        constexpr quint8 QUERY_MEASDVM               = 0x86;
        constexpr quint8 QUERY_THERFAN               = 0x87;
        constexpr quint8 QUERY_MEASDVM_AC            = 0x89;
        constexpr quint8 QUERY_MEASTMP1              = 0x8A;
        constexpr quint8 QUERY_MEASTMP2              = 0x8B;
        constexpr quint8 QUERY_SENSNPLC              = 0x8C;
        constexpr quint8 QUERY_MEASTMP3              = 0x8D;
        constexpr quint8 QUERY_SENSCURR_RANGE        = 0x8E;
        constexpr quint8 QUERY_SENSAVER              = 0x8F;
        constexpr quint8 QUERY_SENSFUNC              = 0x90;
        constexpr quint8 QUERY_FETCCURR_HIGH         = 0x91;
        constexpr quint8 QUERY_FETCCURR_LOW          = 0x92;
        constexpr quint8 QUERY_FETCCURR_MAX          = 0x93;
        constexpr quint8 QUERY_FETCCURR_MIN          = 0x94;
        constexpr quint8 QUERY_FETCDVM_HIGH          = 0x95;
        constexpr quint8 QUERY_FETCDVM_LOW           = 0x96;
        constexpr quint8 QUERY_FETCDVM_MAX           = 0x97;
        constexpr quint8 QUERY_FETCDVM_MIN           = 0x98;
        constexpr quint8 QUERY_FETCVOLT_HIGH         = 0x99;
        constexpr quint8 QUERY_FETCVOLT_LOW          = 0x9A;
        constexpr quint8 QUERY_FETCVOLT_MAX          = 0x9B;
        constexpr quint8 QUERY_FETCVOLT_MIN          = 0x9C;
        constexpr quint8 QUERY_THERDUTY              = 0x9D; // repeat
        constexpr quint8 QUERY_SENSSWE_TINT          = 0x9E;
        constexpr quint8 QUERY_SENSCURR_RANGE_TIME   = 0x9F;
        constexpr quint8 QUERY_MEASARR_VOLT          = 0xA0;
        constexpr quint8 QUERY_MEASARR_CURR          = 0xA1;
        constexpr quint8 QUERY_MEASARR_DVM           = 0xA2;
        constexpr quint8 QUERY_SENSSWE_OFFS_POIN     = 0xA3;
        constexpr quint8 QUERY_MEASADC_VOLT          = 0xB0;
        constexpr quint8 QUERY_MEASADC_CURR          = 0xB1;
        constexpr quint8 QUERY_MEASADC_SCUR          = 0xB2;
        constexpr quint8 QUERY_MEASADC_DVM           = 0xB6;

        constexpr quint8 SET_SENSNPLC                = 0x0C;
        constexpr quint8 SET_SENSCURR_RANGE          = 0x0E;
        constexpr quint8 SET_SENSAVER                = 0x0F;
        constexpr quint8 SET_SENSFUNC                = 0x10;
        constexpr quint8 SET_SENSSWE_POIN            = 0x1D;
        constexpr quint8 SET_SENSSWE_TINT            = 0x1E;
        constexpr quint8 SET_SENSCURR_RANGE_TIM      = 0x1F;
        constexpr quint8 SET_SENSSWE_OFFS_POIN       = 0x23;
    }

    namespace Register {
        constexpr quint8 QUERY_STATOPER_EVEN         = 0x80;
        constexpr quint8 QUERY_SYSTENAB              = 0x81;
        constexpr quint8 QUERY_STATOPER_COND         = 0x82;
        constexpr quint8 QUERY_STATQUE_NEXT          = 0x83;
        constexpr quint8 QUERY_SYSTSWV               = 0x84;
        constexpr quint8 QUERY_SYSTHWV               = 0x85;
        constexpr quint8 QUERY_SYSTCONF              = 0x86;
        constexpr quint8 QUERY_SYSTREG               = 0xff;

        constexpr quint8 SET_STATQUE_CLEAR           = 0x03;
    }

    namespace Calibrate {
        constexpr quint8 QUERY_CALISTAR_ALL          = 0x84;

        constexpr quint8 SET_CALIEXIT                = 0x00;
        constexpr quint8 SET_CALIINIT                = 0x01;
        constexpr quint8 SET_CALIREST                = 0x02;
        constexpr quint8 SET_CALISAVE                = 0x03;
        constexpr quint8 SET_CALISTART_ALL           = 0x04;
        constexpr quint8 SET_CALISTART_ADC           = 0x05;
        constexpr quint8 SET_CALISTART_DAC           = 0x06;
        constexpr quint8 SET_CALITART_ENAB           = 0x07;
        constexpr quint8 SET_CALISTART_IMP           = 0x08;
        constexpr quint8 SET_CALISTART_DCP           = 0x10;
        constexpr quint8 SET_CALISTART_DCN           = 0x11;
        constexpr quint8 SET_CALICLEARTC             = 0x16;
        constexpr quint8 SET_CALISAVETC              = 0x17;
        constexpr quint8 SET_CALIUPDATETC            = 0x18;
    }

    namespace Trigger { // not match new protocol
        constexpr quint8 QUERY_TRIGSEQ2_SOUR         = 0x85;
        constexpr quint8 QUERY_TRIGSEQ2_COUN         = 0x86;
        constexpr quint8 QUERY_TRIGSEQ2_HYST         = 0x87;
        constexpr quint8 QUERY_TRIGSEQ2_LEV          = 0x88;
        constexpr quint8 QUERY_TRIGSEQ2_SLOP         = 0x89;
        constexpr quint8 QUERY_SOURVOLT_AMPL_TRIG    = 0x8A;
        constexpr quint8 QUERY_SOURCURR_TRIG         = 0x8B;
        constexpr quint8 QUERY_SOURRES_TRIG          = 0x8C;
        constexpr quint8 QUERY_TRIGSEQ1_SOUR         = 0x8D;
        constexpr quint8 QUERY_TRIGSEQ2_COUN_VOIT    = 0x8E;
        constexpr quint8 QUERY_TRIGSEQ2_COUN_DVM     = 0x8F;
        constexpr quint8 QUERY_TRIGSEQ2_HYST_CURR    = 0x90;

        constexpr quint8 SET_ABORT                   = 0x00;
        constexpr quint8 SET_INIT_IMM                = 0x01;
        constexpr quint8 SET_INIT_CONT               = 0x02;
        constexpr quint8 SET_TRIGSEQ1                = 0x03;
        constexpr quint8 SET_TRIGSEQ2                = 0x04;
        constexpr quint8 SET_TRIGSEQ2_SOUR           = 0x05;
        constexpr quint8 SET_TRIGSEQ2_COUN           = 0x06;
        constexpr quint8 SET_TRIGSEQ2_HYST           = 0x07;
        constexpr quint8 SET_TRIGSEQ2_LEV            = 0x08;
        constexpr quint8 SET_TRIGSEQ2_SLOP           = 0x09;
        constexpr quint8 SET_SOURVOLT_AMPL_TRIG      = 0x0A;
        constexpr quint8 SET_SOURCURR_TRIG           = 0x0B;
        constexpr quint8 SET_SOURRES_TRIG            = 0x0C;
        constexpr quint8 SET_TRIGSEQ1_SOUR           = 0x0D;
        constexpr quint8 SET_TRIGSEQ2_COUN_VOIT      = 0x0E;
        constexpr quint8 SET_TRIGSEQ2_COUN_DVM       = 0x0F;
        constexpr quint8 SET_TRIGSEQ2_HYST_CURR      = 0x10;
    }

    namespace Battery {
        constexpr quint8 QUERY_BATTMODE              = 0x80;
        constexpr quint8 QUERY_BATTSTATVOLT          = 0x81;
        constexpr quint8 QUERY_BATTINCVOLT           = 0x82;
        constexpr quint8 QUERY_BATTENDVOLT           = 0x83;
        constexpr quint8 QUERY_BATTOUTCURR           = 0x84;
        constexpr quint8 QUERY_BATTINCURR            = 0x85;
        constexpr quint8 QUERY_BATTSTOPCURR          = 0x86;
        constexpr quint8 QUERY_BATTSTOPMODE          = 0x87;
        constexpr quint8 QUERY_BATTINCTIME           = 0x88;
        constexpr quint8 QUERY_BATTRES               = 0x89;

        constexpr quint8 SET_BATTMODE                = 0x00;
        constexpr quint8 SET_BATTSTATVOLT            = 0x01;
        constexpr quint8 SET_BATTINCVOLT             = 0x02;
        constexpr quint8 SET_BATTENDVOLT             = 0x03;
        constexpr quint8 SET_BATTOUTCURR             = 0x04;
        constexpr quint8 SET_BATTINCURR              = 0x05;
        constexpr quint8 SET_BATTSTOPCURR            = 0x06;
        constexpr quint8 SET_BATTSTOPMODE            = 0x07;
        constexpr quint8 SET_BATTINCTIME             = 0x08;
        constexpr quint8 SET_BATTRES                 = 0x09;
    }

    namespace Soc {
        constexpr quint8 QUERY_SOCEDIT_FILE          = 0x80;
        constexpr quint8 QUERY_SOCEDIT_LENG          = 0x81;
        constexpr quint8 QUERY_SEQEDIT_STEP          = 0x82;
        constexpr quint8 QUERY_SOCEDIT_Q             = 0x83;
        constexpr quint8 QUERY_SOCEDIT_VOIT          = 0x84;
        constexpr quint8 QUERY_SOCEDIT_OUTCURR       = 0x85;
        constexpr quint8 QUERY_SOCEDIT_INCURR        = 0x86;
        constexpr quint8 QUERY_SOCEDIT_RES           = 0x87;
        constexpr quint8 QUERY_SOCRUN_STEP           = 0x88;
        constexpr quint8 QUERY_SOCRUN_Q              = 0x89;
        constexpr quint8 QUERY_SOCFILE_SAVE          = 0x8A;
        constexpr quint8 QUERY_SOCFILE_CLEAR         = 0x8B;

        constexpr quint8 SET_SOCEDIT_FILE            = 0x00;
        constexpr quint8 SET_SOCEDIT_LENG            = 0x01;
        constexpr quint8 SET_SEQEDIT_STEP            = 0x02;
        constexpr quint8 SET_SOCEDIT_Q               = 0x03;
        constexpr quint8 SET_SOCEDIT_VOIT            = 0x04;
        constexpr quint8 SET_SOCEDIT_OUTCURR         = 0x05;
        constexpr quint8 SET_SOCEDIT_INCURR          = 0x06;
        constexpr quint8 SET_SOCEDIT_RES             = 0x07;
    }

    namespace Sequence {
        constexpr quint8 QUERY_SEQEDIT_FILE          = 0x80;
        constexpr quint8 QUERY_SEQEDIT_LENG          = 0x81;
        constexpr quint8 QUERY_SEQEDIT_CYCL          = 0x82;
        constexpr quint8 QUERY_SEQEDIT_STEP          = 0x83;
        constexpr quint8 QUERY_SEQEDIT_VOIT          = 0x84;
        constexpr quint8 QUERY_SEQEDIT_OUTCURR       = 0x85;
        constexpr quint8 QUERY_SEQEDIT_INCURR        = 0x86;
        constexpr quint8 QUERY_SEQEDIT_RES           = 0x87;
        constexpr quint8 QUERY_SEQEDIT_RUNT          = 0x88;
        constexpr quint8 QUERY_SEQEDIT_LINKS         = 0x89;
        constexpr quint8 QUERY_SEQEDIT_LINKE         = 0x8A;
        constexpr quint8 QUERY_SEQEDIT_LINKC         = 0x8B;
        constexpr quint8 QUERY_SEQRUN_TIME           = 0x8C;
        constexpr quint8 QUERY_SEQRUN_STEP           = 0x8D;
        constexpr quint8 QUERY_SEQFILE_SAVE          = 0x8E;
        constexpr quint8 QUERY_SEQFILE_CLEAR         = 0x8F;

        constexpr quint8 SET_SEQEDIT_FILE            = 0x00;
        constexpr quint8 SET_SEQEDIT_LENG            = 0x01;
        constexpr quint8 SET_SEQEDIT_CYCL            = 0x02;
        constexpr quint8 SET_SEQEDIT_STEP            = 0x03;
        constexpr quint8 SET_SEQEDIT_VOIT            = 0x04;
        constexpr quint8 SET_SEQEDIT_OUTCURR         = 0x05;
        constexpr quint8 SET_SEQEDIT_INCURR          = 0x06;
        constexpr quint8 SET_SEQEDIT_RES             = 0x07;
        constexpr quint8 SET_SEQEDIT_RUNT            = 0x08;
        constexpr quint8 SET_SEQEDIT_LINKS           = 0x09;
        constexpr quint8 SET_SEQEDIT_LINKE           = 0x0A;
        constexpr quint8 SET_SEQEDIT_LINKC           = 0x0B;
    }

    namespace Param {
        constexpr quint8 A            = 0x00;
        constexpr quint8 mA           = 0x01;
        constexpr quint8 Auto         = 0x10;
    }
}
