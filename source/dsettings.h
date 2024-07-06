/******************************************************************************
        DSettings - Drop-in replacement for QSettings
******************************************************************************/

#ifndef DSETTINGS_H
#define DSETTINGS_H

#include <QObject>
#include <QFile>
#include <QSettings>
#include <QTextStream>

class DSettings : public QObject
{
    Q_OBJECT
public:
    explicit DSettings(const QString &fileName, QSettings::Format format=QSettings::IniFormat, QObject *parent = nullptr);
    explicit DSettings(QSettings::Format format, QSettings::Scope scope, QString organization, QString application, QObject *parent = nullptr);
    ~DSettings();

    void setValue(QString key,QString value);
    void setValue(QString key,double value);
    void setValue(QString key,int value);
    void setValue(QString key,bool value);
    void sync();
    QSettings::Status status();
    QString fileName();

private:
    QFile file;
    QTextStream outstream;
    QSettings *qsettings;

    QString escape(QString str);
};

#endif // DSETTINGS_H
