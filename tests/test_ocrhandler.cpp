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

TEST_CASE("OcrHandler")
{
    OcrHandler &ocr = OcrHandler::instance();

    const cv::Mat bayer = cv::imread("/home/dilshodm/work/proj/upwork/leon/Zyrlo/tests/data/RawFull_000.bmp", cv::IMREAD_GRAYSCALE);

    DOCTEST_SUBCASE("idle status first test") {
        CHECK(ocr.isIdle());
    }

    DOCTEST_SUBCASE("cancelProcess") {
        ocr.startProcess(bayer);
        REQUIRE_FALSE(ocr.isIdle());

        QElapsedTimer timer; timer.start();
        ocr.cancelProcess();
        const auto elapsed = timer.elapsed();
        INFO(QString::number(elapsed).toStdString());
        INFO(ocr.getStatus().toStdString());
        CHECK(ocr.isIdle());
    }
}
