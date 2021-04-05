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
#include <QThread>
#include <QDebug>
#include <QSettings>

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

        loadSettings();
    }
}

QStringList BluetoothHandler::getDeviceNames(const QVector<QBluetoothDeviceInfo> &remotes) const
{
    QStringList names;
    for (const auto &device : qAsConst(remotes)) {
        names.append(device.name());
    }

    return names;
}

bool BluetoothHandler::contains(const QBluetoothDeviceInfo &remoteDevice,
                                const QVector<QBluetoothDeviceInfo> &remotes) const
{
    for (const auto &remote : remotes) {
        if (remote.address() == remoteDevice.address())
            return true;
    }

    return false;
}

void BluetoothHandler::loadSettings()
{
    QSettings settings;
    settings.beginGroup("bluetooth");
    QVariantList list = settings.value("pairedDevices").toList();
    settings.endGroup();

    m_pairedRemotes.clear();
    for (const auto &item : qAsConst(list)) {
        QVariantMap map = item.toMap();
        QString name = map.value("name").toString();
        QBluetoothAddress address(map.value("address").toString());
        m_pairedRemotes.append(QBluetoothDeviceInfo(address, name, 0));
    }
}

void BluetoothHandler::saveSettings()
{
    QVariantList list;
    for (const auto &remote : qAsConst(m_pairedRemotes)) {
        QVariantMap map;
        map["name"] = remote.name();
        map["address"] = remote.address().toString();
        list.append(map);
    }

    QSettings settings;
    settings.beginGroup("bluetooth");
    settings.setValue("pairedDevices", list);
    settings.endGroup();
}

const QVector<QBluetoothDeviceInfo> &BluetoothHandler::devices() const
{
    return m_remotes;
}

QStringList BluetoothHandler::deviceNames() const
{
    return getDeviceNames(m_remotes);
}

QStringList BluetoothHandler::pairedDeviceNames() const
{
    return getDeviceNames(m_pairedRemotes);
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

void BluetoothHandler::startPairing(int index)
{
    m_remoteDeviceInfo = m_remotes.at(index);
    m_localDevice.requestPairing(m_remoteDeviceInfo.address(), QBluetoothLocalDevice::AuthorizedPaired);
}

void BluetoothHandler::prepareConnectedDevices()
{
    // Get connected devices
    auto remotes = m_localDevice.connectedDevices();

    qDebug() << "Name" << m_localDeviceName;
    qDebug() << "Found" << m_remotes.size() << "connected devices";
    for (const auto &remote : qAsConst(remotes)) {
        qDebug() << remote.toString() << remote.toUInt64();
    }
}

void BluetoothHandler::unpair(int index)
{
    m_index = index;
    m_remoteDeviceInfo = m_pairedRemotes.at(index);
    m_localDevice.requestPairing(m_remoteDeviceInfo.address(), QBluetoothLocalDevice::Unpaired);
}

void BluetoothHandler::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')'
             << "major" << device.majorDeviceClass() << "minor" << device.minorDeviceClass();
    if (device.majorDeviceClass() == QBluetoothDeviceInfo::AudioVideoDevice
        && !contains(device, m_remotes) && !contains(device, m_pairedRemotes) ) {
        m_remotes.append(device);
        qDebug() << "added!";
    } else {
        qDebug() << "skipped";
    }
}

void BluetoothHandler::onPairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
    if (pairing == QBluetoothLocalDevice::Paired || pairing == QBluetoothLocalDevice::AuthorizedPaired) {
        // Paired
        qDebug() << "Pairing finished" << address << pairing;
        if (!contains(m_remoteDeviceInfo, m_pairedRemotes)) {
            m_pairedRemotes.append(m_remoteDeviceInfo);
        }
        saveSettings();
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
            qDebug() << "Controller connected.";
            qDebug() << m_localDevice.hostMode();
            emit connected(m_remoteDeviceInfo.name());
        });

        connect(m_controller, &QLowEnergyController::disconnected, this, [this]() {
            qDebug() << "LowEnergy controller disconnected";
            qDebug() << m_localDevice.hostMode();
            qDebug() << "elapsed" << m_connectionTimer.elapsed();
            if (m_connectionTimer.elapsed() < 6000) {
                qDebug() << "It was automatically disconnected and now connecting again";
                m_controller->connectToDevice();
            }
        });

        m_connectionTimer.start();
        m_controller->connectToDevice();
    } else if(m_index >=0 && m_index < m_pairedRemotes.size()) {
        // Unpaired
        m_pairedRemotes.removeAt(m_index);
        saveSettings();
        qDebug() << "Device unpaired" << address << "it's left" << m_pairedRemotes.size() << "paired devices";
        emit unpaired(m_index, m_remoteDeviceInfo.name());
    }
}
