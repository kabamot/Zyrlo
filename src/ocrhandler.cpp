/****************************************************************************
 *
 * @author Dilshod Mukhtarov <dilshodm(at)gmail.com>
 * Nov 2020
 *
 ****************************************************************************/

#include "ocrhandler.h"

#include <ZyrloOcr.h>

#ifdef RPI
constexpr auto DATA_DIR = "/home/pi/zyrlo/Distrib";
#else
constexpr auto DATA_DIR = "Distrib";
#endif

OcrHandler::OcrHandler(QObject *parent) : QObject(parent)
{

}
