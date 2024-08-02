
#include "Utils.hpp"
#include <UnionType>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <xlsxdocument.h>

#include <math.h>
#include <stdarg.h>

#pragma warning(disable :4244)
#include <stdio.h>

namespace fs = std::filesystem;

#define N         4096 /* size of ring buffer */
#define F         18   /* upper limit for match_length */
#define THRESHOLD 2    /* encode string into position and length \
                          if match_length is greater than this */
#define NIL       N    /* index for root of binary search trees,NIL用来表示空指针 */

void InitTree(void); /* initialize trees */
void InsertNode(int r);
void DeleteNode(int p); /* deletes node p from tree */

unsigned long int
    textsize   = 0, /* text size counter */
    codesize   = 0, /* code size counter */
    printcount = 0; /* counter for reporting progress every 1K bytes */
unsigned char
    text_buf[N + F - 1]; /* ring buffer of size N,
     with extra F-1 bytes to facilitate string comparison
     定义的滑动窗口，最多容纳N个互相重叠的字符串，每个字符串的长度是F，第一个字符串的偏移
     是0～F-1，最后一个字符串的偏移是N-1～(N-1)+(F-1)。所以，为了把最后一个字符串完整
     包含在内，textbuf[]的大小设为N+F-1
//*/

int match_position, match_length;             /* of longest match.  These are
                   set by the InsertNode() procedure. */
short lson[N + 1], rson[N + 257], dad[N + 1]; /* left & right children &
         parents -- These constitute binary search trees.
         右子树大256，用来表示0－255个字符开始的树,
         每棵树的根指针存储在rson数组的最后256个元素中
//*/
// FILE	*infile, *outfile;  /* input & output files */

void InitTree(void) /* initialize trees */
{
    int i;

    /* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
       left children of node i.  These nodes need not be initialized.
       Also, dad[i] is the parent of node i.  These are initialized to
       NIL (= N), which stands for 'not used.'
       For i = 0 to 255, rson[N + i + 1] is the root of the tree
       for strings that begin with character i.  These are initialized
       to NIL.  Note there are 256 trees. */

    for (i = N + 1; i <= N + 256; i++)
        rson[i] = NIL; // 树的根节点
    for (i = 0; i < N; i++)
        dad[i] = NIL; // 父节点
}

void InsertNode(int r) // 插入节点
/* Inserts string of length F, text_buf[r..r+F-1], into one of the
   trees (text_buf[r]'th tree) and returns the longest-match position
   and length via the global variables match_position and match_length.
   If match_length = F, then removes the old node in favor of the new
   one, because the old one will be deleted sooner.
   Note r plays double role, as tree node and position in buffer. */
{
    int            i, p, cmp;
    unsigned char* key;

    cmp     = 1;
    key     = &text_buf[r];
    p       = N + 1 + key[0];
    rson[r] = lson[r] = NIL;
    match_length      = 0;
    /*
    cmp的初始值大于1，然后令p = N + 1 + key[0]，这时p的确大于N，但因为cmp>0，下面
    的代码在第一次循环中只会在rson[]超出N的那256个位置查找。还记得我上次画的图吗？
    这其实就是确定字符串应该插入到256棵树中的哪棵树里。确定之后，后续的查找过程就
    进入了那棵树的内部，p的值也就不会大于N了。
    //*/
    for (;;) {
        if (cmp >= 0) {
            if (rson[p] != NIL)
                p = rson[p];
            else {
                rson[p] = r;
                dad[r]  = p;
                return;
            }
        } else {
            if (lson[p] != NIL)
                p = lson[p];
            else {
                lson[p] = r;
                dad[r]  = p;
                return;
            }
        }
        for (i = 1; i < F; i++) {
            if ((cmp = key[i] - text_buf[p + i]) != 0)
                break;
        }
        if (i > match_length) {
            match_position = p;
            if ((match_length = i) >= F)
                break;
        }
    }
    dad[r]       = dad[p];
    lson[r]      = lson[p];
    rson[r]      = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
        rson[dad[p]] = r;
    else
        lson[dad[p]] = r;
    dad[p] = NIL; /* remove p */
}

void DeleteNode(int p) /* deletes node p from tree */
{
    int q;

    if (dad[p] == NIL)
        return; /* not in tree */
    if (rson[p] == NIL)
        q = lson[p];
    else if (lson[p] == NIL)
        q = rson[p];
    else {
        q = lson[p];
        if (rson[q] != NIL) {
            do {
                q = rson[q];
            } while (rson[q] != NIL);
            rson[dad[q]] = lson[q];
            dad[lson[q]] = dad[q];
            lson[q]      = lson[p];
            dad[lson[p]] = q;
        }
        rson[q]      = rson[p];
        dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if (rson[dad[p]] == p)
        rson[dad[p]] = q;
    else
        lson[dad[p]] = q;
    dad[p] = NIL;
}

// 解压缩
int Decode(BYTE* src, int srclen, BYTE* dest, int deslength) {
    int          i, j, k, r, c;
    unsigned int flags;
    int          soulen, destlen;

    soulen = destlen = 0;
    r                = *(src + 0);
    c                = *(src + 1);
    if (r != 0x55 && c != 0xaa) { // 未压缩
        memcpy(dest, src, srclen);
        return srclen;
    }
    soulen = 2; // 压缩文件除掉开始的两个标志位

    for (i = 0; i < N - F; i++)
        dest[i] = ' ';
    r     = N - F;
    flags = 0;
    for (;;) {
        if (((flags >>= 1) & 256) == 0) {
            if (soulen >= srclen)
                break;
            c = *(src + soulen);
            soulen++;
            /// if ((c = getc(infile)) == EOF) break;
            flags = c | 0xff00; /* uses higher byte cleverly */
        } /* to count eight */
        if (flags & 1) {
            if (soulen >= srclen)
                break;
            c = *(src + soulen);
            soulen++;
            /// if ((c = getc(infile)) == EOF) break;
            /// putc(c, outfile);
            if ((dest) != NULL) {
                try {
                    //	OutputDebugPrintf(" \n dest=%d(;dest+destlen)=%d;destlen%d;srclen%d", dest,dest+destlen,destlen,srclen);
                    //		OutputDebugPrintf(" \n %d",deslength);
                    //		TRACE("dest=%d(;dest+destlen)=%d;*( dest+destlen)%d", dest,dest+destlen,*( dest+destlen));
                    // printf("Out: %ld bytes\n", dest);
                    //	if( destlen >= srclen)break;
                    // if (*(dest + destlen)==NULL){
                    if (destlen < deslength) {
                        *(dest + destlen) = c;
                        destlen++;
                    }
                    //	}

                } catch (...) // 这里不晓得什么类型可用三点
                {
                }
            }

            text_buf[r++] = c;
            r &= (N - 1);
        } else {
            if (soulen >= srclen)
                break;
            i = *(src + soulen);
            soulen++;
            if (soulen >= srclen)
                break;
            j = *(src + soulen);
            soulen++;
            /// if ((i = getc(infile)) == EOF) break;
            /// if ((j = getc(infile)) == EOF) break;
            i |= ((j & 0xf0) << 4);
            j = (j & 0x0f) + THRESHOLD;
            for (k = 0; k <= j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                /// putc(c, outfile);
                if (destlen < deslength) {
                    *(dest + destlen) = c;
                    destlen++;
                }
                text_buf[r++] = c;
                r &= (N - 1);
            }
        }
    }
    return destlen;
}

std::unique_ptr<FILE_RES> FILE_RES::FromFile(const std::wstring& file_name) noexcept {
    std::ifstream             file(file_name, std::ios::binary);
    std::unique_ptr<FILE_RES> ret = nullptr;
    if (file.is_open()) {
        ret = std::make_unique<FILE_RES>();
        file.read(reinterpret_cast<char*>(&ret->bitmapFileHeader), sizeof(BITMAPFILEHEADER));
        file.read(reinterpret_cast<char*>(&ret->bitmapInfoHeader), sizeof(BITMAPINFOHEADER));
        file.read(reinterpret_cast<char*>(&ret->palette), sizeof(ret->palette));

        file.seekg(ret->bitmapFileHeader.bfOffBits, std::ios::beg);
        ret->image.resize(ret->bitmapInfoHeader.biSizeImage);
        file.read(reinterpret_cast<char*>(ret->image.data()), ret->bitmapInfoHeader.biSizeImage);
        file.seekg(ret->bitmapFileHeader.bfOffBits + ret->bitmapInfoHeader.biSizeImage, std::ios::beg);

        file.read(reinterpret_cast<char*>(&ret->coderMin), sizeof(ret->coderMin));
        file.read(reinterpret_cast<char*>(&ret->coderMax), sizeof(ret->coderMax));
        file.read(reinterpret_cast<char*>(&ret->channelMax), sizeof(ret->channelMax));
        file.read(reinterpret_cast<char*>(&ret->dataNumberPerChannel), sizeof(ret->dataNumberPerChannel));
        file.read(reinterpret_cast<char*>(&ret->rectCScan), sizeof(ret->rectCScan));
        file.read(reinterpret_cast<char*>(&ret->bScanMode), sizeof(ret->bScanMode));
        file.read(reinterpret_cast<char*>(&ret->weldInfo), sizeof(ret->weldInfo));
        file.read(reinterpret_cast<char*>(&ret->channelParam), sizeof(ret->channelParam));
        char buf[20] = {};
        file.read(buf, 20);
        std::stringstream ss;
        ss << buf;
        ss >> ret->date;
        uint32_t ts;
        ss >> ts;
        int h       = ts / 100;
        int m       = ts % 100;
        ret->time   = std::to_string(h) + ":" + std::to_string(m);
        int lenEcho = 0;
        file.read(reinterpret_cast<char*>(&lenEcho), sizeof(lenEcho));
        auto echo = std::make_unique<uint8_t[]>(lenEcho);
        file.read(reinterpret_cast<char*>(echo.get()), lenEcho);
        auto unziped_size = ret->channelMax * ret->dataNumberPerChannel * ret->coderMax;
        auto unziped      = std::make_unique<uint8_t[]>(unziped_size);
        Decode(echo.get(), lenEcho, unziped.get(), unziped_size);
        ret->aScanData.resize(ret->coderMax);
        for (int coder = 0; coder < ret->coderMax; coder++) {
            ret->aScanData[coder].resize(ret->channelMax);
            for (int ch = 0; ch < ret->channelMax; ch++) {
                ret->aScanData[coder][ch].resize(ret->dataNumberPerChannel);
                auto ptr = unziped.get() + ch * (ret->coderMax * ret->dataNumberPerChannel) + coder * ret->dataNumberPerChannel;
                memcpy(ret->aScanData[coder][ch].data(), ptr, ret->dataNumberPerChannel);
            }
        }
    }
    return ret;
}

bool FILE_RES::RenderExcel(const std::wstring&                           file_path,
                           const std::vector<std::shared_ptr<FILE_RES>>& data_vec,
                           std::optional<double>                         overrid_value) noexcept {
    fs::path file_dir = fs::path(file_path).parent_path();
    if (file_dir != "" && !fs::exists(file_dir)) {
        if (!fs::create_directories(file_dir)) {
            return false;
        }
    }

    QXlsx::Document doc(QString("GHT-2BReportTemplate.xlsx"));
    if (!doc.load()) {
        return false;
    }

    for (int index = 0; index < std::ssize(data_vec); index++) {
        if (data_vec[index] == nullptr) {
            continue;
        }
        _G_RT      res;
        const bool is_dac = data_vec[index]->hasDacLine(0) && data_vec[index]->hasDacLine(2) && data_vec[index]->hasDacLine(4);
        if (is_dac) {
            res = data_vec[index]->GetResultFromDAC(overrid_value);
        } else {
            res = data_vec[index]->GetResultFromGate(overrid_value);
        }
        using _T_RAIL_POS    = std::array<std::vector<std::variant<DefectInfo::ResultPos, DefectInfo::ResultIndex>>, 3>;
        _T_RAIL_POS rail_pos = {};

        // 比较当量的大小
        auto COMP_HELPER = [](const auto& lhs, const auto& rhs)
            requires std::is_same_v<std::decay_t<decltype(lhs)>, DefectInfo::ResultPos> ||
                     std::is_same_v<std::decay_t<decltype(rhs)>, DefectInfo::ResultPos> ||
                     std::is_same_v<std::decay_t<decltype(lhs)>, DefectInfo::ResultIndex> ||
                     std::is_same_v<std::decay_t<decltype(rhs)>, DefectInfo::ResultIndex>
        {
            return lhs.equivalent_max > rhs.equivalent_max;
        };

        // 计算单个缺陷的比例
        auto GET_DEFECT_RATE_HELPER = [&](const auto& val, int i) -> double
            requires std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultPos> ||
                     std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultIndex>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultPos>) {
                return val.pos_end - val.pos_start + (1 / 300.0);
            } else {
                int ch    = i * 2;
                int range = data_vec[index]->channelParam[ch].nEndcoder - data_vec[index]->channelParam[ch].nEncoder + 1;
                return static_cast<double>(val.index_end - val.index_start + 1) / static_cast<double>(range);
            }
        };

        std::array<std::pair<double, double>, 3> defect_statistics = {};

        for (int i = 0; i < 3; i++) {
            rail_pos[i].insert(rail_pos[i].end(), res[i * 2].begin(), res[i * 2].end());
            rail_pos[i].insert(rail_pos[i].end(), res[i * 2 + 1].begin(), res[i * 2 + 1].end());
            std::sort(rail_pos[i].begin(), rail_pos[i].end(), [&](const auto& lhs, const auto& rhs) {
                return std::visit(COMP_HELPER, lhs, rhs);
            });

            double __max_second_1 = 0;
            for (const auto& it : res[i * 2]) {
                auto temp = GET_DEFECT_RATE_HELPER(it, i);
                if (defect_statistics[i].first < temp) {
                    defect_statistics[i].first = temp;
                }
                __max_second_1 += temp;
            }

            double __max_second_2 = 0;
            for (const auto& it : res[i * 2 + 1]) {
                auto temp = GET_DEFECT_RATE_HELPER(it, i);
                if (defect_statistics[i].first < temp) {
                    defect_statistics[i].first = temp;
                }
                __max_second_2 += temp;
            }

            defect_statistics[i].second = std::max(__max_second_2, __max_second_1);

            if (rail_pos[i].size() > 5) {
                rail_pos[i].erase(rail_pos[i].begin() + 5, rail_pos[i].end());
            }
        }

        int idx_column = 1;
        int idx_row    = index + 5;
        doc.write(idx_row, idx_column++, index + 1);
        doc.write(idx_row, idx_column++, QString::fromLocal8Bit(data_vec[index]->channelParam[0].m_person));
        doc.write(idx_row, idx_column++, QString::fromStdString(data_vec[index]->date));
        doc.write(idx_row, idx_column++, QString::number(data_vec[index]->weldInfo.m_nNumOfDay));
        doc.write(idx_row, idx_column++, QString::fromLocal8Bit(data_vec[index]->weldInfo.m_szSerial));
        doc.write(idx_row, idx_column++, QString::fromLocal8Bit(data_vec[index]->weldInfo.m_szRail));
        doc.write(idx_row, idx_column++, data_vec[index]->weldInfo.m_nType == 0 ? 60 : 75);

        // DONE: 转换缺陷的范围
        auto CONVERT_RANGE_HELPER = [&](const auto& val, int i) -> std::pair<double, double>
            requires std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultPos> ||
                     std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultIndex>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultPos>) {
                int  ch     = i * 2;
                auto delay  = data_vec[index]->channelParam[ch].nDelay / 100.0;
                auto range  = data_vec[index]->channelParam[ch].nRange / 100.0;
                auto _start = val.pos_start * range + delay;
                auto _end   = val.pos_end * range + delay;
                return std::make_pair(_start, _end);
            } else {
                constexpr auto        RAIL_HEAD_LEN   = 73.0;
                constexpr auto        RAIL_WEB_LEN    = 176.0;
                constexpr auto        RAIL_FLANGE_LEN = 150.0;
                int                   ch              = i * 2;
                double                _start = 0, _end = 0;
                std::array<double, 2> _range = {};
                _range[0]                    = data_vec[index]->channelParam[ch].nEncoder;
                _range[1]                    = data_vec[index]->channelParam[ch].nEndcoder;

                switch (i) {
                    case 0:
                        _start = Union::ValueMap(val.index_start, {0.0, RAIL_HEAD_LEN}, _range);
                        _end   = Union::ValueMap(val.index_end, {0.0, RAIL_HEAD_LEN}, _range);
                        break;
                    case 1:
                        _start = Union::ValueMap(val.index_start, {0.0, RAIL_WEB_LEN}, _range);
                        _end   = Union::ValueMap(val.index_end, {0.0, RAIL_WEB_LEN}, _range);
                        break;
                    case 2:
                        _start = Union::ValueMap(val.index_start, {0.0, RAIL_FLANGE_LEN}, _range);
                        _end   = Union::ValueMap(val.index_end, {0.0, RAIL_FLANGE_LEN}, _range);
                        break;
                }

                return std::make_pair(_start, _end);
            }
        };

        // 返回当量的显示字符串
        auto GET_EVUIVALENT_HELPER = [&](const auto& val) -> QString
            requires std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultPos> ||
                     std::is_same_v<std::decay_t<decltype(val)>, DefectInfo::ResultIndex>
        {
            if (val.isDB) {
                return QString::number(val.equivalent_max, 'f', 1) + "dB";
            }
            return QString::number(val.equivalent_max * 100.0, 'f', 1) + "%";
        };

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 5; j++) {
                if (j >= rail_pos[i].size()) {
                    idx_column += 2;
                } else {
                    auto _HEPLER = std::bind(CONVERT_RANGE_HELPER, std::placeholders::_1, i);
                    auto range   = std::visit(_HEPLER, rail_pos[i][j]);
                    doc.write(idx_row, idx_column++, QString::asprintf("%.1fmm~%.1fmm", range.first, range.second));
                    doc.write(idx_row, idx_column++, std::visit(GET_EVUIVALENT_HELPER, rail_pos[i][j]));
                }
            }

            doc.write(idx_row, idx_column++, QString::number(defect_statistics[i].first * 100.0, 'f', 1) + "%");
            doc.write(idx_row, idx_column++, QString::number(defect_statistics[i].second * 100.0, 'f', 1) + "%");
            doc.write(idx_row, idx_column++, "");
        }
    }

    return doc.saveAs(QString::fromStdWString(file_path));
}

FILE_RES::_G_RT FILE_RES::GetResultFromDAC(std::optional<double> dac_soft_gain) const noexcept {
    if (channelMax > 6) {
        return _G_RT();
    }
    std::array<std::vector<DefectInfo>, 6> _temp;
    for (int _ch = 0; _ch < channelMax; _ch++) {
        Union::Base::Gate gate;
        gate.pos    = channelParam[_ch].nGposi[0] / 1000.0;
        gate.width  = channelParam[_ch].nGwide[0] / 1000.0;
        gate.height = channelParam[_ch].nGhigh[0] / 1000.0;

        gate.enable = true;
        for (int _coder = 0; _coder < coderMax; _coder++) {
            // 越过零点和终止点的编码区域
            if (_coder < channelParam[_ch].nEncoder || _coder > channelParam[_ch].nEndcoder) {
                continue;
            }
            auto gate_result = Union::Base::CalculateGateResult(aScanData[_coder][_ch], gate);
            if (gate_result.has_value()) {
                auto [_pos, _amp] = gate_result.value();
                if (_amp / 200.0 >= gate.height) {
                    DefectInfo info;
                    info.index         = _coder;
                    info.pos           = _pos;
                    const auto __delay = channelParam[_ch].nDelay / 100.0;
                    auto       __range = channelParam[_ch].nRange / 100.0;
                    if (_ch == 3) {
                        __range /= 2.0;
                    }

                    auto line = getDacLine(_ch);
                    if (line.has_value()) {
                        auto _x       = Union::ValueMap(_pos, std::make_pair(__delay, __range));
                        auto _dac_amp = line.value()(_x);
                        if (_dac_amp > 200.0) {
                            _dac_amp = 200;
                        }
                        info.equivalent = Union::CalculatedGain(_dac_amp, _amp);
                        info.isDB       = true;
                        if (info.equivalent + dac_soft_gain.value_or(0) >= -0.1) {
                            _temp[_ch].emplace_back(info);
                        }
                    } else {
                        info.equivalent = _amp / 200.0;
                        info.isDB       = false;
                        _temp[_ch].emplace_back(info);
                    }
                }
            }
        }
    }

    _G_RT ret;
    for (int i = 1; i < 6; i += 1) {
        // 通过编码器计算缺陷位置
        ret[i] = DefectInfo::GetResultByIndex(_temp[i], 3);
    }
    return ret;
}

std::optional<std::function<double(double)>> FILE_RES::getDacLine(int channel) const noexcept {
    if (!hasDacLine(channel)) {
        return std::nullopt;
    }
    const int           dac_number    = channelParam[channel].nDacNum;
    const double        dac_base_gain = channelParam[channel].nBGain / 10.0;
    std::vector<double> dist(dac_number);
    std::vector<double> db(dac_number);
    for (int i = 0; i < dac_number; i++) {
        dist[i] = channelParam[channel].nDacDist[i] / 10.0;
        db[i]   = channelParam[channel].nDacDb[i] / 10.0;
    }

    std::transform(db.begin(), db.end(), db.begin(), [=](double x) {
        auto _t = (48.1 + dac_base_gain - x) / 20;
        return std::pow(10, _t > 0 ? _t : 0);
    });
    std::vector<double> dacays(dac_number - 1);
    for (int i = 0; i < dac_number - 1; i++) {
        dacays[i] = (std::log(db[i + 1]) - std::log(db[i])) / (dist[i + 1] - dist[i]);
    }

    return [=](double x) -> double {
        if (x <= dist[0]) {
            return db[0];
        } else if (x >= dist[dist.size() - 1]) {
            return db[db.size() - 1];
        } else {
            for (int i = 0; i < dist.size(); i++) {
                if (x >= dist[i] && x < dist[i + 1]) {
                    return db[i] * std::exp(dacays[i] * (x - dist[i]));
                }
            }
        }
        return db[db.size() - 1];
    };
}

bool FILE_RES::hasDacLine(int channel) const noexcept {
    if (channel > 6 || channel < 0) {
        return false;
    }

    if (channelParam[channel].nDacNum > 10 || channelParam[channel].nDacNum <= 1) {
        return false;
    }
    return true;
}

FILE_RES::_G_RT FILE_RES::GetResultFromGate(std::optional<double> gate_height) const noexcept {
    if (channelMax > 6) {
        return _G_RT();
    }
    std::array<std::vector<DefectInfo>, 6> _temp;
    for (int _ch = 0; _ch < channelMax; _ch++) {
        Union::Base::Gate gate;
        gate.pos   = channelParam[_ch].nGposi[0] / 1000.0;
        gate.width = channelParam[_ch].nGwide[0] / 1000.0;
        if (gate_height.has_value()) {
            if (gate_height.value() > 1.0 && gate_height.value() < 100.0) {
                gate.height = gate_height.value() / 100.0;
            } else if (gate_height.value() > 0 && gate_height.value() < 1.0) {
                gate.height = gate_height.value();
            } else {
                gate.height = channelParam[_ch].nGhigh[0] / 1000.0;
            }
        } else {
            gate.height = channelParam[_ch].nGhigh[0] / 1000.0;
        }
        gate.enable = true;
        for (int _coder = 0; _coder < coderMax; _coder++) {
            // 越过零点和终止点的编码区域
            if (_coder < channelParam[_ch].nEncoder || _coder > channelParam[_ch].nEndcoder) {
                continue;
            }
            auto gate_result = Union::Base::CalculateGateResult(aScanData[_coder][_ch], gate);
            if (gate_result.has_value()) {
                auto [_pos, _amp] = gate_result.value();
                if (_amp / 200.0 >= gate.height) {
                    DefectInfo info;
                    info.index      = _coder;
                    info.pos        = _pos;
                    info.equivalent = _amp / 200.0;
                    info.isDB       = false;
                    _temp[_ch].emplace_back(info);
                }
            }
        }
    }

    _G_RT ret;
    for (int i = 1; i < 6; i += 1) {
        // 通过编码器计算缺陷位置
        ret[i] = DefectInfo::GetResultByIndex(_temp[i], 3);
    }
    return ret;
}

template <class T, class _D, class _M>
static std::vector<T> MergeHelper(
    std::vector<T>&                       data,
    std::function<_D&(T&)>                FIRST,
    std::function<_D&(T&)>                SECOND,
    _D                                    bias,
    std::optional<std::function<_M&(T&)>> MAX) noexcept {
    std::vector<T> ret;
    for (auto i = 0; i < data.size(); i++) {
        if ((ret.size() == 0) || (FIRST(data[i]) - SECOND(ret.back()) >= bias)) {
            ret.emplace_back(data[i]);
        }
        if ((ret.size() > 0) && (FIRST(data[i]) - SECOND(ret.back()) < bias)) {
            SECOND(ret.back()) = SECOND(data[i]);
            if (MAX.has_value()) {
                if (MAX.value()(ret.back()) < MAX.value()(data[i])) {
                    MAX.value()(ret.back()) = MAX.value()(data[i]);
                }
            }
        }
    }
    return ret;
}

std::vector<DefectInfo::ResultPos> DefectInfo::GetResultByPos(const std::vector<DefectInfo>& data, double bias) noexcept {
    __debugbreak();
    auto _copy = data;
    std::sort(_copy.begin(), _copy.end(), [](DefectInfo& a, DefectInfo& b) { return a.pos < b.pos; });

    std::vector<DefectInfo::ResultPos> ret;
    for (const auto& i : _copy) {
        DefectInfo::ResultPos res;
        res.pos_start      = i.pos;
        res.pos_end        = i.pos;
        res.equivalent_max = i.equivalent;
        res.isDB           = i.isDB;
        ret.emplace_back(res);
    }

    return MergeHelper<DefectInfo::ResultPos, double, double>(ret, [](auto& i) -> double& { return i.pos_start; }, [](auto& i) -> double& { return i.pos_end; }, bias, [](auto& i) -> double& { return i.equivalent_max; });
}

std::vector<DefectInfo::ResultIndex> DefectInfo::GetResultByIndex(const std::vector<DefectInfo>& data, int bias) noexcept {
    auto _copy = data;
    std::sort(_copy.begin(), _copy.end(), [](DefectInfo& a, DefectInfo& b) { return a.index < b.index; });

    std::vector<DefectInfo::ResultIndex> ret;
    for (const auto& i : _copy) {
        DefectInfo::ResultIndex res;
        res.index_start    = i.index;
        res.index_end      = i.index;
        res.equivalent_max = i.equivalent;
        res.isDB           = i.isDB;
        ret.emplace_back(res);
    }

    return MergeHelper<DefectInfo::ResultIndex, int, double>(ret, [](auto& i) -> int& { return i.index_start; }, [](auto& i) -> int& { return i.index_end; }, bias, [](auto& i) -> double& { return i.equivalent_max; });
}
