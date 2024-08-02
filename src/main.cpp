/* encoding: GB2312 */
#include "Utils.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <optional>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption opt_output_name("o", "output file name", "output", "export.xlsx");
    QCommandLineOption opt_override("override", "override value", "override", "0");
    parser.addPositionalArgument("inputs", "input file names", "[input file names]");
    parser.addOptions({opt_output_name,
                       opt_override});
    parser.parse(app.arguments());

    auto                  output   = parser.value(opt_output_name);
    auto                  inputs   = parser.positionalArguments();
    std::optional<double> override = std::nullopt;
    if (parser.isSet(opt_override)) {
        bool _ok = false;
        auto _v  = parser.value("override").toDouble(&_ok);
        if (_ok) {
            override = _v;
        }
    }
    std::vector<std::shared_ptr<FILE_RES>> file_vec;

    for (auto& input : inputs) {
        file_vec.emplace_back(FILE_RES::FromFile(input.toStdWString()));
    }

    return FILE_RES::RenderExcel(output.toStdWString(), file_vec, override) ? 0 : -1;
}
