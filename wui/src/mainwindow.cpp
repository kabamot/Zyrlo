/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QtGui>
#include <QTextBlock>
#include <QInputDialog>
#include <regex>
#include <QMessageBox>

#include "menuwidget.h"
#include "bluetoothhandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

using namespace cv;
using namespace std;

constexpr int STATUS_MESSAGE_TIMEOUT = 5000; // ms
constexpr int BLUETOOTH_SCANNING_ANNOUNCEMENT_TIME = 4000; // ms

//#define ZYRLO_ENABLE_ADJUSTMENT_CONTRIOLS

QString GetCpuTemp() {
    FILE *fp;
    char path[256] = {0};

    fp = popen("vcgencmd measure_temp", "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        return "";
    }
    fgets(path, sizeof(path), fp);
    pclose(fp);
    return path;
}

bool IsBtRunning();

void MainWindow::ShowButtons(bool bShow) {
    ui->startButton->setVisible(bShow);
    ui->pauseButton->setVisible(bShow);
    ui->nextWordButton->setVisible(bShow);
    ui->backWordButton->setVisible(bShow);
    ui->nextSentenceButton->setVisible(bShow);
    ui->backSentenceButton->setVisible(bShow);
    ui->spellWordButton->setVisible(bShow);
    ui->rateUpButton->setVisible(bShow);
    ui->rateDownButton->setVisible(bShow);
    ui->nextVoiceButton->setVisible(bShow);
    ui->toggleSinkButton->setVisible(bShow);
    ui->menuButton->setVisible(bShow);
    ui->fileNameLineEdit->setVisible(bShow);
    ui->menubar->setVisible(bShow);
    ui->label->setVisible(bShow);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->readerPage);

    m_pCameraViewPage = new QWidget();
    m_pCameraViewPage->setObjectName(QString::fromUtf8("cameraViewPage"));
    m_pCameraView = new QLabel;
    m_pCameraView->setObjectName(QString::fromUtf8("cameraView"));
    m_pCameraView->setGeometry(QRect(190, 30, 701, 581));
    m_pCameraView->setParent(m_pCameraViewPage);

    ShowButtons(m_bShowButtons);

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::start);
    connect(&m_controller, &MainController::textUpdated, this, &MainWindow::updateText);
    connect(&m_controller, &MainController::wordPositionChanged, this, &MainWindow::highlighWord);
    connect(&m_controller, &MainController::previewUpdated, this, &MainWindow::updatePreview);
    connect(&m_controller, &MainController::openMainMenu, this, &MainWindow::mainMenu);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::onPauseResumeButton);

    connect(ui->nextWordButton, &QPushButton::clicked, &m_controller, &MainController::nextWord);
    connect(ui->backWordButton, &QPushButton::clicked, &m_controller, &MainController::backWord);
    connect(ui->nextSentenceButton, &QPushButton::clicked, &m_controller, &MainController::nextSentence);
    connect(ui->backSentenceButton, &QPushButton::clicked, &m_controller, &MainController::backSentence);
    connect(ui->spellWordButton, &QPushButton::clicked, &m_controller, &MainController::spellCurrentWord);
    connect(ui->rateUpButton, &QPushButton::clicked, &m_controller, &MainController::speechRateUp);
    connect(ui->rateDownButton, &QPushButton::clicked, &m_controller, &MainController::speechRateDown);
    connect(ui->nextVoiceButton, &QPushButton::clicked, &m_controller, &MainController::nextVoice);
    connect(ui->toggleSinkButton, &QPushButton::clicked, &m_controller, &MainController::onToggleAudioSink);

    connect(ui->menuButton, &QPushButton::clicked, this, &MainWindow::mainMenu);

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [this](int index){
        auto *menuWidget = dynamic_cast<MenuWidget *>(ui->stackedWidget->widget(index));
        if (menuWidget) {
            menuWidget->enteredToMenu();
        }
    });

    // Bluetooth handler
    connect(&m_bluetoothHandler, &BluetoothHandler::deviceDiscoveryFinished,
            this, &MainWindow::onDeviceScanningFinished);
    connect(&m_bluetoothHandler, &BluetoothHandler::deviceDiscoveryError, this,
            &MainWindow::onDeviceScanningError);
    connect(&m_bluetoothHandler, &BluetoothHandler::connected, this,
            &MainWindow::onBluetoothConnected);
    connect(&m_bluetoothHandler, &BluetoothHandler::connectionError, this,
            &MainWindow::onBluetoothConnectionError);
    connect(&m_bluetoothHandler, &BluetoothHandler::unpaired, this,
            &MainWindow::onBluetoothUnpaired);

    connect(&m_scanningTimer, &QTimer::timeout, this, &MainWindow::onScanningTimer);
    connect(&m_scanningTimer, &QTimer::timeout, this, &MainWindow::onScanningTimer);

    ui->fileNameLineEdit->setText("/opt/zyrlo/RawFull_000.bmp");
}

MainWindow::~MainWindow() {
    if(m_pCameraView)
        delete m_pCameraView;

    if(m_pCameraViewPage) {
        ui->stackedWidget->removeWidget(m_pCameraViewPage);
        delete m_pCameraViewPage;
    }
    delete ui;
    m_controller.setLed(false);
}

void MainWindow::keyPressEvent(QKeyEvent *ev) {
    if(m_controller.isMenuOpen())
        return;
    switch(ev->key()) {
    case Qt::Key_S:
        if(ev->modifiers() & Qt::CTRL) {
            m_bPreviewOn = !m_bPreviewOn;
            if(m_bPreviewOn) {
                ui->stackedWidget->addWidget(m_pCameraViewPage);
                ui->stackedWidget->setCurrentWidget(m_pCameraViewPage);
            }
            else {
                ui->stackedWidget->removeWidget(m_pCameraViewPage);
                ui->stackedWidget->setCurrentWidget(ui->readerPage);
            }
            qDebug() << "Preview" << (m_bPreviewOn ? "ON" : "OFF");
        }
        else if(ev->modifiers() & Qt::SHIFT) {
            m_bShowButtons = !m_bShowButtons;
            ShowButtons(m_bShowButtons);
        }
        else {
            m_bSavePreviewImage = true;
            qDebug() << "Saving preview image\n";
        }
        break;
    case Qt::Key_H:
        m_controller.setFullResPreview((ev->modifiers() & Qt::SHIFT) == 0);
        break;
    case Qt::Key_F:
        m_controller.flashLed();
        qDebug() << "Flash Led\n";
        break;
    case Qt::Key_I:
        m_controller.snapImage();
        qDebug() << "SnapShot\n";
        break;
    case Qt::Key_T:
        m_controller.sayText(GetCpuTemp());
        qDebug() << "SnapShot\n";
        break;
    case Qt::Key_L:
        if(ev->modifiers() & Qt::SHIFT) {
            m_controller.setLed(false);
            qDebug() << "Light OFF\n";
        }
        else {
            m_controller.setLed(true);
            qDebug() << "Light ON\n";
        }
        break;
    case Qt::Key_N:
        m_controller.onToggleVoice();
        break;
    case Qt::Key_P:
        m_controller.toggleAudioSink();
        break;
    case Qt::Key_F1:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(1);
        else
            m_controller.ReadImage(1);
        break;
    case Qt::Key_F2:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(2);
        else
            m_controller.ReadImage(2);
        break;
    case Qt::Key_F3:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(3);
        else
            m_controller.ReadImage(3);
        break;
    case Qt::Key_F4:
        if(ev->modifiers() & Qt::SHIFT)
            m_controller.SaveImage(4);
        else
            m_controller.ReadImage(4);
        break;
    case Qt::Key_F5:
    {
        m_controller.sayText("Type the serial number");
        bool ok;
        QString text = QInputDialog::getText(this, tr("Keypad MAC"), tr(""), QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty())
            m_controller.write_keypad_config(text.toStdString());
    }
        break;
    case Qt::Key_F6:
        m_controller.SaySN();
        break;
    case Qt::Key_F7:
        m_controller.SetLocalLightFreqTest(ev->modifiers() & Qt::SHIFT);
        break;
    case Qt::Key_E:
        {QMessageBox msgBox;
        msgBox.setText("Current exposure:" + QString::number(m_controller.getCurrentExposure()) + " Gain: "+ QString::number(m_controller.getCurrentGain()));
        msgBox.exec();}
        break;
    case Qt::Key_Q:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.ChangeCameraExposure(1);
      else
          m_controller.ChangeCameraExposure(10);
      break;
  case Qt::Key_Z:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.ChangeCameraExposure(-1);
      else
          m_controller.ChangeCameraExposure(-10);
      break;
#ifdef ZYRLO_ENABLE_ADJUSTMENT_CONTRIOLS
    case Qt::Key_B:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.ChangeCameraExposureStep(1);
      else
          m_controller.ChangeCameraExposureStep(10);
      break;
  case Qt::Key_M:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.ChangeCameraExposureStep(-1);
      else
          m_controller.ChangeCameraExposureStep(-10);
      break;
#else
    case Qt::Key_B:
        if((ev->modifiers() & (Qt::SHIFT | Qt::CTRL)) == (Qt::SHIFT | Qt::CTRL)) {
            m_controller.startBatteryTest();
        }
        break;

#endif
     case Qt::Key_A:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.ChangeCameraGain(1);
      else
          m_controller.ChangeCameraGain(10);
      break;
  case Qt::Key_V:
      if(ev->modifiers() & Qt::SHIFT)
          m_controller.changeVoiceVolume((ev->modifiers() & Qt::CTRL) ? -1 :-10);
          //m_controller.ChangeCameraGain(-1);
      else
          m_controller.changeVoiceVolume((ev->modifiers() & Qt::CTRL) ? 1 :10);
          //m_controller.ChangeCameraGain(-10);
      break;
    case Qt::Key_D:
        {QMessageBox msgBox;
        msgBox.setText("Current exposure step:" + QString::number(m_controller.getExposureStep()));
        msgBox.exec();}
        break;
#ifdef ZYRLO_ENABLE_ADJUSTMENT_CONTRIOLS
    case Qt::Key_1:
        if(ev->modifiers() & Qt::CTRL)
            if(m_controller.setSpeakerSetting(1))
                m_controller.sayText("Audio profile 1");
        break;
    case Qt::Key_2:
        if(m_controller.setSpeakerSetting(2))
            m_controller.sayText("Audio profile 2");
        break;
    case Qt::Key_3:
        if(m_controller.setSpeakerSetting(3))
            m_controller.sayText("Audio profile 3");
        break;
    case Qt::Key_4:
        if(m_controller.setSpeakerSetting(4))
            m_controller.sayText("Audio profile 4");
        break;
    case Qt::Key_5:
        if(m_controller.setSpeakerSetting(5))
            m_controller.sayText("Audio profile 5");
        break;
#endif
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev) {
    if(m_controller.isMenuOpen())
        return;
    switch(ev->key()) {
    case Qt::Key_Left:
        m_controller.onLeftArrow();
        break;
    case Qt::Key_Right:
        m_controller.onRightArrow();
        break;
    case Qt::Key_Up:
        m_controller.toggleNavigationMode(false);
        break;
    case Qt::Key_Down:
        m_controller.toggleNavigationMode(true);
        break;
    }
}

void MainWindow::start()
{
    m_prevPosition.clear();
    m_controller.startFile(ui->fileNameLineEdit->text());
}

void MainWindow::updateText(QString text)
{
    const auto currentText = ui->textBrowser->toPlainText();
    if (text.left(currentText.size()) == currentText) {
        // Remove the currentText from the text
        text.remove(0, currentText.size());
    } else {
        // text is not according to the current text, will clear and replace it
        ui->textBrowser->clear();
    }

    QTextCharFormat fmt;
    fmt.setFontPointSize(14);

    QTextCursor cursor(ui->textBrowser->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text, fmt);
}

void MainWindow::highlighWord(const TextPosition &position)
{
    QTextCursor cursor(ui->textBrowser->document());

    if (m_prevPosition.isValid()) {
        // Restore previous position's format
        setCursorAtPosition(m_prevPosition, cursor);
        cursor.setCharFormat(m_prevFormat);
    }

    setCursorAtPosition(position, cursor);
    QTextCharFormat fmt = cursor.charFormat();

    // Save current format and position
    m_prevFormat = fmt;
    m_prevPosition = position;

    // Set new background on the new position
    fmt.setBackground(Qt::yellow);
    cursor.setCharFormat(fmt);

    // Make sure that the current highlighted word is always visible (using scrolling when needed)
    cursor.setPosition(position.absPos());
    ui->textBrowser->setTextCursor(cursor);
}

void MainWindow::setCursorAtPosition(const TextPosition &position, QTextCursor &cursor)
{
    cursor.setPosition(position.absPos(), QTextCursor::MoveAnchor);
    cursor.setPosition(position.absPos() + position.length(), QTextCursor::KeepAnchor);
}

void MainWindow::addDiscoveredDevicesToMenu()
{
    // Add scanned items to the list
    auto menuWidget = dynamic_cast<MenuWidget *>(ui->stackedWidget->currentWidget());
    if (menuWidget) {
        QStringList items = m_bluetoothHandler.deviceNames();
        items.append(m_controller.translateTag(MENU_EXIT));
        menuWidget->setItems(items);
    }
}

void MainWindow::updatePreview(const Mat &img) {
    if(m_bPreviewOn) {
        cvtColor(img, m_prevImg, CV_GRAY2RGB);
        rotate(m_prevImg, m_prevImg, ROTATE_180);
        //ui->previewLabel->setPixmap(QPixmap::fromImage(QImage(m_prevImg.data, m_prevImg.cols, m_prevImg.rows, m_prevImg.step, QImage::Format_RGB888)));
        m_pCameraView->setPixmap(QPixmap::fromImage(QImage(m_prevImg.data, m_prevImg.cols, m_prevImg.rows, m_prevImg.step, QImage::Format_RGB888)));
    }
    if(m_bSavePreviewImage) {
        m_bSavePreviewImage = false;
        imwrite("PreviewImage.bmp", img);
    }
}

void MainWindow::mainMenu()
{
    if(m_pMenuWidget)
        return;
    m_controller.setMenuOpen(true);
    m_controller.pause();

    m_pMenuWidget = new MenuWidget("Main menu", &m_controller, ui->stackedWidget);
    QStringList items{m_controller.translateTag(MENU_BLUETOOTH), m_controller.translateTag(MENU_LANGUAGE),  m_controller.translateTag(MENU_OPTIONS), m_controller.translateTag(MENU_ABOUT), m_controller.translateTag(MENU_EXIT)};
    m_pMenuWidget->setItems(items);

    ui->stackedWidget->addWidget(m_pMenuWidget);
    ui->stackedWidget->setCurrentWidget(m_pMenuWidget);

    connect(m_pMenuWidget, &MenuWidget::activated, this, [this](int , const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            ui->stackedWidget->removeWidget(m_pMenuWidget);
            delete m_pMenuWidget;
            m_pMenuWidget = NULL;
            m_controller.sayTranslationTag(MENU_MSG_EXITED);
            m_controller.setMenuOpen(false);
        } else if (item == m_controller.translateTag(MENU_BLUETOOTH)) {
            bluetoothMenu();
        } else if (item == m_controller.translateTag(MENU_LANGUAGE)) {
            langugesMenu();
        } else if (item == m_controller.translateTag(MENU_OPTIONS)) {
            optionsMenu();
        } else if (item == m_controller.translateTag(MENU_ABOUT)) {
            aboutMenu();
        }
    });
}

void MainWindow::bluetoothMenu()
{
    m_controller.pause();
    if(!IsBtRunning()) {
        m_controller.sayTranslationTag(MENU_MSG_BLUETOOTH_ERROR);
        return;
    }
    auto *menuWidget = new MenuWidget("Bluetooth menu", &m_controller, ui->stackedWidget);
    QStringList items{m_controller.translateTag(MENU_SCAN_DEV), m_controller.translateTag(MENU_PAIRED_DEV), m_controller.translateTag(MENU_EXIT)};
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int , const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        } else if (item == m_controller.translateTag(MENU_SCAN_DEV)) {
            bluetoothScanMenu();
        } else if (item == m_controller.translateTag(MENU_PAIRED_DEV)) {
            bluetoothPairedMenu();
        }
    });
}

void MainWindow::langugesMenu()
{
    m_controller.pause();

    auto *menuWidget = new MenuWidget("Languages menu", &m_controller, ui->stackedWidget);
    QStringList items;
    m_controller.getListOfLanguges(items);
    items.push_back(m_controller.translateTag(MENU_EXIT));
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int i, const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            m_controller.saveVoiceSettings();
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        } else {
            m_controller.toggleVoiceEnabled(i);
            QStringList items;
            m_controller.getListOfLanguges(items);
            menuWidget->setItem(i, items[i]);
        }
    });
}

void MainWindow::optionsMenu() {
    m_controller.pause();

    auto *menuWidget = new MenuWidget("About menu", &m_controller, ui->stackedWidget);
    QStringList items;
    m_controller.getListOfOptions(items);
    items.push_back(m_controller.translateTag(MENU_EXIT));
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int i, const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            m_controller.writeSettings();
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        } else {
            m_controller.toggleOption(i);
            QStringList items;
            m_controller.getListOfOptions(items);
            menuWidget->setItem(i, items[i]);
        }
    });
}

void MainWindow::aboutMenu() {
    m_controller.pause();

    auto *menuWidget = new MenuWidget("About menu", &m_controller, ui->stackedWidget);
    QStringList items;
    m_controller.getListOfAboutItems(items);
    items.push_back(m_controller.translateTag(MENU_EXIT));
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int i, const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            m_controller.saveVoiceSettings();
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        }
    });
}

void MainWindow::bluetoothScanMenu()
{
    m_controller.pause();

    auto *menuWidget = new MenuWidget("Bluetooth scan", &m_controller, ui->stackedWidget);
    QStringList items{m_controller.translateTag(MENU_EXIT)};
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int index, const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            m_scanningTimer.stop();
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        } else {
            m_scanningTimer.stop();
            m_controller.sayTranslationTag(MENU_MSG_PAIRING);
            m_bluetoothHandler.startPairing(index);
        }
    });

    m_controller.sayTranslationTag(MENU_MSG_SCANNING);
    m_bluetoothHandler.startDeviceDiscovery();
    m_scanningTimer.start(BLUETOOTH_SCANNING_ANNOUNCEMENT_TIME);
}

void MainWindow::bluetoothPairedMenu()
{
    m_bluetoothHandler.prepareConnectedDevices();

    auto *menuWidget = new MenuWidget("Paired devices menu", &m_controller, ui->stackedWidget);
    QStringList items = m_bluetoothHandler.pairedDeviceNames();
    items.append(m_controller.translateTag(MENU_EXIT));
    menuWidget->setItems(items);

    ui->stackedWidget->addWidget(menuWidget);
    ui->stackedWidget->setCurrentWidget(menuWidget);

    connect(menuWidget, &MenuWidget::activated, this, [this, menuWidget](int index, const QString &item){
        if (item == m_controller.translateTag(MENU_EXIT)) {
            m_scanningTimer.stop();
            ui->stackedWidget->removeWidget(menuWidget);
            delete menuWidget;
        } else {
            m_controller.switchToBuiltInSink();
            m_bluetoothHandler.unpair(index);
        }
    });
}

void MainWindow::onDeviceScanningError(QBluetoothDeviceDiscoveryAgent::Error error, const QString &errorStr)
{
    qDebug() << "Bluetooth scanning error:" << error << errorStr;
    m_scanningTimer.stop();
    QString message = m_controller.translateTag(MENU_MSG_BLUETOOTH_ERROR) + " " + errorStr;
    ui->statusbar->showMessage(message, STATUS_MESSAGE_TIMEOUT);
    m_controller.sayText(message);
}

void MainWindow::onDeviceScanningFinished()
{
    qDebug() << "Bluetooth scanning finished";
    m_scanningTimer.stop();

    QString message = m_controller.translateTag(MSG_BLUETOOTH_SCAN_FINISHED);
    ui->statusbar->showMessage(message, STATUS_MESSAGE_TIMEOUT);
    m_controller.sayText(message);

    addDiscoveredDevicesToMenu();
}

void MainWindow::onScanningTimer()
{
    QString message = m_controller.translateTag(MENU_MSG_SCANNING);
    ui->statusbar->showMessage(message, STATUS_MESSAGE_TIMEOUT);
    m_controller.sayText(message);

    addDiscoveredDevicesToMenu();
}

void MainWindow::onBluetoothConnected(const QString &name)
{
    QString message = m_controller.translateTag(MENU_MSG_PAIRED_DEV) + " " + name;
    ui->statusbar->showMessage(message, STATUS_MESSAGE_TIMEOUT);
    m_controller.sayText(message);

    MenuWidget *menuWidget = dynamic_cast<MenuWidget *>(ui->stackedWidget->currentWidget());
    if (menuWidget) {
        m_controller.waitForSayTextFinished();
        menuWidget->exit();
    }
}

void MainWindow::onBluetoothConnectionError(const QString &name)
{
    QString message = m_controller.translateTag(MENU_MSG_PAIR_ERROR) + " " + name;
    ui->statusbar->showMessage(message, STATUS_MESSAGE_TIMEOUT);
    m_controller.sayText(message);
}

void MainWindow::onBluetoothUnpaired(int /*index*/, const QString &/*name*/)
{
    m_controller.resetAudio();
    QString message = m_controller.translateTag(MENU_MSG_UNPAIR);
    MenuWidget *menuWidget = dynamic_cast<MenuWidget *>(ui->stackedWidget->currentWidget());
    m_controller.sayText(message);
    if (menuWidget) {
        m_controller.waitForSayTextFinished();
        menuWidget->exit();
    }
}

void MainWindow::onPauseResumeButton() {
    if(!m_controller.tryProcessScannedImages())
        m_controller.pauseResume();
}
