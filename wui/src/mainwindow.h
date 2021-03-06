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
#include <QLabel>

#include "maincontroller.h"
#include "menuwidget.h"
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
    void keyReleaseEvent(QKeyEvent *ev);

private slots:
    void start();
    void updateText(QString text);
    void highlighWord(const TextPosition &position);
    void updatePreview(const cv::Mat &img);

    void mainMenu();
    void bluetoothMenu();
    void bluetoothScanMenu();
    void bluetoothPairedMenu();
    void langugesMenu();
    void optionsMenu();
    void aboutMenu();

    void onDeviceScanningError(QBluetoothDeviceDiscoveryAgent::Error error, const QString &errorStr);
    void onDeviceScanningFinished();
    void onScanningTimer();
    void onBluetoothConnected(const QString &name);
    void onBluetoothConnectionError(const QString &name);
    void onBluetoothUnpaired(int, const QString &);
    void onPauseResumeButton();

private:
    void setCursorAtPosition(const TextPosition &position, QTextCursor &cursor);
    void addDiscoveredDevicesToMenu();
    void ShowButtons(bool bShow);

private:
    Ui::MainWindow *ui;
    MainController m_controller;
    TextPosition m_prevPosition;
    QTextCharFormat m_prevFormat;
    cv::Mat m_prevImg;
    bool m_bSavePreviewImage = false, m_bPreviewOn = false, m_bShowButtons = false;

    QAction m_actionExit;
    BluetoothHandler m_bluetoothHandler;
    QTimer m_scanningTimer;
    MenuWidget *m_pMenuWidget = NULL;
    QWidget *m_pCameraViewPage = NULL;
    QLabel *m_pCameraView = NULL;

};
