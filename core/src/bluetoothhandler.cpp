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

    connect(&m_localDevice, &QBluetoothLocalDevice::pairingFinished,
            this, &BluetoothHandler::onPairingFinished);

    connect(&m_localDevice, &QBluetoothLocalDevice::pairingDisplayConfirmation,
            this, [](const QBluetoothAddress &address, QString pin){
                qDebug() << "Pairing display confirmation" << address << pin;
            });

    connect(&m_localDevice, &QBluetoothLocalDevice::pairingDisplayPinCode,
            this, [](const QBluetoothAddress &address, QString pin){
                qDebug() << "Pairing display pin code" << address << pin;
            });

    connect(&m_localDevice, &QBluetoothLocalDevice::error,
            this, [this](QBluetoothLocalDevice::Error error){
                qDebug() << "Local device error" << error;
                emit connectionError(m_remoteDeviceInfo.name());
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
    }
}

const QVector<QBluetoothDeviceInfo> &BluetoothHandler::devices() const
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

void BluetoothHandler::startDeviceDiscovery(int timeout)
{
    // Start a discovery
    m_remotes.clear();
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(timeout);
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void BluetoothHandler::stopDeviceDiscovery()
{
    m_discoveryAgent->stop();
}

void BluetoothHandler::startPairing(int index)
{
    m_remoteDeviceInfo = m_remotes.at(index);
    m_localDevice.requestPairing(m_remoteDeviceInfo.address(), QBluetoothLocalDevice::Paired);
}

void BluetoothHandler::prepareConnectedDevices()
{
    // Get connected devices
//    m_remotes = m_localDevice.connectedDevices();

//    qDebug() << "Name" << m_localDeviceName;
//    qDebug() << "Found" << m_remotes.size() << "connected devices";
//    for (const auto &remote : qAsConst(m_remotes)) {
//        qDebug() << remote.toString() << remote.toUInt64();
//    }
}

void BluetoothHandler::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')' << device.majorDeviceClass();
//    if (device.majorDeviceClass() == QBluetoothDeviceInfo::AudioVideoDevice) {
        m_remotes.append(device);
//        qDebug() << "added!";
//    } else {
//        qDebug() << "skipped";
//    }
}

void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    qDebug() << "Pairing finished" << address << pairing;
    m_controller = QLowEnergyController::createCentral(m_remoteDeviceInfo, this);

    // Setup low energry controller
    connect(m_controller, &QLowEnergyController::serviceDiscovered,
            this, [](const QBluetoothUuid &newService){
                qDebug() << "Service discovered" << newService;
            });

    connect(m_controller, &QLowEnergyController::discoveryFinished,
            this, [](){
                qDebug() << "Service discovery finished";
            });

    connect(m_controller, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error error) {
                qDebug() << "Cannot connect to remote device." << error;
                emit connectionError(m_remoteDeviceInfo.name());
            });

    connect(m_controller, &QLowEnergyController::connected, this, [this]() {
        qDebug() << "Controller connected. Search services...";
        emit connected(m_remoteDeviceInfo.name());
        // m_controller->discoverServices();
    });

    connect(m_controller, &QLowEnergyController::disconnected, this, []() {
        qDebug() << "LowEnergy controller disconnected";
    });

    m_controller->connectToDevice();
}
