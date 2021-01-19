/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "maincontroller.h"
#include "ocrhandler.h"
#include "textpage.h"
#include "hwhandler.h"
#include "cerence/cerencetts.h"
#include <opencv2/imgcodecs.hpp>

#include <QDebug>
#include <QTextToSpeech>
#include <QPluginLoader>

using namespace cv;

#define SHUTER_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/camera-shutter-click-01.wav"
#define BEEP_SOUND_WAVE_FILE "/opt/zyrlo/Distrib/Data/beep-08b.wav"

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    m_ttsEngine = new CerenceTTS(this);
    connect(m_ttsEngine, &CerenceTTS::wordNotify, this, &MainController::wordNotify);

    m_hwhandler = new HWHandler(this);
    connect(m_hwhandler, &HWHandler::imageReceived, this, [this](const Mat &image){
        qDebug() << "imageReceived 0\n";
        ocr().startProcess(image);
        qDebug() << "imageReceived 2\n";
        if(m_shutterSound)
            m_shutterSound->play();

    }, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::buttonReceived, this, [](Button button){
        qDebug() << "received" << (int)button;
    }, Qt::QueuedConnection);

    connect(m_hwhandler, &HWHandler::previewImgUpdate, this, &MainController::previewImgUpdate, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::readerReady, this, &MainController::readerReady, Qt::QueuedConnection);
    connect(m_hwhandler, &HWHandler::targetNotFound, this, &MainController::targetNotFound, Qt::QueuedConnection);

    m_translator.Init();
    m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
    m_hwhandler->start();
}

void MainController::start(const QString &filename)
{
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ocr().startProcess(image);
//    m_ttsEngine->say("Hello world! My name is Ava");
    m_ttsEngine->say(
        "Audio support in Qt is actually quite rudimentary. The goal is to "
        "have media playback at the lowest possible implementation and "
        "maintenance cost. The situation is especially bad on windows, where I "
        "think the ancient MME API is still employed for audio playback.  As a "
        "result, the Qt audio API is very far from realtime, making it "
        "particularly ill-suited for such applications. I recommend using "
        "portaudio or rtaudio, which you can still wrap in Qt style IO devices "
        "if you will. This will give you access to better performing platform "
        "audio APIs and much better playback performance at very low "
        "latency. ");
}

OcrHandler &MainController::ocr()
{
    return OcrHandler::instance();
}

void MainController::previewImgUpdate(const Mat & prevImg) {
    emit previewUpdated(prevImg);
}

void MainController::snapImage() {
    m_hwhandler->snapImage();
}

void MainController::flashLed() {
    m_hwhandler->flashLed(1000);
}

void MainController::setLed(bool bOn) {
     m_hwhandler->setLed(bOn);
}

static int getCurrentPulseSinkIndex() {
    FILE *fp;
    char path[1035];
    int indx = -1;
    fp = popen("pacmd info | grep \"* index:\"", "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        qDebug() << path;
        if(strstr(path, "index: 1")) {
            indx = 1;
            break;
        }
        if(strstr(path, "index: 0")) {
            indx = 0;
            break;
        }
     }

    /* close */
    pclose(fp);
    return indx;
}

bool MainController::toggleAudioSink() {
    int indx = getCurrentPulseSinkIndex();
    if(indx < 0)
        return false;
    bool bret = true;
    FILE *fp;
    char path[1035], cmd[256];
    sprintf(cmd, "pacmd set-default-sink %d", 1 - indx);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        qDebug() << "Failed to run command\n";
        bret = false;
    }
    else {
        /* Read the output a line at a time - output it. */
        while (fgets(path, sizeof(path), fp) != NULL) {
            bret = false;
            qDebug() << path;
        }
    }
    /* close */
    pclose(fp);
    if(m_shutterSound) {
        delete m_shutterSound;
        m_shutterSound = new QSound(SHUTER_SOUND_WAVE_FILE, this);
     }
    if(m_beepSound) {
        delete m_beepSound;
        m_beepSound = new QSound(BEEP_SOUND_WAVE_FILE, this);
     }
    if(bret && m_ttsEngine) {
        m_ttsEngine->resetAudio();
    }
    return bret;
}

void MainController::readerReady() {
    if(m_ttsEngine)
        m_ttsEngine->say(m_translator.GetString("PLACE_DOC").c_str());
}

void MainController::targetNotFound() {
    if(m_ttsEngine)
        m_ttsEngine->say(m_translator.GetString("CLEAR_SURF").c_str());
}
