#include "btwindow.h"
#include "ui_btwindow.h"

#include <QBluetoothUuid>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QMessageBox>

BtWindow::BtWindow(QString *bt,QWidget *parent) :
    bt(bt),
    QDialog(parent),
    ui(new Ui::BtWindow)
{
    ui->setupUi(this);




    // Discover bluetooth devices
    printf("Starting scan\n");
    qDebug("************************************************start \n");

    // do something here with bluetooth
    QBluetoothDeviceDiscoveryAgent *devDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    //QBluetoothServiceDiscoveryAgent *srvDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(devDiscoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
    connect(devDiscoveryAgent, SIGNAL(finished()),this, SLOT(deviceFinished()));

    //connect(srvDiscoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));

    // Start a discovery
    devDiscoveryAgent->start();
    //srvDiscoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
    //srvDiscoveryAgent->start();


   // Fill the dialog
   /*QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
   ui->uitwPorts->setRowCount(ports.size());

   for (int i = 0; i < ports.size(); i++)
   {
      QTableWidgetItem *newItem;
      newItem = new QTableWidgetItem(ports.at(i).portName());
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,0, newItem);
      newItem = new QTableWidgetItem(ports.at(i).description());
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,1, newItem);
      newItem = new QTableWidgetItem(ports.at(i).manufacturer());
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,2, newItem);
      newItem = new QTableWidgetItem(QString::number(ports.at(i).productIdentifier()));
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,3, newItem);
      newItem = new QTableWidgetItem(QString::number(ports.at(i).vendorIdentifier()));
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,4, newItem);
      newItem = new QTableWidgetItem(ports.at(i).serialNumber());
      newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,5, newItem);
   }*/
}

BtWindow::~BtWindow()
{
    delete ui;
}



void BtWindow::deviceDiscovered(QBluetoothDeviceInfo di)
{
    qDebug("discovered something\n");
    qDebug("address: %s\n",di.address().toString().toStdString().c_str());

    bdi.push_back(di);
    populateTable();


    // address, uuid, major, minor, name, rssi, serviceclasses


}

void BtWindow::serviceDiscovered(QBluetoothServiceInfo si)
{
    qDebug("discovered service\n");

    qDebug("Service desc: %s\n",si.serviceDescription().toStdString().c_str());
    qDebug("Service name: %s\n",si.serviceName().toStdString().c_str());
    qDebug("Service prov: %s\n",si.serviceProvider().toStdString().c_str());

    qDebug("************************************************ Hello\n");

}

QString BtWindow::coreConfigurationToString(QBluetoothDeviceInfo::CoreConfigurations c)
{
    switch(c)
    {
    case 1:
        return "BLE";
        break;
    case 2:
        return "STD";
        break;
    case 3:
        return "STD+BLE";
        break;
    default:
        return "?";
    }
    return "?";
}

void BtWindow::populateTable()
{


    // Take all the list and filbidl the table
    ui->uitwPorts->setRowCount(bdi.size());
    for(int i=0;i<bdi.size();i++)
    {
        printf("Populate table %d\n",i);
        QTableWidgetItem *newItem;

        newItem = new QTableWidgetItem(bdi.at(i).name());
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,0, newItem);

        // TODO: on iOS/OSX must replace address by uuid, as the os does not return the address
        newItem = new QTableWidgetItem(bdi.at(i).address().toString());
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,1, newItem);

        newItem = new QTableWidgetItem(coreConfigurationToString(bdi.at(i).coreConfigurations()));
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,2, newItem);

        newItem = new QTableWidgetItem(QString::number(bdi.at(i).majorDeviceClass())+":"+QString::number(bdi.at(i).minorDeviceClass()));
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,3, newItem);

        newItem = new QTableWidgetItem(QString::number(bdi.at(i).rssi()));
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,4, newItem);


        newItem = new QTableWidgetItem(QString::number(bdi.at(i).serviceClasses()));
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,5, newItem);


        QBluetoothDeviceInfo::DataCompleteness c = bdi.at(i).serviceUuidsCompleteness();
        QList<QBluetoothUuid> siid = bdi.at(i).serviceUuids();
        QString siidstr="";
        printf("number of service uuid %d\n",siid.size());
        for(int j=0;j<siid.size();j++)
        {
            // TODO: use toUInt128 when QString::number will support quint128
            siidstr+=QString::number(siid.at(j).toUInt32());
            siidstr+=";";
        }
        if(c==QBluetoothDeviceInfo::DataIncomplete)
        {
            if(siidstr=="")
                siidstr="N/A";
            else
                siidstr+=" (partial)";
        }
        if(c==QBluetoothDeviceInfo::DataUnavailable)
            siidstr+="N/A";


        newItem = new QTableWidgetItem(siidstr);
        //newItem->setFlags(Qt::NoItemFlags);
        ui->uitwPorts->setItem(i,6, newItem);
    }

    ui->uitwPorts->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->uitwPorts->horizontalHeader()->setStretchLastSection(true);

    // Select first row, if available
    if(bdi.size()>0)
    {
        ui->uitwPorts->selectRow(0);
    }
}


void BtWindow::deviceFinished()
{
    QMessageBox::critical(this, "finished", "Discovery finished\n");
}
void BtWindow::on_uitwPorts_cellDoubleClicked(int row, int column)
{
    printf("cell double clicked\n");
    // Get the leftmost column which is the name
    printf("%s\n",ui->uitwPorts->item(row,0)->text().toStdString().c_str());

    accept();
}


void BtWindow::accept()
{
    // Check which line is selected
    int row = ui->uitwPorts->currentRow();

    printf("current row: %d\n",row);

    if(row!=-1)
    {
        *bt = ui->uitwPorts->item(row,1)->text();
    }


    QDialog::accept();
}
