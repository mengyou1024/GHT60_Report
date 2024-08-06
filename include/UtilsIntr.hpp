#pragma once

#include <QList>
#include <QObject>
#include <QTextCodec>
#include <QVariant>
#include <QtCore>

class UtilsIntr : public QObject {
    Q_OBJECT
public:
    explicit UtilsIntr(QObject* parent = nullptr);

    Q_INVOKABLE bool excelRender(QList<QVariant> dirs, bool use_dac, bool override_gate, int gate_height, bool override_dac_db, int dac_db);

signals:
};

class QSUtils : public QObject {
    Q_OBJECT
    explicit QSUtils(QObject* parent = nullptr);

public:
    Q_DISABLE_COPY_MOVE(QSUtils);

    Q_INVOKABLE QString gb2312Tottf8(QString str) const;

    Q_INVOKABLE QString fromLocal8Bit(QString str) const;

    Q_INVOKABLE QString utf8Togb2312(QString str) const;

    Q_INVOKABLE QUrl toAbsoluteUrl(QString path) const;

    static QSUtils* Instance();
};
