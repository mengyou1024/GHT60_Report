#include "UtilsIntr.hpp"
#include "QFileDialog"
#include "Utils.hpp"
#include <QByteArray>
#include <QDateTime>
UtilsIntr::UtilsIntr(QObject *parent) :
QObject{parent} {}

bool UtilsIntr::excelRender(QList<QVariant> dirs, bool use_dac, bool override_gate, int gate_height, bool override_dac_db, int dac_db) {
    std::vector<std::shared_ptr<FILE_RES>> file_vec;
    for (const auto &it_dir : qAsConst(dirs)) {
        QFileInfo info(it_dir.toString());
        if (info.isFile()) {
            qDebug().noquote() << "resolve: " << info.filePath();
            file_vec.emplace_back(FILE_RES::FromFile(info.filePath().toStdWString()));
        } else if (info.isDir()) {
            QDir dir(it_dir.toString());
            if (!dir.exists()) {
                continue;
            }
            dir.setFilter(QDir::Files);
            dir.setNameFilters({"*.bmp"});
            const auto entryList = dir.entryInfoList();
            for (const auto &file : qAsConst(entryList)) {
                qDebug().noquote() << "resolve: " << file.filePath();
                file_vec.emplace_back(FILE_RES::FromFile(file.filePath().toStdWString()));
            }
        }
    }

    std::optional<double> o_dac = std::nullopt, o_gate = std::nullopt;
    if (override_gate) {
        o_gate = gate_height / 100.0;
    }
    if (override_dac_db) {
        o_dac = dac_db;
    }

    QSettings settings("export_setting.ini", QSettings::Format::IniFormat);
    settings.beginGroup("DIR");

    if (settings.value("export").isNull() || !QDir(settings.value("export").toString()).exists()) {
        QString fileUrl = QFileDialog::getExistingDirectory(nullptr, "选择默认报表导出位置");
        if (fileUrl.isEmpty()) {
            return false;
        }
        settings.setValue("export", fileUrl);
        settings.endGroup();
        settings.sync();
        settings.beginGroup("DIR");
    }

    QString exportFilename = settings.value("export").toString() + "/报表-" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + ".xlsx";
    auto    ret            = FILE_RES::RenderExcel(exportFilename.toStdWString(), file_vec, use_dac, o_dac, o_gate);
    qDebug() << "ret=" << ret;
    if (ret == true) {
        QProcess process(nullptr);
        QObject::connect(&process, &QProcess::readyReadStandardOutput, [&process]() { qInfo() << process.readAllStandardOutput(); });
        QObject::connect(&process, &QProcess::readyReadStandardError, [&process]() { qCritical() << process.readAllStandardError(); });

#ifdef QT_DEBUG
        process.start("cmd", QStringList() << "/c" << "start ksolaunch.exe" << exportFilename);
#else
        process.start("ksolaunch.exe", QStringList() << exportFilename);
#endif
        process.waitForStarted();
        process.waitForFinished();
    }
    return ret;
}

QSUtils::QSUtils(QObject *parent) :
QObject(parent) {}

QString QSUtils::gb2312Tottf8(QString str) const {
    QTextCodec *utf8Codec   = QTextCodec::codecForName("utf-8");
    QTextCodec *gb2312Codec = QTextCodec::codecForName("gb2312");
    QString     strUnicode  = gb2312Codec->toUnicode(str.toLocal8Bit().data());
    QByteArray  utf8Str     = utf8Codec->fromUnicode(strUnicode);
    return utf8Str;
}

QString QSUtils::utf8Togb2312(QString str) const {
    QTextCodec *utf8Codec   = QTextCodec::codecForName("utf-8");
    QTextCodec *gb2312Codec = QTextCodec::codecForName("gb2312");
    QString     strUnicode  = utf8Codec->toUnicode(str.toLocal8Bit().data());
    QByteArray  gb2312      = gb2312Codec->fromUnicode(strUnicode);
    return gb2312;
}

QUrl QSUtils::toAbsoluteUrl(QString path) const {
    QFileInfo dir(path);
    return QUrl("file:///" + dir.absoluteFilePath());
}

QSUtils *QSUtils::Instance() {
    static QSUtils inst;
    return &inst;
}

QString QSUtils::fromLocal8Bit(QString str) const {
    return QString::fromLocal8Bit(str.toUtf8());
}
