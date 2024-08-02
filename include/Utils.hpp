#pragma once

#include <QVariantMap>
#include <Windows.h>
#include <array>
#include <cstdint>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

struct WELD_INFO {
    int   m_nType          = {}; // 轨型
    char  m_szSerial[24]   = {}; // 编号
    char  res0[10]         = {}; // 编号
    char  m_szRail[20]     = {}; // 轨条号
    char  res1[26]         = {};
    int   m_lTime          = {}; // 探伤日期
    int   m_nNumOfDay      = {}; // 当天的探伤序号
    short m_nResult        = {}; // 探伤结果
    short res              = {};
    char  res2[4]          = {};
    char  res3[4]          = {}; // 线路名
    int   m_nWeldType      = {}; // 焊缝类型
    char  m_szLineName[24] = {}; // 线路名
    char  res4[72]         = {}; // 线路名
    char  res5[4]          = {}; // 线路名
    int   m_nLinePos1      = {}; // 位置1
    int   m_nLinePos2      = {}; // 位置2
    int   m_nLine          = {}; // 线路序号
    short m_nDaobie1       = {}; // 道别，上下行,0上1下
    short m_nDaobie2       = {}; // 道别，1道，2道。。。，0无道
    short m_nGubie         = {}; // 股别，左右轨。。。
    short s2               = {};
    int   m_ShowMsg        = {}; // 信息显示 0 全显示 1显示部分
    int   s4               = {};
};

struct CH_PARA {
    int   m_days       = {}; // 0表示日常性能,>0否表示灵敏度校验
    BOOL  m_IsDayTest  = {}; // 表示当天有无校过
    TCHAR m_time[20]   = {}; // 测试时间
    TCHAR m_person[20] = {}; // 测试人员姓名或工号
    int   m_rbtype     = {}; // 测试实物试块型号
    TCHAR m_aim[20]    = {}; // 探测部位
    TCHAR m_sbtype[20] = {}; // 标准试块型号
    TCHAR m_sbaim[20]  = {}; // 标准试块部位
    TCHAR m_depth[20]  = {}; // 探测深度；
    int   m_ch         = {}; // 测试通道——以下为一个通道的数据；
    int   m_sbhigh     = {}; // 标准试块波高
    int   m_sbcomp     = {}; // 标准试块补偿
    int   m_rbhigh     = {}; // 实物试块波高
    ///////////////////////////////////////////////////////////////////////
    int nCh_logic  = {}; // 逻辑通道号
    int nCh_tr     = {}; // 发射,最多256
    int nCh_re     = {}; // 接受,最多256
    int nProbeType = {}; // 探头类型
    int nVoltage   = {}; // 发射电压0=100v,1=200v,3=300v,4=400v
    int nRepeat    = {}; // 重复频率
    int nDamp      = {}; // 探头阻尼
    int nFilter    = {}; // 滤波
    int nAsmode    = {}; // 检波方式

    int nScale    = {}; // 声程标度
    int nPreProbe = {}; // 探头前沿

    int nBGain       = {};
    int nCGain       = {};
    int nSGain       = {};
    int nRange       = {};
    int nDelay       = {};
    int nZero        = {};
    int nAngle       = {};
    int nSpeed1      = {}; // 液体声速
    int nSpeed2      = {}; // 工件声速
    int nReject      = {};
    int nThick1      = {}; // 液体厚度
    int nThick2      = {}; // 工件厚度
    int nAlarmHeight = {}; // 报警高度
    int nGposi[5]    = {};
    int nGwide[5]    = {};
    int nGhigh[5]    = {};
    int nGtype[5]    = {}; // 当前门类型0进波1失波
    int nGalarm[5]   = {}; // 报警，－1不报警0全,1门A，2门B，3门C

    int nAmpPos[5] = {}; // 实时的门内最高波的位置(时间)和高度
    int nAmpMax[5] = {};

    int nAmpDire[5] = {}; // 实时的门内最高波的距离
    int nAmpVert[5] = {}; // 实时的门内最高波的垂直
    int nAmpHori[5] = {}; // 实时的门内最高波的水平

    int nEncoder  = {}; // 探头到零点时编码器读数
    int nEndcoder = {}; // 探头到终点时编码器读数

    unsigned int nChannelStartTime   = {}; // 每个通道的开始时间
    unsigned int nChannelDelayTime   = {}; // 每个通道的结束时间
    unsigned int nChannelElapsedTime = {}; // 持续时长

    int bTrack      = {}; // 是否允许界面跟踪
    int bTrackGate  = {}; // 界面跟踪的门
    int nTrackPosi  = {}; // 界面跟踪时波峰距门位的距离，用百分比表示
    int nTrackRange = {}; // 界面跟踪时门可移动的点数，用长度表示

    int nDacDist[10] = {};
    int nDacDb[10]   = {};
    int nDacNum      = {};
};

struct DefectInfo {
    int    index      = -1;
    double pos        = 0;
    double equivalent = 0; // 波高(0-1) 或者 ΔdB
    bool   isDB       = false;

    struct ResultPos {
        double pos_start      = 0;
        double pos_end        = 0;
        double equivalent_max = 0;
        bool   isDB           = false;
    };

    struct ResultIndex {
        int    index_start    = 0;
        int    index_end      = 0;
        double equivalent_max = 0;
        bool   isDB           = false;
    };

    static std::vector<ResultPos>   GetResultByPos(const std::vector<DefectInfo>& data, double bias = 0.01) noexcept;
    static std::vector<ResultIndex> GetResultByIndex(const std::vector<DefectInfo>& data, int bias = 5) noexcept;
};

struct FILE_RES {
    BITMAPFILEHEADER         bitmapFileHeader = {};
    BITMAPINFOHEADER         bitmapInfoHeader = {};
    std::array<RGBQUAD, 256> palette          = {};
    std::vector<uint8_t>     image            = {}; // 图像

    int32_t                                        coderMin             = {};
    int32_t                                        coderMax             = {};
    int32_t                                        channelMax           = {};
    int32_t                                        dataNumberPerChannel = {};
    std::array<RECT, 3>                            rectCScan            = {}; ///< 3个C扫区域的大小
    uint32_t                                       bScanMode            = {}; ///< B扫模式
    WELD_INFO                                      weldInfo             = {};
    std::array<CH_PARA, 8>                         channelParam         = {};
    std::string                                    date                 = {};
    std::string                                    time                 = {};
    std::vector<std::vector<std::vector<uint8_t>>> aScanData            = {}; ///< A扫数据

    static std::unique_ptr<FILE_RES> FromFile(const std::wstring& file_name) noexcept;

    static bool RenderExcel(
        const std::wstring&                           file_path,
        const std::vector<std::shared_ptr<FILE_RES>>& data_vec,
        std::optional<double>                         overrid_value = std::nullopt) noexcept;

    using _T_R_POS = std::vector<DefectInfo::ResultPos>;
    using _T_R_IDX = std::vector<DefectInfo::ResultIndex>;

    using _T_RESULT = _T_R_IDX;
    using _G_RT     = std::array<_T_RESULT, 6>;

    _G_RT GetResultFromGate(std::optional<double> gate_height = std::nullopt) const noexcept;
    _G_RT GetResultFromDAC(std::optional<double> dac_soft_gain = std::nullopt) const noexcept;

    bool                                         hasDacLine(int channel) const noexcept;
    std::optional<std::function<double(double)>> getDacLine(int channel) const noexcept;
};
