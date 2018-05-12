/*
   Copyright (C) 2017:
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
/*
 *
 * iodev
 *
 * Generic IO device for bluetooth, serial and tcp
 *
 * */


#include <QMessageBox>
#include <QBluetoothSocket>
#include <QBluetoothUuid>
#include "iodev.h"

IoDevice::IoDevice(QWidget *parent) :
    QWidget(parent),
    deviceBT(QBluetoothServiceInfo::RfcommProtocol)
{
    deviceSER=0;

    connectstate=DevNotConnected;

    // Connect and parametrise BT device
    deviceBT.setPreferredSecurityFlags(QBluetooth::NoSecurity);
    connect(&deviceBT, SIGNAL(readyRead()), this, SLOT(gotBTData()));
    connect(&deviceBT, SIGNAL(connected()), this, SLOT(BTConnected()));
    connect(&deviceBT, SIGNAL(disconnected()), this, SLOT(BTDisconnected()));
    connect(&deviceBT, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(BTError(QBluetoothSocket::SocketError)));


    // Connect TCP device - can be done as construct time
    connect(&deviceTCP, SIGNAL(readyRead()), this, SLOT(gotTCPData()));
    connect(&deviceTCP, SIGNAL(connected()), this, SLOT(TCPConnected()));
    connect(&deviceTCP, SIGNAL(disconnected()), this, SLOT(TCPDisconnected()));
}

bool IoDevice::open(ConnectionData cd)
{
    qDebug("OpDevoce::open\n");
    // If we're connected we disconnect
    if(connectstate!=DevNotConnected)
    {
        close();
    }
    // Here we should be disconnected. Connect to target device
    switch(cd.type)
    {
        case DevTCPConnected:
            qDebug("Connecting to TCP\n");
            deviceTCP.connectToHost(cd.tcphost,cd.tcpport);
            if (!deviceTCP.waitForConnected(2000))
            {
               QMessageBox::critical(this, "Connection failed", "Connection to host failed. Check host:port settings.\n");
               return false;
            }
            connectstate=DevTCPConnected;
            break;
        case DevSerialConnected:
            qDebug("Connecting to SER %s %d\n",cd.port.toStdString().c_str(),cd.baud);
            deviceSER = new QSerialPort(cd.port);
            connect(deviceSER,SIGNAL(readyRead()),this,SLOT(gotSERData()));
            deviceSER->open(QIODevice::ReadWrite);
            if(!deviceSER->isOpen())
            {
                QMessageBox::critical(this, "Connection failed", "Cannot open serial device. Check port:speed settings.\n");
                return false;
            }
            deviceSER->setBaudRate(cd.baud);
            deviceSER->setDataBits(QSerialPort::Data8);
            deviceSER->setFlowControl(QSerialPort::NoFlowControl);
            deviceSER->setParity(QSerialPort::NoParity);
            deviceSER->setStopBits(QSerialPort::OneStop);
            connectstate=DevSerialConnected;
            emit connected();

            break;
        case DevBTConnected:
            qDebug("Connecting to BT\n");
            // Connect to remote; the uuid corresponds to rfcomm
            #ifdef Q_OS_ANDROID
                deviceBT.connectToService(cd.ba,QBluetoothUuid(QString("00001101-0000-1000-8000-00805f9b34fb")));        // Works on android but not linux
            #else
                deviceBT.connectToService(cd.ba,QBluetoothUuid::SerialPort);       // Works on linux, does not work on android
            #endif
            connectstate=DevBTConnected;
            break;
        default:
            break;
    }
    return true;

}
bool IoDevice::close()
{
    printf("closing the port\n");
    // If not connected, we do nothing
    if(connectstate==DevNotConnected)
        return true;
    // We are connected, therefore we disconnect
    switch(connectstate)
    {
        case DevTCPConnected:
            if( deviceTCP.state() != QAbstractSocket::UnconnectedState )
            {
               deviceTCP.disconnectFromHost();
               if(deviceTCP.state() != QAbstractSocket::UnconnectedState)
               {
                  if(!deviceTCP.waitForDisconnected())
                  {
                     QMessageBox::critical(this, "Disconnect failed", "Could not disconnect from host\n");
                     return false;
                  }
               }
            }
            connectstate=DevNotConnected;
            break;
        case DevSerialConnected:
            deviceSER->close();
            delete deviceSER;
            deviceSER=0;
            connectstate=DevNotConnected;
            emit disconnected();
            break;
        case DevBTConnected:
            deviceBT.disconnectFromService();
            break;
        default:
            connectstate=DevNotConnected;
    }
    return true;
}

bool IoDevice::isConnected()
{
    return (connectstate==DevNotConnected)?0:1;
}

bool IoDevice::send(const QByteArray &ba)
{
    if(!isConnected())
        return false;
    switch(connectstate)
    {
        case DevSerialConnected:
            deviceSER->write(ba);
            break;
        case DevTCPConnected:
            deviceTCP.write(ba);
            break;
        case DevBTConnected:
            //deviceBT->write(ba);
            deviceBT.write(ba);
            break;
        default:
            break;
    }
    return true;
}


void IoDevice::gotTCPData()
{
    QByteArray ba=deviceTCP.readAll();
    emit readyRead(ba);
}

void IoDevice::gotBTData()
{
    QByteArray ba=deviceBT.readAll();
    emit readyRead(ba);
}
void IoDevice::gotSERData()
{
    QByteArray ba=deviceSER->readAll();
    emit readyRead(ba);
}
void IoDevice::errTCP(QAbstractSocket::SocketError)
{
   printf("Socket error\n");
}




void IoDevice::BTConnected()
{
    printf("BT Connected\n");
    //QMessageBox::critical(this, "bt", "connected");
    emit connected();
}

void IoDevice::BTDisconnected()
{
    printf("BT Disconnected\n");
    emit disconnected();
    //if(deviceBT)
    //{
        //delete deviceBT;
      //  deviceBT=0;
    //}
    connectstate=DevNotConnected;
}
void IoDevice::BTError(QBluetoothSocket::SocketError err)
{
    qDebug("Bluetooth error code: %d\n",err);

    switch(err)
    {
        case QAbstractSocket::SocketAddressNotAvailableError:
            QMessageBox::critical(this, "Bluetooth error", "Connection failed");
            break;
        case QAbstractSocket::NetworkError:
            QMessageBox::critical(this, "Bluetooth error", "Connection lost");
            break;
        default:
            QMessageBox::critical(this, "Bluetooth error", "Error code "+QString::number(err));
            break;
    }

    qDebug("Bt is open\n");
    //deviceBT->disconnectFromService();
    deviceBT.disconnectFromService();
    //deviceBT.close();

    emit disconnected();
    //deviceBT->close();
    //deviceBT->deleteLater();
    //delete deviceBT;
    //deviceBT=0;

    connectstate=DevNotConnected;
}


void IoDevice::TCPConnected()
{
    printf("TCP Connected\n");
    emit connected();
}

void IoDevice::TCPDisconnected()
{
    printf("TCP Disconnected\n");
    emit disconnected();
}


