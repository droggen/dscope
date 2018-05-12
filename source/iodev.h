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

#include <QtWidgets>
#include <QObject>
#include <QBluetoothAddress>
#include <QSerialPort>
#include <QtSerialPort/QSerialPort>
#include <QTcpSocket>
#include <QBluetoothSocket>

typedef enum __connect_t {DevNotConnected,DevTCPConnected,DevSerialConnected,DevBTConnected} connect_t ;

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



class IoDevice : public QWidget
{
    Q_OBJECT

private:
    connect_t connectstate;

    QSerialPort *deviceSER;
    QTcpSocket deviceTCP;
    QBluetoothSocket deviceBT;


public:
    explicit IoDevice(QWidget *parent = 0);

    bool open(ConnectionData cd);
    bool close(void);
    bool isConnected();
    bool send(const QByteArray &ba);


signals:
    void readyRead(QByteArray b);
    void connected();
    void disconnected();

private slots:
    void gotTCPData();
    void gotBTData();
    void gotSERData();
    void BTConnected();
    void BTDisconnected();
    void BTError(QBluetoothSocket::SocketError);
    void TCPConnected();
    void TCPDisconnected();
    void errTCP(QAbstractSocket::SocketError);

};

#endif // IODEV_H

