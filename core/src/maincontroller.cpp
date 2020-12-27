/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "maincontroller.h"
#include "ocrhandler.h"
#include "textpage.h"
#include "cerence/cerencetts.h"
#include <opencv2/imgcodecs.hpp>

#include <QDebug>
#include <QTextToSpeech>
#include <QPluginLoader>

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    m_ttsEngine = new CerenceTTS(this);
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
