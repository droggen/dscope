/*
   Copyright (C) 2017-2018:
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
 *
 *
 * */
/*
 * TODO:
 * Open error: what to do?
 * ISSUES
 *  Bluetooth used as a static QBluetoothSocket leads to crash after closing and reopening the connection. Instead, using a pointer to QBluetoothSocket (deleteing and newing) works
*/

#include "iodev.h"

IoDevice::IoDevice(QObject *parent) :
    QObject(parent)
    , deviceSER(parent)
#if BTPOINTER==0
    , deviceBT(QBluetoothServiceInfo::RfcommProtocol,parent)
#endif
{
    connectiondata.type = DevNotConnected;


    // Connect and parametrise BT device
#if BTPOINTER==0
    deviceBT.setPreferredSecurityFlags(QBluetooth::NoSecurity);
    connect(&deviceBT, SIGNAL(readyRead()), this, SLOT(BTGotData()));
    connect(&deviceBT, SIGNAL(connected()), this, SLOT(BTConnected()));
    connect(&deviceBT, SIGNAL(disconnected()), this, SLOT(BTDisconnected()));
    connect(&deviceBT, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(BTError(QBluetoothSocket::SocketError)));
#else
    deviceBT=0;
#endif

    // Serial
    connect(&deviceSER,SIGNAL(readyRead()),this,SLOT(SERGotData()));
    connect(&deviceSER,SIGNAL(errorOccurred(QSerialPort::SerialPortError)),this,SLOT(SERError(QSerialPort::SerialPortError)));

    // Connect TCP device - can be done as construct time
    connect(&deviceTCP, SIGNAL(readyRead()), this, SLOT(TCPGotData()));
    connect(&deviceTCP, SIGNAL(connected()), this, SLOT(TCPConnected()));
    connect(&deviceTCP, SIGNAL(disconnected()), this, SLOT(TCPDisconnected()));
    connect(&deviceTCP, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(TCPErr(QAbstractSocket::SocketError)));
}

/************************************
Return value:
    false: error
    true: success
************************************/
bool IoDevice::open(ConnectionData cd)
{
    qDebug("IoDevice::open\n");
    printf("IoDevice::open\n");

    // If we're connected we return an error
    if(isConnected())
    {
        return false;
    }

    // Update the connection info
    this->connectiondata=cd;

    // Here we should be disconnected. Connect to target device
    switch(cd.type)
    {
        case DevTCPConnection:
            qDebug("Connecting to TCP\n");
            //printf("host: %s\n",cd.tcphost.toStdString().c_str());
            deviceTCP.connectToHost(cd.tcphost,cd.tcpport);
            break;
        case DevSerialConnection:
            //qDebug("Connecting to SER %s %d\n",cd.port.toStdString().c_str(),cd.baud);
            deviceSER.setPortName(cd.port);
            //deviceSER.setBaudRate(cd.baud);
            deviceSER.setDataBits(QSerialPort::Data8);
            deviceSER.setFlowControl(QSerialPort::NoFlowControl);
            deviceSER.setParity(QSerialPort::NoParity);
            deviceSER.setStopBits(QSerialPort::OneStop);

            deviceSER.open(QIODevice::ReadWrite);

            if(!deviceSER.isOpen())
            {
                printf("IoDevice: Serial: cannot open. Check port:speed settings.\n");
                emit connectionError();
                return false;
            }
            else
            {
                printf("IoDevice: Serial: Open successful\n");
                emit connected();
                return true;
            }

            break;
        case DevBTConnection:
            qDebug("Connecting to BT\n");
            printf("Connecting to BT\n");

#if BTPOINTER==0
#else
            if(deviceBT)
            {
                delete deviceBT;
                deviceBT=0;
            }
            deviceBT = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol,this);

            // Initialise here
            deviceBT->setPreferredSecurityFlags(QBluetooth::NoSecurity);
            connect(deviceBT, SIGNAL(readyRead()), this, SLOT(BTGotData()));
            connect(deviceBT, SIGNAL(connected()), this, SLOT(BTConnected()));
            connect(deviceBT, SIGNAL(disconnected()), this, SLOT(BTDisconnected()));
            connect(deviceBT, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(BTError(QBluetoothSocket::SocketError)));
#endif

#if BTPOINTER==0
            // Connect to remote; the uuid corresponds to rfcomm
            #ifdef Q_OS_ANDROID
                deviceBT.connectToService(cd.ba,QBluetoothUuid(QString("00001101-0000-1000-8000-00805f9b34fb")));        // Works on android but not linux
            #else
                deviceBT.connectToService(cd.ba,QBluetoothUuid::SerialPort);       // Works on linux, does not work on android
            #endif
#else
            // Connect to remote; the uuid corresponds to rfcomm
            #ifdef Q_OS_ANDROID
                deviceBT->connectToService(cd.ba,QBluetoothUuid(QString("00001101-0000-1000-8000-00805f9b34fb")));        // Works on android but not linux
            #else
                deviceBT->connectToService(cd.ba,QBluetoothUuid::SerialPort);       // Works on linux, does not work on android
            #endif
#endif

            break;
        default:
            break;
    }
    return true;

}
bool IoDevice::close()
{
    printf("IoDevice: Closing. (type: %d)\n",connectiondata.type);
    // We are connected, therefore we disconnect
    switch(connectiondata.type)
    {
        case DevTCPConnection:
            if( deviceTCP.state() != QAbstractSocket::UnconnectedState )
            {
               deviceTCP.disconnectFromHost();
               if(deviceTCP.state() != QAbstractSocket::UnconnectedState)
               {
                  if(!deviceTCP.waitForDisconnected())
                  {
                     printf("Could not disconnect from host\n");
                     return false;
                  }
               }
            }
            break;
        case DevSerialConnection:
            if(deviceSER.isOpen())
                deviceSER.close();
            emit disconnected();
            break;
        case DevBTConnection:
#if BTPOINTER==0
            if( deviceBT.state() != QAbstractSocket::UnconnectedState )
            {
                deviceBT.disconnectFromService();
                // No waitForDisconnected exists
            }
#else
            if( deviceBT->state() != QAbstractSocket::UnconnectedState )
            {
                deviceBT->disconnectFromService();
                // No waitForDisconnected exists
            }
#endif
            break;
    }
    return true;
}

bool IoDevice::isConnected()
{
    switch(connectiondata.type)
    {
        case DevTCPConnection:
            if( deviceTCP.state() == QAbstractSocket::ConnectedState )
                return true;
            return false;
            break;
        case DevSerialConnection:
            return deviceSER.isOpen();
            break;
        case DevBTConnection:
#if BTPOINTER==0
            printf("IoDevice: isConnected: bt state %s\n",BTStateToString(deviceBT.state()).toStdString().c_str());
            if(deviceBT.state() == QAbstractSocket::ConnectedState)
                return true;
#else
            printf("IoDevice: isConnected: bt state %s\n",BTStateToString(deviceBT->state()).toStdString().c_str());
            if(deviceBT->state() == QAbstractSocket::ConnectedState)
                return true;
#endif
            return false;

            //return deviceBT.isOpen();
            break;
    }
    return false;
}

bool IoDevice::send(const QByteArray &ba)
{
    if(!isConnected())
        return false;
    switch(connectiondata.type)
    {
        case DevSerialConnection:
            deviceSER.write(ba);
            break;
        case DevTCPConnection:
            deviceTCP.write(ba);
            break;
        case DevBTConnection:
#if BTPOINTER==0
            deviceBT.write(ba);
#else
            deviceBT->write(ba);
#endif
            break;
        default:
            break;
    }
    return true;
}




void IoDevice::SERGotData()
{
    QByteArray ba=deviceSER.readAll();
    emit readyRead(ba);
}
void IoDevice::SERError(QSerialPort::SerialPortError err)
{
    printf("IoDevice: Serial error: %d. isOpen: %d\n",err,deviceSER.isOpen());
    if(err == QSerialPort::NoError)
    {
        // No error: do nothing
        //emit connected();
        return;
    }
    // Some error occurred - close the device and emit an error
    QString str=QString("IoDevice: serial error %1").arg(err);

    emit error(str);

    // Close the device if open
    close();


}



void IoDevice::BTGotData()
{
#if BTPOINTER==0
    QByteArray ba=deviceBT.readAll();
#else
    QByteArray ba=deviceBT->readAll();
#endif
    emit readyRead(ba);
}
void IoDevice::BTConnected()
{
    printf("BT Connected\n");
    //QMessageBox::critical(this, "bt", "connected");
    emit connected();
}

void IoDevice::BTDisconnected()
{
    printf("IoDevice: BT Disconnected\n");
    emit disconnected();

}
void IoDevice::BTError(QBluetoothSocket::SocketError err)
{

    qDebug("IoDevice: Bluetooth error code: %d\n",err);

#if BTPOINTER==0
    QString se = BTStateToString(deviceBT.state());
    printf("IoDevice: BT: error: %d state: %d (%s). isOpen: %d\n",err,deviceBT.state(),se.toStdString().c_str(),deviceBT.isOpen());
#else
    QString se = BTStateToString(deviceBT->state());
    printf("IoDevice: BT: error: %d state: %d (%s). isOpen: %d\n",err,deviceBT->state(),se.toStdString().c_str(),deviceBT->isOpen());
#endif








    QString errstr;

    switch(err)
    {
        case QAbstractSocket::SocketAddressNotAvailableError:
            /*errstr = QString("Bluetooth error: connection failed");
            qDebug(errstr.toLatin1());
            emit error(errstr);*/
            //deviceBT.disconnectFromService();       // try
            emit connectionError();
            break;
        case QAbstractSocket::NetworkError:
            errstr = QString("Bluetooth error: connection lost");
            qDebug(errstr.toLatin1());
            emit error(errstr);
            break;
        case QBluetoothSocket::UnknownSocketError:
            errstr = QString("Bluetooth error: unknown socket error");
            qDebug(errstr.toLatin1());
            emit error(errstr);
            break;
        case QBluetoothSocket::RemoteHostClosedError:
            /*  If RemoteHostClosedError, the device still reports connected and open and no disconnect signal
                is sent.
                Explicitly disconnect from service. This triggers a disconnect signal.
            */

            errstr = QString("Bluetooth error: RemoteHostClosedError");
            qDebug(errstr.toLatin1());
            //emit error(errstr);

#if BTPOINTER==0
            deviceBT.disconnectFromService();
            se = BTStateToString(deviceBT.state());
            printf("after disconnect from service: deviceBT state: %d (%s). isOpen: %d\n",deviceBT.state(),se.toStdString().c_str(),deviceBT.isOpen());
#else
            deviceBT->disconnectFromService();
            se = BTStateToString(deviceBT->state());
            printf("after disconnect from service: deviceBT state: %d (%s). isOpen: %d\n",deviceBT->state(),se.toStdString().c_str(),deviceBT->isOpen());
#endif

            break;
        default:
            errstr = QString("Bluetooth error code ")+QString::number(err);
            qDebug(errstr.toLatin1());
            emit error(errstr);
            break;
    }






}

void IoDevice::TCPGotData()
{
    QByteArray ba=deviceTCP.readAll();
    emit readyRead(ba);
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
void IoDevice::TCPErr(QAbstractSocket::SocketError err)
{
    printf("IoDevice: TCP: Socket error %d\n",err);
    switch(err)
    {
        case QAbstractSocket::ConnectionRefusedError:
            emit connectionError();
            break;
        default:
            emit error(QString("IoDevice: TCP socket error %1").arg(err));
    }
}





/*
    Parse the connection string

    Connection string
    <number>                TCP localhost on port <number>
    <number1>:<number2>     TCP host <number1> port <number2>
    <string>                COM port <string>
    <MACstring>             BT port <MACstring> when MACstring is of the type aa:aa:aa:aa:aa:aa

    Returns:
        false:              Failure
        true:               Ok
*/
bool IoDevice::ParseConnection(QString str, ConnectionData &conn)
{
    conn.type = DevNotConnected;
    conn.port="";
    conn.tcphost="";
    conn.tcpport=0;
    conn.ba.clear();

    // Split string at colons
    QStringList parts=str.split(':');
    //printf("num semi: %d\n",parts.size());
    if(parts.size()==1)
    {
        // One element: TCP or com.

        // Try TCP: must be numerical value
        bool ok;
        int port = parts[0].toInt(&ok);
        if(ok)
        {
            conn.type = DevTCPConnection;
            conn.tcphost="127.0.0.1";
            conn.tcpport = port;
            return true;
        }

        // Falls through: must be COM port
        conn.type = DevSerialConnection;
        conn.port = parts[0];
        conn.baud = QSerialPort::Baud19200;
        return true;

    }
    if(parts.size()==2)
    {
        // Must be TCP address:port
        conn.type = DevTCPConnection;
        bool ok;
        int port = parts[1].toInt(&ok);
        if(ok)
            conn.tcpport = port;
        else
            return false;

        // part[0] will be port - we don't parse as hostname can be quite arbitrary
        conn.tcphost = parts[0];
        return true;
    }
    if(parts.size()==6)
    {
        // BT MAC address
        int mac[6];

        for(int i=0;i<6;i++)
        {
            bool ok;
            mac[i] = parts[i].toInt(&ok,16);
            if(!ok)
                return false;
            // Check number is in 0-255
            if(mac[i]<0 || mac[i]>255)
                return false;
        }

        QString MAC;
        MAC.sprintf("%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        printf("mac: %s\n",MAC.toStdString().c_str());
        conn.type = DevBTConnection;
        conn.ba = QBluetoothAddress(MAC);
        return true;
    }




    return false;
}


QString IoDevice::ConnectionToString(const ConnectionData &conn)
{
    QString s="";
    switch(conn.type)
    {
        case DevTCPConnection:
            s = "TCP:" + conn.tcphost+":"+QString::number(conn.tcpport);
            break;
        case DevSerialConnection:
            s = "SER:" + conn.port;
            break;
        case DevBTConnection:
            s = "BT:" + conn.ba.toString();
            break;
        default:
            s = "UNKNOWN";
    }
    return s;
}

QString IoDevice::BTStateToString(QBluetoothSocket::SocketState state)
{
    switch(state)
    {
        case QBluetoothSocket::SocketState::UnconnectedState:
            return "UnconnectedState";
        case QBluetoothSocket::SocketState::ServiceLookupState:
            return "ServiceLookupState";
        case QBluetoothSocket::SocketState::ConnectingState:
            return "ConnectingState";
        case QBluetoothSocket::SocketState::ConnectedState:
            return "ConnectedState";
        case QBluetoothSocket::SocketState::BoundState:
            return "BoundState";
        case QBluetoothSocket::SocketState::ClosingState:
            return "ClosingState";
        case QBluetoothSocket::SocketState::ListeningState:
            return "ListeningState";
        default:
            return "(Unknown state)";
    }



}
