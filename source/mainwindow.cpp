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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QScreen>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>

#include "cio.h"
#include "precisetimer.h"

#include "portwindow.h"
#include "btwindow.h"
#include "helpwindow.h"
#include "iodev.h"
#include "parse.h"

#include <QtSerialPort/QSerialPort>
#include "dterminal.h"

#include "parse.h"

/*
 Example format:
 <Calibrated;5 6 7;xscale=1;yscale=-3300 3300 ;color=ff0000 ff00 ff><Raw;2 3 4;xscale=1;yscale=0 1024 ;color=ff0000 ff00 ff>
*/



MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::MainWindow),
   fp(string("")),
   terminal(100)
{
   ui->setupUi(this);

#ifdef DEVELMODE
    ConsoleInit();
   printf("Starting up\n");
#endif


    totreceiveddata=0;

    // Set the NaN replacement value
    nanvalue=0;
    ui->uiNanValue->setValue(nanvalue);

    // Set how much past data we keep
    buffersize=100000;
    ui->uiBufferSize->setValue(buffersize);


    // Initialize the scopes
    dscopes = new DScopesQTWidget(0,0,640,480,false);
    ui->mainVerticalLayout->addWidget(dscopes);
    resize(800, 640);


    // Initialise the UI
    QFontMetrics m(ui->uiteDisplayFormat->font());
    int RowHeight = m.lineSpacing() ;
    printf("Row height %d\n",RowHeight);
    ui->uiteDisplayFormat->setFixedHeight(3*RowHeight) ;

    // Connect the device
    connect(&iodev,SIGNAL(readyRead(QByteArray)),this,SLOT(iodevread(QByteArray)));
    connect(&iodev,SIGNAL(connected()),this,SLOT(ioconnected()));
    connect(&iodev,SIGNAL(disconnected()),this,SLOT(iodisconnected()));

    binary=false;		// Text mode by default

    //ui->uileHostPort->setText(QString("ser:com13:19200"));
    ui->uileHostPort->setText(QString("ser:com18:19200"));
    //ui->uileHostPort->setText(QString("tcp:127.0.0.1:50008"));
    //ui->uileHostPort->setText(QString("bt:00066686854B"));
    //ui->uileHostPort->setText(QString("bt:00066686835E"));


    pt=0;
    lastplot=0;


    fp.Status();

    time_received_last=0;
    time_received_delta=0;
    time_displayed_last=PreciseTimer::QueryTimer();
    time_displayed_delta=0;

    datarateLabel=new QLabel(statusBar());
    statusBar()->addWidget(datarateLabel);
    displayrateLabel=new QLabel(statusBar());
    statusBar()->addWidget(displayrateLabel);

    time_received_delta_vector.resize(100,0.0);




    /*QCoreApplication::setOrganizationName("danielroggen");
    QCoreApplication::setOrganizationDomain(".com");
    QCoreApplication::setApplicationName("DScopeQt");*/

    //ui->uiptTerminal->ensureCursorVisible();
    //ui->uiptTerminal->setCenterOnScroll(true);


    loadSettings();


    qDebug("devicePixelRatio %f\n",QApplication::primaryScreen()->devicePixelRatio());
    qDebug("logicalDotsPerInchX %f\n",QApplication::primaryScreen()->logicalDotsPerInchX());
    qDebug("logicalDotsPerInchY %f\n",QApplication::primaryScreen()->logicalDotsPerInchY());
    qDebug("physicalDotsPerInchX %f\n",QApplication::primaryScreen()->physicalDotsPerInchX());
    qDebug("physicalDotsPerInchY %f\n",QApplication::primaryScreen()->physicalDotsPerInchY());

    //if(QApplication::primaryScreen()->devicePixelRatio()>1.5)
    if(QApplication::primaryScreen()->physicalDotsPerInchX()>400)
        dscopes->setScale(4);
    else
        if(QApplication::primaryScreen()->physicalDotsPerInchX()>200)
            dscopes->setScale(2);
        else
            dscopes->setScale(1);

    // Desactivate the mode to navigate back/forth in the stacked widget
#ifndef DEVELMODE
    ui->frame_4->setVisible(false);
#endif
    // Desactivate the group box to choose the zoom factor
    ui->groupBox->setVisible(false);

    // Set by default the main tab
    ui->stackedWidget->setCurrentIndex(0);
    ui->tabWidget->setCurrentIndex(0);


    // Load specific settings from command line, if specified
    QStringList arg = qApp->arguments();
    if(arg.contains("-f"))
    {
      int idx=arg.indexOf("-f");
      if(idx<arg.size()-1)
      {
          loadSettings(arg.at(idx+1));
      }
      else
      {
         QMessageBox::critical(this, "Command line error", "Missing configuration file name to load");
      }
    }
    if(arg.contains("-c"))
    {
      on_uipbConnect_clicked();
    }
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::SaveData()
{
   QString t="Save data ";
   QString fileName = QFileDialog::getSaveFileName(this, t,QString(),"Text (*.txt)");
   if(!fileName.isNull())
   {
      QFile file(fileName);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
      {
         // Stream writer...
         QTextStream out(&file);

         int i1,i2;
         i2=alldata[0].size();
         if(totreceiveddata<=i2)
            i1=i2-totreceiveddata;
         else
            i1=0;
         for(int i=i1;i<i2;i++)
         {
            for(unsigned j=0;j<alldata.size();j++)
               out << alldata[j][i] << " ";
            out << endl;
         }
        file.close();
      }
      else
      {
         QMessageBox::critical(this,t+"error", "Cannot write to file");
      }
   }
}


void MainWindow::Plot()
{
   double t1,t2;
   //t1=PreciseTimer::QueryTimer();
   // Plot scopes
   if(sd.scopedefinition.size()==0)
   {
      for(unsigned i=0;i<alldata.size();i++)
         dscopes->operator[](handles[i])->Plot(alldata[i],0xffffffff);
   }
   else
   {
      // Iterate all scopes
      for(unsigned s=0;s<sd.scopedefinition.size();s++)
      {
         // Iterate all traces of the scope
         vector<vector<int> *> vd;		// data for the traces in this scope
         vector<unsigned> vc;			// colors  of the traces
         for(unsigned t=0;t<sd.scopedefinition[s].traces.size();t++)
         {
            // If trace available
            if(sd.scopedefinition[s].traces[t]<alldata.size())
            {
               vd.push_back(&alldata[sd.scopedefinition[s].traces[t]]);
               if(sd.scopedefinition[s].colors.size()!=0)
                  vc.push_back(sd.scopedefinition[s].colors[t]);
               else
                  vc.push_back(0xffffffff);
            }
            dscopes->operator[](handles[s])->Plot(vd,vc);
         }
      }
   }
   /*t2=PreciseTimer::QueryTimer();
   pt = 0.9*pt+0.1*(t2-t1);
   printf("plot time: %.5lf\n",pt);*/
//	dscopes->Render();
}
void MainWindow::timerEvent(QTimerEvent *event)
{
   double f=0.9;
   double t = PreciseTimer::QueryTimer();
   time_displayed_delta = f*time_displayed_delta + (1.0-f)*(t-time_displayed_last);
   time_displayed_last=t;
   QString s;
   s.sprintf("Display rate: %3.0lf Hz",1.0/time_displayed_delta);
   displayrateLabel->setText(s);

   Plot();
   dscopes->Render();
   dscopes->repaint();

}



void MainWindow::on_uipbConnect_clicked()
{
    printf("iodev %d\n",iodev.isConnected());
    // Disconnection
    if(iodev.isConnected())
    {
        ui->uipbConnect->setText("Disconnecting...");
        ui->uipbConnect->setEnabled(false);
        iodev.close();
        return;
    }
    // Connection
    ConnectionData conn;
    bool ok=ParseConnection(ui->uileHostPort->text(),conn);
    if(!ok)
    {
        QMessageBox::critical(this, "Device specification error",conn.message);
        return;
    }
    printf("going to open conn\n");
    ok=iodev.open(conn);
    if(ok)
    {
        printf("Connection request successful\n");
        // Update the UI
        ui->uipbConnect->setText("Connecting...");
        ui->uipbConnect->setEnabled(false);
        time_received_last=PreciseTimer::QueryTimer();
    }
    else
    {
        // In that case
        printf("Connection failed\n");
    }


}
void MainWindow::aboutToClose()
{
        printf("MainWindow::aboutToClose\n");
}

void MainWindow::DeleteAllScopes()
{
   while(handles.size())
   {
      dscopes->RemoveScope(*(handles.end()-1));
      handles.erase(handles.end()-1);
   }
}
void MainWindow::AddScopes(unsigned n)
{
   handles = dscopes->AddScopes(n);
}


/**
  \brief Do something with NaNs - typically set a constant value in the data buffer for display purposes
**/
void MainWindow::treatNaN(vector<int> &linedata,std::vector<bool> &linedatanan)
{
   for(unsigned i=0;i<linedata.size();i++)
      if(linedatanan[i])
         linedata[i]=nanvalue;
}

void MainWindow::receivedData(vector<int> &linedata,std::vector<bool> &linedatanan)
{
   // Handle the NaN
   treatNaN(linedata,linedatanan);


   // Keep a buffer of the last

   double f=0.99;
   double t = PreciseTimer::QueryTimer();
   if(linedata.size())
   {

      time_received_delta_vector.push_back(t-time_received_last);
      time_received_delta = f*time_received_delta + (1.0-f)*(t-time_received_last);

      time_received_delta_vector.erase(time_received_delta_vector.begin());
      double tt=0;
      for(int i=0;i<time_received_delta_vector.size();i++)
         tt+=time_received_delta_vector[i];
      tt/=(double)time_received_delta_vector.size();

      QString s;
      //s.sprintf("Data rate: %3.1lf Hz  %3.1lf Hz",1.0/time_received_delta,1.0/tt);
      s.sprintf("Data rate: %3.0lf Hz",1.0/tt);
      //s.sprintf("Data rate: %3.0lf Hz",1.0/time_received_delta);
      datarateLabel->setText(s);


      time_received_last=t;


   }





   unsigned newsize = linedata.size();
   unsigned oldsize = alldata.size();
   // Check for sensor changes
   if(oldsize!=newsize)
   {
      printf("Changing from %d to %d\n",oldsize,newsize);

      // Resize the data vector according to the new number of sensors
      // Since we cannot identify sensors we should in principle erase all past data to avoid confusion - we don't.
      if(newsize<oldsize)
      {
         for(unsigned i=0;i<oldsize-newsize;i++)
            alldata.erase(alldata.end()-1);
      }
      else
      {
         for(unsigned i=0;i<newsize-oldsize;i++)
            //alldata.push_back(vector<int> (buffersize,0));
            alldata.push_back(vector<int> (buffersize,nanvalue));
      }
      // Adjust scope number
      if(sd.scopedefinition.size()==0)
      {
         DeleteAllScopes();
         AddScopes(newsize);
      }
   }
   // Shift the data in the data buffer
   for(unsigned i=0;i<alldata.size();i++)
   {
      // Erase old data
      alldata[i].erase(alldata[i].begin());
      // Store new data
      alldata[i].push_back(linedata[i]);
   }
   /*if(PreciseTimer::QueryTimer()-lastplot>refresh)
   {
      Plot();
      dscopes->Render();
      dscopes->repaint();
      lastplot = PreciseTimer::QueryTimer();
   }*/

   totreceiveddata++;
}




/*void MainWindow::displayError(QAbstractSocket::SocketError)
{
   printf("Socket error\n");
}*/


void MainWindow::on_uiRefreshRate_valueChanged(int i)
{
   killTimer(timer);
   timer=startTimer(1000/i);
   refresh=1.0/(double)i;
}
void MainWindow::on_uiNanValue_valueChanged(int i)
{
   nanvalue = i;
   printf("changed nan to %d\n",i);
}

void MainWindow::on_uiBufferSize_valueChanged(int i)
{
   buffersize=i;
   // We cut/expand alldata, but only if there are some data in alldata
   if(alldata.size()==0)
      return;
   // Get the how much is the buffer size currently
   unsigned datasize = alldata[0].size();
   // Adjust the buffer size
   printf("Adjust buffer from %d to %d\n",datasize,buffersize);
   for(unsigned c=0;c<alldata.size();c++)
   {
      // Need to cut datasize-buffersize data
      if(buffersize<datasize)
         alldata[c].erase(alldata[c].begin(),alldata[c].begin()+datasize-buffersize);
      // Need to prepend buffersize-datasize data
      if(buffersize>datasize)
         alldata[c].insert(alldata[c].begin(),buffersize-datasize,nanvalue);
   }
}


void MainWindow::on_uiAfterGlow_stateChanged(int state)
{
   dscopes->SetAlpha(state==Qt::Checked);
}



void MainWindow::applyDisplaySettings()
{
   sd.scopedefinition.clear();
   //int rv = ParseLayoutString(uileDisplayFormat->text().toStdString(),sd);
   int rv = ParseLayoutString(ui->uiteDisplayFormat->toPlainText().toStdString(),sd);
   if(rv!=0)
   {
      printf("Error in layout string\n");
      QMessageBox::information(this, "Invalid display format string","Check help for more information");
      return;
   }

   // Erase all the scopes
   DeleteAllScopes();
   // Hardcore init of the number of scopes
   if(sd.scopedefinition.size()!=0)
   {

      handles=dscopes->AddScopes(sd.scopedefinition.size());
      for(unsigned i=0;i<sd.scopedefinition.size();i++)
      {
         dscopes->operator[](handles[i])->SetTitle(sd.scopedefinition[i].title);
         if(sd.scopedefinition[i].yauto)
            dscopes->operator[](handles[i])->SetVAuto();
         else
            dscopes->operator[](handles[i])->SetVRange(sd.scopedefinition[i].yscale[0],sd.scopedefinition[i].yscale[1]);
         dscopes->operator[](handles[i])->HZoom(sd.scopedefinition[i].xscale);
      }
      vector<int> v(1,0);
      for(unsigned i=0;i<sd.scopedefinition.size();i++)
         dscopes->operator[](handles[i])->Plot(v);
   }
   else	// Variable number of scopes: adjust to the number of data last received
   {
      handles = dscopes->AddScopes(alldata.size());
   }
   dscopes->Render();
   dscopes->repaint();
}

void MainWindow::applyFormatSettings()
{
   // DX3 reader: DX3;cc-s-s-s-s-s-s

   // Test the binary string.
   string binaryformat = ui->uileBinaryFormat->text().trimmed().toStdString();
   if(binaryformat.size()==0)
   {
      binary=false;
   }
   else
   {
      binary=true;
      fp = FrameParser3(binaryformat);
      if(!fp.IsValid())
         QMessageBox::critical(this, "Invalid binary format", "Binary format specification is: \"<header>;<[-]c|s|S|i|I|b<num>|B<num>><[-]c|s|S|i|I|b<num>|B<num>>...[;<x|f|F>]\"");
   }
}



void MainWindow::iodevread(QByteArray ba)
{
    // When data is received we either direct it to the scopes, when the scopes or the config tabs are displayed, or to the terminal
    if(ui->tabWidget->currentIndex()==2)
    {
        // Data to be displayed on terminal
        terminal.addBytes(ba);
        QString str=terminal.getText();
        ui->uiptTerminal->setPlainText(str);
        //ui->uiptTerminal->setCenterOnScroll(true);
        ui->uiptTerminal->verticalScrollBar()->setValue(ui->uiptTerminal->verticalScrollBar()->maximum());
        //ui.textEdit->verticalScrollBar()->setValue(ui.textEdit->verticalScrollBar()->maximum());
    }
    else
    {
        // Data to be decoded and fed to time series
        if(binary)
        {
         BinaryChunckDecodeRead(ba);
        }
        else
        {
            TextChunckDecodeRead(ba);
        }
    }
}
void MainWindow::ioconnected()
{
    printf("MainWindow::ioconnected\n");

    ui->uipbConnect->setEnabled(true);
    ui->uipbConnect->setText("Dis&connect");
    // Move to the scope window
    ui->stackedWidget->setCurrentIndex(1);
    ui->tabWidget->setCurrentIndex(0);

    // Move the UI
    //ui->stackedWidget->widget(0)->layout()->removeWidget(ui->frame_Settings);
    ui->verticalLayoutMain->removeWidget(ui->frame_Settings);
    //ui->tabWidget->widget(0)->layout()->addWidget(ui->frame_Settings);
    //ui->tabWidget->widget(1)->layout()->addWidget(ui->frame_Settings);
    //ui->tabWidget->widget(1)->layout()->addWidget(ui->frame_Settings);
    ui->verticalLayoutConfigtab->insertWidget(0,ui->frame_Settings);
}
void MainWindow::iodisconnected()
{
    printf("MainWindow::iodisconnected\n");
    ui->uipbConnect->setEnabled(true);
    ui->uipbConnect->setText("&Connect");
    // Move to the connect window
    ui->stackedWidget->setCurrentIndex(0);

    // Move the UI
    ui->tabWidget->widget(1)->layout()->removeWidget(ui->frame_Settings);
    //ui->stackedWidget->widget(0)->layout()->addWidget(ui->frame_Settings);
    ui->verticalLayoutMain->insertWidget(0,ui->frame_Settings);

}

void MainWindow::TextChunckDecodeRead(const QByteArray& in)
{
    unsigned it=0;
    // Add data to our reader


    dlr.add(std::string(in.constData()));
    //printf("cur dlr: %d\n",dlr.size());

   while(1)
   {
        //printf("it %d\n",it);
        it++;
        std::string str;
        bool rv = dlr.getLine(str);
        if(rv==false)
            break;
        if(str.size()==0)
            break;

        QString qstr(str.c_str());

        //printf("%s\n",qstr.toStdString().c_str());

      // Check the length of the string
      //printf("%d %d\n",qstr.size(),strlen(qstr.toAscii().data()));

      // Generate synthetic string
      //qstr =QString("   3      10 \t 1213 , -31239 ; NaN 3891 NaN NaN 12902 3.1415 -2.7 10.00");

      QRegExp rx(QString("[ ;,\t]"));                             // Match any separator
      QStringList sl = qstr.split(rx,QString::SkipEmptyParts);		// Split on separator, skip empty parts. We don't want multiple spaces or tabs to be interpreted as multiple strings
      vector<int> linedata(sl.size());
        vector<float> linedataf(sl.size());
      vector<bool> linedatanan(sl.size());
      for(int i=0;i<sl.size();i++)			// Convert string list to int vector
      {
         // Check for NaN
         if(QString("NaN").compare(sl[i],Qt::CaseInsensitive)==0)
         {
            linedata[i]=0;
                linedataf[i]=0;
            linedatanan[i]=true;
         }
         else
         {
            // Allow floats, but cast them to int
            bool ok;
            double v = sl[i].toDouble(&ok);
            int vi = (int)v;
            if( ok )
            {
               linedata[i] = vi;
                    linedataf[i] = v;
               linedatanan[i] = false;
            }
            else
            {
               linedata[i] = 0;
                    linedataf[i] = 0;
               linedatanan[i] = true;
            }
         }
      }
      /*
      for(int i=0;i<sl.size();i++)
      {
         if(linedatanan[i]==false)
            printf("%d ",linedata[i]);
         else
            printf("- ");
      }
      printf("\n");
      */




      receivedData(linedata,linedatanan);
   }
}
void MainWindow::BinaryChunckDecodeRead(QByteArray &ba)
{
   vector<vector<int> > v = fp.Parser(ba.data(),ba.size());
   if(v.size())
   {
      for(int i=0;i<v.size();i++)
      {
         std::vector<bool> datanan(v[i].size(),false);
         receivedData(v[i],datanan);
      }
   }
}
void MainWindow::dsrChanged(bool s)
{
   printf("dsrChanged status: %d\n",s);
}

/******************************************************************************
  on_actionSaveConfiguration_triggered
*******************************************************************************
Save the configuration in an INI file
******************************************************************************/
void MainWindow::on_actionSaveConfiguration_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save settings",QString(),"Settings (*.ini)");
    if(!fileName.isNull())
    {
        bool rv = saveSettings(fileName);
        if(rv)
            QMessageBox::critical(this, "Save settings error", "Cannot write to file");
    }
}
void MainWindow::on_actionLoadConfiguration_triggered()
{
   QString fileName = QFileDialog::getOpenFileName(this, "Load settings",QString(),"Settings (*.ini)");
   if(!fileName.isNull())
      loadSettings(fileName);
}
/******************************************************************************
  loadSettings
*******************************************************************************
Load the app configuration from QSettings.
******************************************************************************/
bool MainWindow::loadSettings(QString fileName)
{
    QSettings *settings;

    if(fileName==QString())
        settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,"danielroggen","dscopeqt");
    else
        settings = new QSettings(fileName,QSettings::IniFormat);

    if(settings->status()!=QSettings::NoError)
        return true;

    QString df = settings->value("DisplayFormat","").toString();
    QString bf = settings->value("BinaryFormat","").toString();
    QString id = settings->value("InputDevice","").toString();
    int bs = settings->value("BufferSize",100000).toInt();
    int nv = settings->value("NanValue",0).toInt();
    int ag = settings->value("AfterGlow",0).toInt();
    int rr = settings->value("RefreshRate",10).toInt();

    ui->uiAfterGlow->setCheckState(ag?Qt::Checked:Qt::Unchecked);

    if(rr<ui->uiRefreshRate->minimum()) rr=ui->uiRefreshRate->minimum();
    if(rr>ui->uiRefreshRate->maximum()) rr=ui->uiRefreshRate->maximum();
    ui->uiRefreshRate->setValue(rr);
    on_uiRefreshRate_valueChanged(rr);

    ui->uiteDisplayFormat->setPlainText(df);
    ui->uileBinaryFormat->setText(bf);
    ui->uileHostPort->setText(id);

    if(bs<ui->uiBufferSize->minimum())
        bs=ui->uiBufferSize->minimum();
    if(bs>ui->uiBufferSize->maximum())
        bs=ui->uiBufferSize->maximum();
     // The onValueChanged event will change buffersize if needed
     ui->uiBufferSize->setValue(bs);

    if(nv<ui->uiNanValue->minimum())
        nv=ui->uiNanValue->minimum();
    if(nv>ui->uiNanValue->maximum())
        nv=ui->uiNanValue->maximum();
    // The onValueChanged event will change nanvalue if needed
    ui->uiNanValue->setValue(nv);

   applyDisplaySettings();
   applyFormatSettings();
}
/******************************************************************************
  saveSettings
*******************************************************************************
Save the app configuration to QSettings.
Returns 0 in case of success
******************************************************************************/
bool MainWindow::saveSettings(QString ini)
{
    QSettings *settings;

    if(ini==QString())
        settings=new QSettings(QSettings::IniFormat,QSettings::UserScope,"danielroggen","dscopeqt");
    else
        settings=new QSettings(ini,QSettings::IniFormat);

    if(settings->status()!=QSettings::NoError)
        return true;

    //
    printf("Storing settings to %s\n",settings->fileName().toStdString().c_str());
    qDebug("Storing settings to %s\n",settings->fileName().toStdString().c_str());


    settings->setValue("DisplayFormat",ui->uiteDisplayFormat->toPlainText());
    settings->setValue("BinaryFormat",ui->uileBinaryFormat->text());
    settings->setValue("InputDevice",ui->uileHostPort->text());
    settings->setValue("BufferSize",ui->uiBufferSize->text());
    settings->setValue("NanValue",nanvalue);
    settings->setValue("RefreshRate",ui->uiRefreshRate->value());
    settings->setValue("AfterGlow",ui->uiAfterGlow->isChecked()?1:0);
    delete settings;
    return false;
}


void MainWindow::on_actionAbout_triggered()
{
   QMessageBox::about(this, "About",
   "<p><b>DScope</b> - QT Version</p>\n"
   "<p>Version 1.11</p>"
   "<p>(c) 2007-2020 Daniel Roggen</p>");

}
void MainWindow::on_actionListSerialPorts_triggered()
{
    selectSerial();
}
void MainWindow::on_actionList_Bluetooth_devices_triggered()
{
    selectBt();
}

void MainWindow::on_actionHowto_triggered()
{

   QFile file(":/howto.html");
   file.open(QIODevice::ReadOnly | QIODevice::Text);
   QByteArray filedata = file.readAll();
   HelpWindow dialog(QString(filedata),this);
#ifdef Q_OS_ANDROID
    dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
#endif
   dialog.exec();
}

void MainWindow::on_pushButton_ClearData_clicked()
{
    clearDataClicked();
}

void MainWindow::on_actionSave_data_triggered()
{
    saveDataClicked();
}
void MainWindow::on_actionClear_data_triggered()
{
    clearDataClicked();
}

void MainWindow::on_pushButton_SaveData_clicked()
{
    saveDataClicked();
}

void MainWindow::saveDataClicked()
{
    if(totreceiveddata==0)
    {
       QMessageBox::information(this,"Save data","No data has been collected - connect to a device first");
       return;
    }
    SaveData();
}
void MainWindow::clearDataClicked()
{
    totreceiveddata=0;
    alldata.clear();
    if(sd.scopedefinition.size()==0)
       DeleteAllScopes();
}




/*
 * Upon press sends a text with CRLF to the remote device
*/
void MainWindow::on_uipbSend_clicked()
{
    // Get text
    QString text = ui->uileSendText->text();
    text.append("\r\n");
    iodev.send(text.toLatin1());

    // Select the text in the line edit, so that the user can type something overwriting the old content,
    // or press enter to repeat the old message

    ui->uileSendText->selectAll();
    ui->uileSendText->setFocus();
}







void MainWindow::on_pbMinus1_clicked()
{
    printf("-1\n");
    // Change the widget
    int current = ui->stackedWidget->currentIndex();
    printf("current: %d/%d\n",current,ui->stackedWidget->count());
    if(current<=0)
        return;
    ui->stackedWidget->setCurrentIndex(current-1);

}

void MainWindow::on_pbPlus1_clicked()
{
    printf("+1\n");
    // Change the widget
    int current = ui->stackedWidget->currentIndex();
    printf("current: %d/%d\n",current,ui->stackedWidget->count());
    if(current>=ui->stackedWidget->count()-1)
        return;
    ui->stackedWidget->setCurrentIndex(current+1);
}

void MainWindow::on_pushButtonSelectSerial_clicked()
{
    selectSerial();

}

void MainWindow::on_pushButtonSelectBt_clicked()
{
    selectBt();
}

void MainWindow::selectSerial()
{
    QString port;
    PortWindow dialog(&port,this);
    // The below should only be done on Android
#ifdef Q_OS_ANDROID
    dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
#endif
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        if(port!=QString())
        {
            // Update the port string field
            ui->uileHostPort->setText(QString("ser:")+port+":19200");
        }
    }
}
void MainWindow::selectBt()
{
    QString bt;
    BtWindow dialog(&bt,this);
    // The below should only be done on Android
#ifdef Q_OS_ANDROID
    dialog.setWindowState(dialog.windowState() | Qt::WindowMaximized);
#endif
    int ret = dialog.exec();
    if(ret == QDialog::Accepted)
    {
        if(bt!=QString())
        {
            // Remove all the colons in the MAC address
            QString bt2;
            for(int i=0;i<bt.size();i++)
                if(bt.at(i)!=':')
                    bt2=bt2+bt.at(i);
            // Update the port string field
            ui->uileHostPort->setText(QString("bt:")+bt2);
        }
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    // Disconnect
    // Disconnection
    if(iodev.isConnected())
    {
        ui->uipbConnect->setText("Disconnecting...");
        ui->uipbConnect->setEnabled(false);
        iodev.close();
        return;
    }
}

void MainWindow::on_uipbApplySettings_clicked()
{
    // Apply the binary mode and dispaly settings
    applyDisplaySettings();
    applyFormatSettings();
}



void MainWindow::on_uirbScale1_clicked()
{
    dscopes->setScale(1);
}

void MainWindow::on_uirbScale2_clicked()
{
    dscopes->setScale(2);
}

void MainWindow::on_uirbScale4_clicked()
{
    dscopes->setScale(4);
}
