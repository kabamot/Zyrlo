/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Dec 2020
 *
 ****************************************************************************/

#include <doctest.h>
#include "ocrhandler.h"
#include <opencv2/opencv.hpp>
#include <QThread>
#include <QElapsedTimer>
#include <QCoreApplication>

TEST_CASE("OcrHandler")
{
    int argc = 1;
    const char *argv[] = {"hello"};
    QCoreApplication app(argc, const_cast<char **>(argv));

    OcrHandler &ocr = OcrHandler::instance();

    const cv::Mat bayer = cv::imread("/home/dilshodm/work/proj/upwork/leon/Zyrlo/tests/data/RawFull_000.jpg", cv::IMREAD_GRAYSCALE);

    DOCTEST_SUBCASE("idle status first test") {
        CHECK(ocr.isIdle());
    }

    DOCTEST_SUBCASE("abruptly stopProcess") {
        ocr.startProcess(bayer);
        REQUIRE_FALSE(ocr.isIdle());

        QElapsedTimer timer; timer.start();
        ocr.stopProcess();
        const auto elapsed = timer.elapsed();
        INFO(QString::number(elapsed).toStdString());
        CHECK(ocr.isIdle());
    }

    DOCTEST_SUBCASE("waiting for process finished") {
        int gotResults = 0;
        QObject::connect(&ocr, &OcrHandler::lineAdded, [&](){
            ++gotResults;
//            app.quit();
        });
        QObject::connect(&ocr, &OcrHandler::finished, &app, &QCoreApplication::quit);

        ocr.startProcess(bayer);
        REQUIRE_FALSE(ocr.isIdle());

        app.exec();
        ocr.stopProcess();

        CHECK(gotResults == 19);
    }
}
