/*
   DScope Application

   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QKeyEvent>
#include <QImage>
#include <QSplitter>
#include <QtNetwork>
#include <QTcpSocket>
#include <vector>
#include "helper.h"
#include "scope/DScopesQT.h"
#include "scope/DScopesQTWidget.h"
#include "scope/Scope.h"

#include "dterminal.h"
#include <QtSerialPort/QSerialPort>

#include "FrameParser/FrameParser3.h"
#include "dlinereader.h"
#include "iodev.h"
#include "parse.h"



namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
   virtual void timerEvent(QTimerEvent *event);


private:
    Ui::MainWindow *ui;
    DScopesQTWidget *dscopes;

    IoDevice iodev;

    int timer;
    bool binary;
    FrameParser3 fp;
    DLineReader dlr;
    DTerminal terminal;
    int totreceiveddata;
    int nanvalue;              // Value used to replace NaNs
    unsigned buffersize;     // Size of the buffer to save data / display

    QLabel *datarateLabel;
    QLabel *displayrateLabel;

    double pt;
    double lastplot,refresh;

    double time_received_last,time_received_delta;
    std::vector<double> time_received_delta_vector;

    double time_displayed_last,time_displayed_delta;

    // Scaling options
    double scaling_a,scaling_b;
    bool scaling_enabled;

    void Plot();
    void receivedData(vector<int> &linedata,std::vector<bool> &linedatanan);
    void receivedQuat(vector<float> &linedataf);
    void treatNaN(vector<int> &linedata,std::vector<bool> &linedatanan);


   // Trace storage
   vector<vector<int> > alldata;
   ScopesDefinition sd;
   vector<unsigned> handles;

   int scale(int data);

   void BinaryChunckDecodeRead(QByteArray &ba);
    void TextChunckDecodeRead(const QByteArray& in);

   bool loadSettings(QString=QString());
   bool saveSettings(QString=QString());
   void SaveData();
   void DeleteAllScopes();
   void AddScopes(unsigned n);

    void saveDataClicked();
    void clearDataClicked();

    void selectSerial();
    void selectBt();

    void applyDisplaySettings();
    void applyFormatSettings();

private slots:
    // Autoconnected signal/slot
    void on_actionSave_data_triggered();
    void on_actionClear_data_triggered();
    void on_actionAbout_triggered();
    void on_actionHowto_triggered();
    void on_actionListSerialPorts_triggered();
    void on_actionSaveConfiguration_triggered();
    void on_actionLoadConfiguration_triggered();
    void on_uipbConnect_clicked();
    void on_pushButton_ClearData_clicked();
    void on_pushButton_SaveData_clicked();
    void on_uiAfterGlow_stateChanged(int state);
    void on_uiRefreshRate_valueChanged(int i);
    void on_uiNanValue_valueChanged(int i);
    void on_uiBufferSize_valueChanged(int i);

   // Programmatically connected signal/slot
   void iodevread(QByteArray ba);
   void ioconnected();
   void iodisconnected();
   void ioerror(QString err);
   void ioconnectionerror();
   //void displayError(QAbstractSocket::SocketError);
   void dsrChanged(bool);
   void aboutToClose();

   // Change UI presentation
   void UiToIdle();
   void UiWaitConnect();
   void UiToConnected();

   void on_uipbSend_clicked();
   void on_actionList_Bluetooth_devices_triggered();
   void on_pbMinus1_clicked();
   void on_pbPlus1_clicked();
   void on_pushButtonSelectSerial_clicked();
   void on_pushButtonSelectBt_clicked();
   void on_pushButton_4_clicked();
   void on_uipbApplySettings_clicked();
   void on_uirbScale1_clicked();
   void on_uirbScale2_clicked();
   void on_uirbScale4_clicked();
   void on_uicb_scaling_stateChanged(int arg1);
   void on_uile_scaling_a_textEdited(const QString &arg1);
   void on_uile_scaling_b_textEdited(const QString &arg1);
};

#endif // __MAINWINDOW_H
