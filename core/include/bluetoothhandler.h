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
#include <QElapsedTimer>

class BluetoothHandler : public QObject
{
    Q_OBJECT

public:
    BluetoothHandler(QObject *parent = nullptr);

    bool isValid() const;
    void startDeviceDiscovery();
    void stopDeviceDiscovery();

    void startPairing(int index);
    void prepareConnectedDevices();
    void unpair(int index);

    const QVector<QBluetoothDeviceInfo> &devices() const;
    QStringList deviceNames() const;
    QStringList pairedDeviceNames() const;

signals:
    void deviceDiscoveryFinished();
    void deviceDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error,
                              const QString &errorStr);
    void connected(const QString &name);
    void connectionError(const QString &name);
    void unpaired(int index, const QString &name);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

private:
    void init();
    QStringList getDeviceNames(const QVector<QBluetoothDeviceInfo> &remotes) const;
    bool contains(const QBluetoothDeviceInfo &remoteDevice,
                  const QVector<QBluetoothDeviceInfo> &remotes) const;
    void loadSettings();
    void saveSettings();

private:
    QBluetoothLocalDevice           m_localDevice;
    QBluetoothDeviceInfo            m_remoteDeviceInfo;
    QVector<QBluetoothDeviceInfo>   m_remotes;
    QVector<QBluetoothDeviceInfo>   m_pairedRemotes;
    QString                         m_localDeviceName;
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QLowEnergyController           *m_controller {nullptr};
    QElapsedTimer                   m_connectionTimer;
    int                             m_index;
};

