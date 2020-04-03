#include "parse.h"
#include <stdio.h>
#include <QStringList>
#include <QBluetoothAddress>
#include <QSerialPort>



/*
 * Parse the connection string
 * */
bool ParseConnection(QString str, ConnectionData &conn)
{
    printf("Hello wordl\n");

    // Convert the string to uppercase
    str=str.toUpper();

    // Split string at colons
    QStringList parts=str.split(':');

#if DEVELMODE
    printf("size: %d\n",parts.size());
    for(int i=0;i<parts.size();i++)
    {
        printf("Part %d: '%s'\n",i,parts[i].toStdString().c_str());
    }
#endif

    // Make sure that there are 2 or more arguments
    if(parts.size()<2)
    {
        #if DEVELMODE
        printf("Not enough params\n");
        #endif
        return false;
    }

    // Trim all the parts
    for(int i=0;i<parts.size();i++)
        parts[i]=parts[i].trimmed();

    // Check that the device type is one of bt, ser or tcp
    if(parts[0]!="SER" && parts[0]!="TCP" && parts[0]!="BT")
    {
        #if DEVELMODE
        printf("must be ser, tcp or bt\n");
        #endif
        conn.message="Invalid connection string: the format is <BT|SER|TCP>:<param>[:<param>]";
        return false;
    }

    printf("OK so far\n");

    // Process bluetooth
    if(parts[0]=="BT")
    {
        printf("BT\n");
        // There should be 2 arguments, no more
        if(parts.size()!=2)
        {
            #if DEVELMODE
            printf("There should be exactly two arguments\n");
            #endif
            conn.message="Invalid BT arguments. The format is: BT:btmac";
            return false;
        }
        // Parse parts[1] which should be a MAC address
        conn.ba = QBluetoothAddress(parts[1]);
        printf("BT address: %s\n",conn.ba.toString().toStdString().c_str());
        if(conn.ba.isNull())
        {
            #if DEVELMODE
            printf("Invalid bt address\n");
            #endif
            conn.message="Invalid BT address. The BT address is a 12-digit hex string, without spaces, dashes or colons";
            return false;
        }
        conn.type=DevBTConnection;
        return true;
    }
    // Process serial
    if(parts[0]=="SER")
    {
        printf("SER\n");
        // There should be 3 parts exactly
        if(parts.size()!=3)
        {
            #if DEVELMODE
            printf("There should be exactly three arguments\n");
            #endif
            conn.message="Invalid SER arguments. The format is: SER:comport:baudrate";
            return false;
        }
        // Read the com port
        conn.port=parts[1];

        // Read the speed
        bool ok;
        int spd = parts[2].toInt(&ok);
        if(ok==false || (spd!=110 && spd!=300 && spd!=600 && spd!=1200 && spd!=2400 && spd!=4800 && spd!=9600 && spd!=14400 &&
                spd!=19200 && spd!=38400 && spd!=57600 && spd!=115200))
        {
            #if DEVELMODE
            printf("Invalid baud rate\n");
            #endif
            conn.message="Invalid baud rate";
            return false;
        }
        switch(spd)
        {
           case 1200:
              conn.baud=QSerialPort::Baud1200;
              break;
           case 2400:
              conn.baud=QSerialPort::Baud2400;
              break;
           case 4800:
              conn.baud=QSerialPort::Baud4800;
              break;
           case 9600:
              conn.baud=QSerialPort::Baud9600;
              break;
           case 19200:
              conn.baud=QSerialPort::Baud19200;
              break;
           case 38400:
              conn.baud=QSerialPort::Baud38400;
              break;
           case 57600:
              conn.baud=QSerialPort::Baud57600;
              break;
           default:
              conn.baud=QSerialPort::Baud115200;
        }
        conn.type=DevSerialConnection;
        return true;
    }
    // Process serial
    if(parts[0]=="TCP")
    {
        printf("TCP\n");
        // There should be 3 arguments
        if(parts.size()!=3)
        {
            #if DEVELMODE
            printf("There should be exactly three arguments\n");
            #endif
            conn.message="Invalid TCP arguments. The format is: TCP:host:port";
            return false;
        }
        // Host
        conn.tcphost=parts[1];

        // Port
        bool ok;
        int port = parts[2].toInt(&ok);
        if(ok==false)
        {
            #if DEVELMODE
            printf("Invalid port\n");
            #endif
            conn.message="Invalid TCP port";
            return false;
        }
        conn.tcpport=port;
        conn.type=DevTCPConnection;
        return true;
    }

    return false;
}
