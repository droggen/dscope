#ifndef __BTWINDOW_H
#define __BTWINDOW_H


#include <QDialog>
#include <QList>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>

namespace Ui {
    class BtWindow;
}

class BtWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BtWindow(QString *bt,QWidget *parent = 0);
    ~BtWindow();

private:
    Ui::BtWindow *ui;

    QString *bt;

    QList<QBluetoothDeviceInfo> bdi;
    void populateTable();
    QString coreConfigurationToString(QBluetoothDeviceInfo::CoreConfigurations c);



private slots:
    void on_uitwPorts_cellDoubleClicked(int row, int column);
    void deviceDiscovered(QBluetoothDeviceInfo);
    void serviceDiscovered(QBluetoothServiceInfo);
    void deviceFinished();
    void accept();
};

#endif // __BTWINDOW_H
