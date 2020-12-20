/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "maincontroller.h"
#include "ocrhandler.h"
#include "textpage.h"
#include "cerence/cerencettsplugin.h"
#include <opencv2/imgcodecs.hpp>

#include <QDebug>
#include <QTextToSpeech>

Q_IMPORT_PLUGIN(CerenceTTSPlugin)

MainController::MainController()
{
    connect(&ocr(), &OcrHandler::lineAdded, this, [this](){
        emit textUpdated(ocr().textPage()->text());
        emit formattedTextUpdated(ocr().textPage()->formattedText());
    });

    QTextToSpeech tts;
    qDebug() << tts.availableEngines();
}

void MainController::start(const QString &filename)
{
    cv::Mat image = cv::imread(filename.toStdString(), cv::IMREAD_GRAYSCALE);
    ocr().startProcess(image);
}

OcrHandler &MainController::ocr()
{
    return OcrHandler::instance();
}
