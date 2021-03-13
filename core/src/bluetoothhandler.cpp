/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Mar 2021
 *
 ****************************************************************************/

#include "bluetoothhandler.h"

#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QDebug>

BluetoothHandler::BluetoothHandler(QObject *parent)
    : QObject(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothHandler::deviceDiscovered);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothHandler::deviceDiscoveryFinished);

    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, [this](QBluetoothDeviceDiscoveryAgent::Error error){
                emit deviceDiscoveryError(error, m_discoveryAgent->errorString());
            });

    init();
}

bool BluetoothHandler::isValid() const
{
    return m_localDevice.isValid();
}

void BluetoothHandler::init()
{
    // Check if Bluetooth is available on this device
    if (m_localDevice.isValid()) {

        // Turn Bluetooth on
        m_localDevice.powerOn();

        // Read local device name
        m_localDeviceName = m_localDevice.name();

        // Make it visible to others
        m_localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

        // Get connected devices
//        m_remotes = m_localDevice.connectedDevices();

//        qDebug() << "Name" << m_localDeviceName;
//        qDebug() << "Found" << m_remotes.size() << "connected devices";
//        for (const auto &remote : qAsConst(m_remotes)) {
//            qDebug() << remote.toString() << remote.toUInt64();
//        }
    }
}

QVector<QBluetoothDeviceInfo> BluetoothHandler::devices() const
{
    return m_remotes;
}

QStringList BluetoothHandler::deviceNames() const
{
    QStringList names;
    for (const auto &device : qAsConst(m_remotes)) {
        names.append(device.name());
    }

    return names;
}

void BluetoothHandler::startDeviceDiscovery()
{
    // Start a discovery
    m_remotes.clear();
    m_discoveryAgent->start();
}

void BluetoothHandler::stopDeviceDiscovery()
{
    m_discoveryAgent->stop();
}

void BluetoothHandler::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')';
    m_remotes.append(device);
}
