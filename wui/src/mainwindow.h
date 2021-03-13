/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#pragma once

#include <QMainWindow>
#include <QTextCharFormat>
#include <QAction>
#include <QTimer>

#include "maincontroller.h"
#include "textposition.h"
#include "bluetoothhandler.h"
#include <opencv2/opencv.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTextCursor;
class QGridLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *ev);

private slots:
    void start();
    void updateText(QString text);
    void highlighWord(const TextPosition &position);
    void updatePreview(const cv::Mat &img);

    void mainMenu();
    void bluetoothMenu();
    void bluetoothScanMenu();
    void bluetoothPairedMenu();

    void onDeviceScanningError(QBluetoothDeviceDiscoveryAgent::Error error, const QString &errorStr);
    void onDeviceScanningFinished();
    void onScanningTimer();

private:
    void setCursorAtPosition(const TextPosition &position, QTextCursor &cursor);
    void addDiscoveredDevicesToMenu();

private:
    Ui::MainWindow *ui;
    MainController m_controller;
    TextPosition m_prevPosition;
    QTextCharFormat m_prevFormat;
    cv::Mat m_prevImg;
    bool m_bSavePreviewImage = false, m_bPreviewOn = false;

    QAction m_actionExit;
    BluetoothHandler m_bluetoothHandler;
    QTimer m_scanningTimer;
};
