/******************************************************************************
        DSettings - Drop-in replacement for QSettings

    DSettings helps address a QSettings issue on Android. On Android,
    QSettings cannot write settings to a file URI.

    DSettings implements similar interfaces to QSettings and uses QFile to
    write settings to a file.
    If instanciated with user/system scope it calls QSettings underneath,
    as this works on Android.

    Limitations: only INI format is supported. Only accepted types: QString,
    bool, int, double.
******************************************************************************/

#include "dsettings.h"


DSettings::DSettings(const QString &fileName, QSettings::Format format, QObject *parent)
    : QObject{parent}
{
    (void)format;

    qsettings=nullptr;

    //qDebug("DSettings::DSettings.\n");
    if(fileName.isNull())
    {
        //qDebug("Filename is null\n");
        return;
    }
    else
        qDebug("Filename: %s\n",fileName.toStdString().c_str());


    // Open the file for writing
    file.setFileName(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        //qDebug("DSettings::DSettings. File open ok\n");
        // Stream writer...
        outstream.setDevice(&file);



        outstream << "[General]" << Qt::endl;

    }
}
DSettings::DSettings(QSettings::Format format, QSettings::Scope scope, QString organization, QString application, QObject *parent)
    : QObject{parent}
{
    qsettings = new QSettings(format,scope,organization,application,parent);
}
DSettings::~DSettings()
{
    qDebug("DSettings::~DSettings.\n");
    if(qsettings!=nullptr)
    {
        qsettings->sync();
        delete qsettings;
    }
    else
    {
        if(file.isOpen())
        {
            qDebug("DSettings::~DSettings. Closing file.\n");
            file.close();
        }
    }
}

/*void DSettings::setValue(QString key,QVariant value)
{
    qDebug("DSettings::setValue.\n");

}*/
void DSettings::setValue(QString key,QString value)
{
    //qDebug("DSettings::setValue: QString\n");
    //qDebug("String is: '%s'\n",value.toStdString().c_str());

    if(qsettings!=nullptr)
    {
        qsettings->setValue(key,value);
        return;
    }

    outstream << key << "=" << escape(value) << Qt::endl;
}
void DSettings::setValue(QString key,double value)
{
    //qDebug("DSettings::setValue: double\n");
    setValue(key,QString("%1").arg(value));
}
void DSettings::setValue(QString key,int value)
{
    //qDebug("DSettings::setValue: int\n");
    setValue(key,QString("%1").arg(value));
}
void DSettings::setValue(QString key,bool value)
{
    //qDebug("DSettings::setValue: bool\n");
    setValue(key,value?QString("true"):QString("false"));
}
QString DSettings::escape(QString str)
{
    // Strings need escaping when having \n, \a or a semicolon
    if( str.indexOf("\n")==-1 && str.indexOf("\a")==-1 && str.indexOf(";")==-1)
        return str;

    QString escaped;
    escaped = str.replace("\n","\\n");
    escaped = str.replace("\a","\\a");
    return "\""+escaped+"\"";
}

void DSettings::sync()
{
    if(qsettings!=nullptr)
    {
        qsettings->sync();
        return;
    }
    file.flush();
}
QSettings::Status DSettings::status()
{
    if(qsettings!=nullptr)
        return qsettings->status();

    if(file.isOpen())
        return QSettings::NoError;
    else
        return QSettings::AccessError;
}
QString DSettings::fileName()
{
    if(qsettings!=nullptr)
        return qsettings->fileName();
    return file.fileName();
}
