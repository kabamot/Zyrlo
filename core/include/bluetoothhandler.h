/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Mar 2021
 *
 ****************************************************************************/

#pragma once

#include <QObject>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>

class BluetoothHandler : public QObject
{
    Q_OBJECT

public:
    BluetoothHandler(QObject *parent = nullptr);

    bool isValid() const;
    void startDeviceDiscovery();
    void stopDeviceDiscovery();

    QVector<QBluetoothDeviceInfo> devices() const;
    QStringList deviceNames() const;

signals:
    void deviceDiscoveryFinished();
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error,
                              const QString &errorStr);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

private:
    void init();

private:
    QBluetoothLocalDevice           m_localDevice;
    QVector<QBluetoothDeviceInfo>   m_remotes;
    QString                         m_localDeviceName;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
};

