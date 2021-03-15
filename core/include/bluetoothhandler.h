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
#include <QLowEnergyController>

class BluetoothHandler : public QObject
{
    Q_OBJECT

public:
    BluetoothHandler(QObject *parent = nullptr);

    bool isValid() const;
    void startDeviceDiscovery(int timeout);
    void stopDeviceDiscovery();

    void startPairing(int index);
    void prepareConnectedDevices();

    const QVector<QBluetoothDeviceInfo> &devices() const;
    QStringList deviceNames() const;

signals:
    void deviceDiscoveryFinished();
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error,
                              const QString &errorStr);
    void connected(const QString &name);
    void connectionError(const QString &name);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

private:
    void init();

private:
    QBluetoothLocalDevice           m_localDevice;
    QBluetoothDeviceInfo            m_remoteDeviceInfo;
    QVector<QBluetoothDeviceInfo>   m_remotes;
    QString                         m_localDeviceName;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QLowEnergyController           *m_controller {nullptr};
};

