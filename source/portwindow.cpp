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

#include "portwindow.h"
#include "ui_portwindow.h"

#include <QList>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>

PortWindow::PortWindow(QString *selectedport,QWidget *parent) :
    selectedport(selectedport),
    QDialog(parent),
    ui(new Ui::PortWindow)
{
   ui->setupUi(this);



   // Fill the dialog
   QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
   ui->uitwPorts->setRowCount(ports.size());

   for (int i = 0; i < ports.size(); i++)
   {
      QTableWidgetItem *newItem;
      newItem = new QTableWidgetItem(ports.at(i).portName());
      //newItem->setFlags(Qt::NoItemFlags);
      //newItem->setFlags(Qt::ItemIsEnabled);
      ui->uitwPorts->setItem(i,0, newItem);
      newItem = new QTableWidgetItem(ports.at(i).description());
      //newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,1, newItem);
      newItem = new QTableWidgetItem(ports.at(i).manufacturer());
      //newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,2, newItem);
      newItem = new QTableWidgetItem(QString::number(ports.at(i).productIdentifier()));
      //newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,3, newItem);
      newItem = new QTableWidgetItem(QString::number(ports.at(i).vendorIdentifier()));
      //newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,4, newItem);
      newItem = new QTableWidgetItem(ports.at(i).serialNumber());
      //newItem->setFlags(Qt::NoItemFlags);
      ui->uitwPorts->setItem(i,5, newItem);
   }
   ui->uitwPorts->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   ui->uitwPorts->horizontalHeader()->setStretchLastSection(true);

   // Select first row, if available
    if(ports.size()>0)
    {
        ui->uitwPorts->selectRow(0);
    }

}

PortWindow::~PortWindow()
{
    delete ui;
}



void PortWindow::on_uitwPorts_cellDoubleClicked(int row, int column)
{
    printf("cell double clicked\n");
    // Get the leftmost column which is the name
    printf("%s\n",ui->uitwPorts->item(row,0)->text().toStdString().c_str());

    accept();
}



void PortWindow::accept()
{
    // Check which line is selected
    int row = ui->uitwPorts->currentRow();

    if(row!=-1)
    {
        *selectedport = ui->uitwPorts->item(row,0)->text();
    }


    QDialog::accept();
}




