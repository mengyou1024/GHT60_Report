/* encoding: GB2312 */
#include "Utils.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption opt_override("override", "override value", "override", "0");
    parser.addPositionalArgument("inputs", "input file names", "[input file names]");
    parser.addOptions({opt_override});
    parser.parse(app.arguments());
    auto inputs = parser.positionalArguments();
    qDebug() << app.arguments();
    qDebug() << inputs;
    std::optional<double> override = std::nullopt;
    if (parser.isSet(opt_override)) {
        bool _ok = false;
        auto _v  = parser.value("override").toDouble(&_ok);
        if (_ok) {
            override = _v;
        }
    }
    qDebug() << override.value_or(0);

    if (inputs.empty()) {
        inputs = QFileDialog::getOpenFileNames(nullptr, "按住ctrl可以选择多个文件");
    } else {
        auto accept_btn = QMessageBox::question(nullptr, "导出模式选择", "点击是导出当前选中数据, 选择否重新选择数据");
        if (accept_btn == QMessageBox::StandardButton::Yes) {
            if (!QFileInfo::exists(inputs[0])) {
                QMessageBox::warning(nullptr, "警告", "文件不存在, 请先选择一个文件打开");
                return -1;
            }
        } else {
            QString _open_path = QDir::currentPath();
            if (QFileInfo::exists(inputs[0])) {
                QFileInfo filePath(inputs[0]);
                _open_path = filePath.dir().absolutePath();
            }
            inputs = QFileDialog::getOpenFileNames(nullptr, "按住ctrl可以选择多个文件", _open_path);
        }
    }

    if (inputs.empty()) {
        return 0;
    }

    std::vector<std::shared_ptr<FILE_RES>> file_vec;

    for (auto& input : inputs) {
        file_vec.emplace_back(FILE_RES::FromFile(input.toStdWString()));
    }

    auto fileUtl = QFileDialog::getSaveFileName(nullptr, "选择报表保存位置", QDir::currentPath() + "/报表", "Excel (*.xlsx;*.xls)");
    if (!FILE_RES::RenderExcel(fileUtl.toStdWString(), file_vec, override.value_or(0))) {
        QMessageBox::warning(nullptr, "警告", "导出失败");
    } else {
        QMessageBox::information(nullptr, "成功", "导出成功");
    }
    return 0;
}
