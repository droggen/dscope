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
 * iodev
 *
 * Generic IO device for bluetooth, serial and tcp
 *
 * */

#ifndef __IODEV_H
#define __IODEV_H

#include <QObject>
#include <QBluetoothAddress>
#include <QSerialPort>
#include <QtSerialPort/QSerialPort>
#include <QTcpSocket>
#include <QBluetoothSocket>


// Whether to use a member or pointer. Pointer seems to avoid a crash after closing/opening
#define BTPOINTER 1

typedef enum __connect_t {DevNotConnected,DevTCPConnection,DevSerialConnection,DevBTConnection} connect_t ;

typedef struct __ConnectionData
{
    connect_t type;

    QString message;

    // For Bluetooth
    QBluetoothAddress ba;

    // For serial
    QString port;
    QSerialPort::BaudRate baud;
    // For TCP
    QString tcphost;
    int tcpport;

} ConnectionData;



class IoDevice : public QObject
{
    Q_OBJECT

private:
    ConnectionData connectiondata;

    QSerialPort deviceSER;
    QTcpSocket deviceTCP;
#if BTPOINTER==0
    QBluetoothSocket deviceBT;
#else
    QBluetoothSocket *deviceBT;
#endif


public:
    explicit IoDevice(QObject *parent = nullptr);

    static bool ParseConnection(QString str, ConnectionData &conn);
    static QString ConnectionToString(const ConnectionData &conn);
    static QString BTStateToString(QBluetoothSocket::SocketState state);

    bool open(ConnectionData cd);
    bool close(void);
    bool isConnected();
    bool send(const QByteArray &ba);


signals:
    void readyRead(QByteArray b);
    void connected();
    void disconnected();
    void error(QString errmsg);
    void connectionError();

private slots:
    void BTGotData();
    void BTConnected();
    void BTDisconnected();
    void BTError(QBluetoothSocket::SocketError);

    void SERGotData();
    void SERError(QSerialPort::SerialPortError err);

    void TCPGotData();
    void TCPConnected();
    void TCPDisconnected();
    void TCPErr(QAbstractSocket::SocketError);

};

#endif // IODEV_H

