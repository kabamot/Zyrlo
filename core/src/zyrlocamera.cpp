/*
 * ZyrloCamera.cpp
 *
 *  Created on: Dec 24, 2020
 *      Author: lev
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <QDebug>

#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include "zyrlocamera.h"

using namespace std;
using namespace cv;

#define MAX_PATH	256

#define ShowError PrintMessage
#define LOGI PrintMessage

#define DEVICE "/dev/video0"
#define CLEAR(x) memset(&(x), 0, sizeof((x)))

#define PREVIEW_WIDTH   1920
#define PREVIEW_HEIGHT  1080
#define FULLRES_WIDTH   3280
#define FULLRES_HEIGHT  2464

#define TARGET_IMG_PATH "/opt/zyrlo/Distrib/Data/Target.bmp"

unsigned long long GetTickCount(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
        return 0;
    return (unsigned long long)now.tv_sec * 1000 + (unsigned long long)now.tv_nsec / 1000000;
}

float CalcBrightness(const Mat & img, int topPercent, int bottomPercent) {
    int pHist[256], i, j, k;
    const Rect frame(0, 0, img.cols, img.rows);
    build_histogram(img, frame, pHist);
    int nSum = 0;
    for(i = 0; i != 256; ++i)
        nSum += pHist[i];
    int nTop = int(float(topPercent * nSum) * 0.01f + 0.5f);
    int nBot = int(float(bottomPercent * nSum) * 0.01f + 0.5f);

    for(i = 255, nSum = 0; i >= 0 && nSum < nTop; --i)
        nSum += pHist[i];
    for(j = 0, nSum = 0; j < 256 && nSum < nBot; ++j)
        nSum += pHist[j];

    int avg = 0;
    j = max(0, j - 1);
    i = min(255, i + 1);
    for(k = j, nSum = 0; k <= i; ++k) {
        nSum += pHist[k];
        avg += k * pHist[k];
    }
    if(nSum < 1)
        return -1;
    return float(avg) / float(nSum);
}

int BaseCommAdapter();

ZyrloCamera::ZyrloCamera()
    : m_targetPos(0, 0) {
    m_targetImg = imread(TARGET_IMG_PATH, IMREAD_GRAYSCALE);
    m_targetCorr.create(PREVIEW_HEIGHT / 8, PREVIEW_WIDTH / 8, CV_32F);
    m_vFullResRawImgs.resize(m_nFullResImgNum);
    m_vFullResGreyImgs.resize(m_nFullResImgNum);
    m_vFullResImgs.resize(m_nFullResImgNum);
    for(auto i = m_vFullResRawImgs.begin(), j = m_vFullResGreyImgs.begin(), k = m_vFullResImgs.begin(); i != m_vFullResRawImgs.end(); ++i) {
        i->create(FULLRES_HEIGHT, FULLRES_WIDTH, CV_8U);
        j->create(FULLRES_HEIGHT, FULLRES_WIDTH, CV_8U);
        k->create(FULLRES_HEIGHT, FULLRES_WIDTH, CV_8UC3);
    }
}

ZyrloCamera::~ZyrloCamera() {
    if(m_fd > 0) {
        stop_capturing();
        LOGI("Closing device\n");
        close(m_fd);
    }
}

void ZyrloCamera::BayerToDownsampledRG2BGR(const Mat & bayer, Mat & BGR, int step) const {
    const UCHAR *pS, *pSend;
    UCHAR *pD;
    step *= 2;
    int nW = bayer.cols / step, nH = bayer.rows / step, widthStep = bayer.step[0];
    if(BGR.empty())
        BGR.create(nH, nW, CV_8UC3);
    for(int i = 0, j = 0; i < bayer.rows; i += step, ++j) {
        for(pS = bayer.ptr(i), pD = BGR.ptr(j), pSend = pS +  bayer.cols; pS < pSend; pS += step, pD += 3) {
            pD[2] = min((pS[0] * m_cr) >> 8, 255); // red
            pD[1] = pS[1]; // green
            pD[0] = min((pS[widthStep + 1] * m_cb) >> 8, 255); // blue
        }
    }
}

void ZyrloCamera::BayerToDownsampledRG2Grey(const Mat & bayer, Mat & grey, int step) const {
    const UCHAR *pS, *pSend;
    UCHAR *pD;
    step *= 2;
    int nW = bayer.cols / step, nH = bayer.rows / step;
    if(grey.empty())
        grey.create(nH, nW, CV_8U);
    for(int i = 0, j = 0; i < bayer.rows; i += step, ++j)
        for(pS = bayer.ptr(i), pD = grey.ptr(j), pSend = pS +  bayer.cols; pS < pSend; pS += step, ++pD)
            *pD = pS[1]; // green
}

void ZyrloCamera::BayerToDownsampledRG2Grey(const Mat & bayer) {
    BayerToDownsampledRG2Grey(bayer, m_previewImg, 1);
}

void ZyrloCamera::WB(const Mat & bayer) {
    int step = 4, i;
    const UCHAR *pS, *pSend;
    float rsum = 0.0f, bsum = 0.0f, gsum = 0.0f;
    int widthStep = bayer.step[0];
    step *= 2;
    for(i = 0; i < bayer.rows; i += step) {
        for(pS = bayer.ptr(i), pSend = pS +  bayer.cols; pS < pSend; pS += step) {
            rsum += pS[0]; // red
            gsum += pS[1]; // green
            bsum += pS[widthStep + 1]; // blue
        }
    }
    if(rsum < 1.0f || bsum < 1.0f)
        return;
    m_cr = gsum * 256.0f / rsum;
    m_cb = gsum * 256.0f / bsum;
}

Point2f ZyrloCamera::GetMotion(const Mat & grey) {
    return m_md.GetMotion(grey);
}

void ZyrloCamera::Clear() {
    m_previewImgPyr1.release();
    m_previewImgPyr2.release();
    m_previewImg.release();
    m_ocrImg.release();
}

const Mat & ZyrloCamera::GetPreviewImg() const {
    return m_previewImgPyr2;
}

const Mat & ZyrloCamera::GetFullResRawImg(int indx) const {
    return m_vFullResRawImgs[indx];
}

void ZyrloCamera::PrintMessage(const char *format, ...) {
    char buf[1024];

    va_list arglist;
    va_start( arglist, format );
    vsprintf(buf,format,arglist);
    va_end( arglist );

    if(m_PrintMessLocation == 0)
        printf("%s\n", buf);
    else {
        if(m_PrintMessageFile != NULL) {
            fwrite(buf, strlen(buf), 1, m_PrintMessageFile);
            fflush(m_PrintMessageFile);
        }
    }
}

ZyrloCamera::Zcevent ZyrloCamera::FollowGestures(Point2f motion) {
    //qDebug() << "FollowGestures:" << motion.x << motion.y << Qt::endl;
    Zcevent ret = eShowPreviewImge;
    if(m_nWait > 0) --m_nWait;
    if(motion.x > 2) {
        ++m_nMotionPX;
        //qDebug() << "eGestBackSentence:" << motion.x << motion.y << Qt::endl;
        m_nMotionNX = 0;
        if(m_nMotionPX == 3 && m_nWait == 0) {
            m_nWait = 20;
            ret = eGestPauseResume;
        }
    }
    else {
        m_nMotionPX = 0;
    }
    if(motion.x < -2) {
        //qDebug() << "eGestBackSentence:" << motion.x << motion.y << Qt::endl;
        m_nMotionPX = 0;
        ++m_nMotionNX;
        if(m_nMotionNX == 3 && m_nWait == 0) {
            //qDebug() << "eGestBackSentence:" << motion.x << motion.y << Qt::endl;
            m_nWait = 20;
            ret = eGestBackSentence;
        }
    }
    else {
        m_nMotionNX = 0;
    }
    return ret;
}

ZyrloCamera::Zcevent ZyrloCamera::AcquireFrameStep() {
    if(!m_bArmOpen)
        return eCameraArmClosed;
    if(AcquireImage() == 1) // Full res snapshot
        return eStartOcr;
    if(m_bIgnoreInputs)
        return eShowPreviewImge;
    Zcevent zcev = eShowPreviewImge;
    if(m_nDelayCount == 0) {
        m_nDelayCount = exp_setting_delay;
        m_fLastBrightness = CalcBrightness(m_previewImgPyr2, 5, 90);
        float newExp = m_fPreviewExposure;
        if(m_fLastBrightness > m_brightUpperLimit)
            newExp = max(m_fPreviewExposure * 0.9f, float(m_nMinExpValue));
        if(m_fLastBrightness < m_brightLowerLimit)
            newExp = min(m_fPreviewExposure * 1.1f, float(m_nMaxExpValue)  * calcGain(m_nMaxGainValue));
        if(m_bAutoExposure && fabs(newExp - m_fPreviewExposure) > 1.0e-6)
            m_fPreviewExposure = setEffectiveExposure(int(newExp + 0.5f));
    }
    --m_nDelayCount;
    switch(m_eState) {
    case eCalibration:
        if(--m_nDelayCount == 0) {
            m_nDelayCount = exp_setting_delay;
            if(adjustExposure(m_previewImgPyr2) == 0) {
                m_wb = true;
                m_eState = eLookingForTarget;
            }
        }
        break;
    case eLocalLightFreqTest:
        LocalLightFreqTest(m_previewImgPyr2);
        break;
    case eLookingForTarget:
        if(LookForTarget(m_previewImgPyr2, m_targetImg, -1) > m_fLookForTargetHighThreshold) {
            m_eState = eReadyOnTarget;
            zcev = eReaderReady;
            qDebug() << "Target found\n";
        }
        else if(++m_nLookingForTargetCount == m_nMaxLookingForTargetCount) {
            zcev = eTargetNotFound;
        }
        break;
    case eReadyOnTarget:
        if(LookForTarget(m_previewImgPyr2, m_targetImg, 10) < m_fLookForTargetLowThreshold) {
            m_eState = eLookingForStableImage;
            qDebug() << "Looking For Stable Image\n";
        }
        break;
    case eLookingForStableImage:
        if(LookForTarget(m_previewImgPyr2, m_targetImg, 10) > m_fLookForTargetHighThreshold) {
            m_eState = eReadyOnTarget;
            zcev = eReaderReady;
            qDebug() << "Target found\n";
            break;
        }
        DetectImageChange(m_previewImgPyr2);
        if(m_nNoChange == MOTION_DETECTOR_STEADY_STATE_COUNT_PREVIEW) {
            m_bPictReq = true;
            m_eState = eLookingForGestures;
        }
        break;
    case eLookingForGestures:
        if(LookForTarget(m_previewImgPyr2, m_targetImg, 10) > m_fLookForTargetHighThreshold) {
            m_eState = eReadyOnTarget;
            zcev = eReaderReady;
            qDebug() << "Target found\n";
            break;
        }
        if(m_bEnableGestureUI) {
            Point2f motion = GetMotion(m_previewImgPyr2);
            zcev = FollowGestures(motion);
        }
        break;
    case eFullResPreview:
        break;
    }
    return zcev;
}

void ZyrloCamera::init_mmap() {
    unsigned int nBuffers;
    struct v4l2_requestbuffers req;
    struct v4l2_exportbuffer expbuf;

    CLEAR(req);
    req.count = 4;
    req.type = 1; //V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = 1; //V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0) {
        LOGI("ioctl VIDIOC_REQBUFS error %d %s\n", errno, strerror(errno));
        if(errno == EINVAL)
            LOGI("mmap not supported\n");
    } else {
        LOGI("MMAP supported. Count=%d\n", req.count);
    }

    m_nBuffers = req.count;
    m_buffers = (struct buffer *)calloc(req.count, sizeof(*m_buffers));
    if(m_buffers == NULL)
        LOGI("Error allocation buffers struct\n");

    for(nBuffers = 0; nBuffers < req.count; nBuffers++) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = nBuffers;
        if(ioctl(m_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            LOGI("ioctl error VIDIOC_QUERYBUF %d\n", errno);
        } else {
            m_buffers[nBuffers].length = buf.length;
            m_buffers[nBuffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                                             MAP_SHARED, m_fd, buf.m.offset );
            LOGI("VIDIOC_QUERYBUF offset %x length %d Start %ld\n", buf.m.offset, buf.length, (long unsigned int)m_buffers[nBuffers].start);
        }
    }

    for(nBuffers = 0; nBuffers < req.count; nBuffers++) {

        struct v4l2_buffer buf;

        CLEAR(buf);
        CLEAR(expbuf);
        expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        expbuf.index = nBuffers;
        expbuf.flags = 0x80002;
        expbuf.plane = 0;
        if(ioctl(m_fd, VIDIOC_EXPBUF, &expbuf) < 0) {
            LOGI("ioctl error VIDIOC_EXPBUF %d %s\n", errno, strerror(errno));
        } else {
            LOGI("VIDIOC_EXPBUF fd=%x\n", expbuf.fd);
            m_buffers[nBuffers].DMABUF_Id = expbuf.fd;
        }

        buf.index = nBuffers;
        buf.memory = 1; //V4L2_MEMORY_DMABUF;
        buf.type = 1;
        if(ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) {
            LOGI("Queue buffer error %d %s\n", errno, strerror(errno));
        } else
            LOGI("Queue buffer success (init_mmap)\n");
    }
}

void ZyrloCamera::uninit_mmap() {
    unsigned int nBuffers;

    for(nBuffers = 0; nBuffers < m_nBuffers; nBuffers++) {
        if( munmap(m_buffers[nBuffers].start, m_buffers[nBuffers].length) < 0)
            LOGI("memory unmap error\n");
        else
            printf("Buffer %d unmapped\n", nBuffers);
        close(m_buffers[nBuffers].DMABUF_Id );
    }

    struct v4l2_requestbuffers req;

    CLEAR(req);
    req.count = 0;
    req.type = 1; //V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = 1; //V4L2_MEMORY_MMAP;
    if(ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0) {
        LOGI("ioctl VIDIOC_REQBUFS error\n");
        if(errno == EINVAL)
            LOGI("mmap not supported\n");
    } else {
        LOGI("MMAP supported. Count=%d\n", req.count);
    }
}

int ZyrloCamera::start_capturing() {
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
        LOGI("ioctl error VIDIOC_STREAMON in start_capturing %d %s\n", errno, strerror(errno));
        return -1;
    }
    LOGI("ioctl VIDIOC_STREAMON Success\n");
    return 0;
}

int ZyrloCamera::stop_capturing() {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0) {
        LOGI("ioctl error VIDIOC_STREAMON in stop_capturing %d\n", errno);
        return -1;
    }
    LOGI("Stopped capturing OK\n");
    return 0;
}

int ZyrloCamera::SwitchMode(bool bModePreview) {
    stop_capturing();
    uninit_mmap();
    return SetMode(bModePreview);
}

int ZyrloCamera::SetMode(bool bModePreview)
{
    struct v4l2_format fmt;
    m_bModePreview = bModePreview;
    if(m_bModePreview) {
        m_nCurrImgWidth = PREVIEW_WIDTH;
        m_nCurrImgHeight = PREVIEW_HEIGHT;
    } else {
        m_nCurrImgWidth = FULLRES_WIDTH;
        m_nCurrImgHeight = FULLRES_HEIGHT;
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_nCurrImgWidth;
    fmt.fmt.pix.height = m_nCurrImgHeight;
    fmt.fmt.pix.pixelformat = 0x42474752;  //8 bit bayer
    fmt.fmt.pix.field = 1;
    fmt.fmt.pix.bytesperline = m_nCurrImgWidth;

    printf("Calling VIDIOC_S_FMT\n");

    if(ioctl(m_fd, VIDIOC_S_FMT, &fmt) == -1) {
        printf("ioctl VIDIOC_S_FMT error in SetPreview %d %s\n", errno, strerror(errno));
    } else {
        printf("Set format success in SetPreview w %d h %d fmt %x\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix_mp.pixelformat);
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(m_fd, VIDIOC_G_FMT, &fmt) == -1) {
        printf("ioctl VIDIOC_G_FMT error\n");
    } else {
        printf("VIDIOC_G_FMT success: W=%d h=%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
        printf("G_FMT Format: %x iField: %d Size: %d BPL: %d\n", fmt.fmt.pix.pixelformat, fmt.fmt.pix.field, fmt.fmt.pix.sizeimage, fmt.fmt.pix.bytesperline );
    }

    m_nCurrImgWidth = fmt.fmt.pix.width;
    m_nCurrImgHeight = fmt.fmt.pix.height;
    m_nCurrBytesPerLine = fmt.fmt.pix.bytesperline;

    init_mmap();
    start_capturing();

    return 0;
}


int ZyrloCamera::initCamera()
{
    wiringPiSetup();
    pinMode(21, OUTPUT);
    pinMode(25, OUTPUT);
    if( (m_fd = open(DEVICE, O_RDWR, O_NONBLOCK, 0)) < 0) {
        printf("Device open error: %s\n", DEVICE);
        return -1;
    }

    //printf("Device opened: %s\n", DEVICE);

    struct v4l2_capability cap;

    if(ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        printf("Query capabilities error\n");
        return -1;
    }

    //printf("Capabilities %x   %x\n", cap.capabilities, cap.device_caps);

    SetMode(true);
    //setGain(m_nGain);
    setEffectiveExposure(int(m_fPreviewExposure + 0.5f));
    qDebug() << "Init Camera Done\n";
    return 0;
}

int ZyrloCamera::acquireBuffer(int nBufferInd) {
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = nBufferInd;
    buf.flags = 0;

    if(ioctl(m_fd, VIDIOC_DQBUF, &buf) < 0) {
        LOGI("error VIDIOC_DQBUF %d: %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ZyrloCamera::releaseBuffer(int nBufferInd) {
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = nBufferInd;
    buf.flags = 0;

    if(ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) {
        //	LOGI("error VIDIOC_QBUF %d: %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ZyrloCamera::adjustExposure(const Mat & img) {
    //qDebug() << "adjustExposure first "<< m_nExposure << " " << m_nAvgTargetBrightness << " " << m_nMaxExp << " " << m_nMinExp << Qt::endl;
    if(m_nMaxExp - m_nMinExp <= 10) {

        return 0;
    }
    int pHist[256], i;
    const Rect frame(0, 0, img.cols, img.rows);
    build_histogram(img, frame, pHist);
    int nSum = 0;
    for(i = 0; i != 256; ++i)
        nSum += pHist[i];
    int nPercnt = nSum / 2, avg = 0;
    for(i = 255, nSum = 0; i >= 0 && nSum < nPercnt; --i) {
        nSum += pHist[i];
        avg += i * pHist[i];
    }
    if(nSum < 1)
        return -1;
    avg = avg / nSum;
    qDebug() << "adjustExposure Exp = " << m_fPreviewExposure << " avg = " <<  avg  << Qt::endl;
    if(avg < m_nAvgTargetBrightness) {
        m_nMinExp = int(m_fPreviewExposure + 0.5f);
    }
    else if(avg > m_nAvgTargetBrightness)
        m_nMaxExp = int(m_fPreviewExposure + 0.5f);
    else
        return 0;
    m_fPreviewExposure = float(m_nMinExp +  m_nMaxExp) * 0.5f;
    setExposure(int(m_fPreviewExposure + 0.5f));
    return 1;
}

int ZyrloCamera::AcquireImage() {
    if(m_bPictReq) {
        m_bPictReq = false;
        float fExpNoFlash =  m_fPreviewExposure * 200.0f / m_fLastBrightness, fExpFlash = 2666.7f;
        //qDebug() << "Exposure =" <<  fExpNoFlash * fExpFlash / (fExpNoFlash + fExpFlash) << "Brightness = " << m_fLastBrightness << Qt::endl;
        if(m_bUseFlash) {
            flashLed(1000);
            AcquireFullResImage(fExpNoFlash * fExpFlash / (fExpNoFlash + fExpFlash), 0);
        }
        else
            AcquireFullResImage(fExpNoFlash, 0);
        //float fBrightness = CalcBrightness(m_vFullResRawImgs[0], 5, 90);
        //qDebug() << "Full Res bright =" << fBrightness;
        //AcquireFullResImage(100, 2500, 1);
        SwitchMode(true);
        setEffectiveExposure(m_fPreviewExposure);
        digitalWrite(21, 0);
        return 1;
    }

    if( m_nSwitchFullResPreview ) {

        if(m_nSwitchFullResPreview == 1)
        {
            m_eState = eFullResPreview;
            SwitchMode(false);
        }
        else
        {
            m_eState = eLookingForTarget;
            SwitchMode(true);
        }
        m_nSwitchFullResPreview = 0;
        Clear();
    }

    acquireBuffer(m_nCamBufInd);

    Mat img(m_nCurrImgHeight, m_nCurrBytesPerLine , CV_8U, m_buffers[m_nCamBufInd].start);

    //BayerToDownsampledRG2BGR(img, imgSmall, 4);
    BayerToDownsampledRG2Grey(img);
    pyrDown(m_previewImg, m_previewImgPyr1);
    pyrDown(m_previewImgPyr1, m_previewImgPyr2);
    if(m_wb) {
        m_wb = false;
        WB(img);
    }
    releaseBuffer(m_nCamBufInd);
    m_nCamBufInd = (m_nCamBufInd + 1) % 4;

    //    if(++m_nCnt == 100) {
    //        int fps = m_nCnt * 1000 /(GetTickCount() - m_timeStamp);
    //        qDebug() << "FPS = " << fps;
    //        m_nCnt = 0;
    //        m_timeStamp = GetTickCount();
    //    }
    return 0;
}

void WriteLog(string s) {
    FILE *fp = fopen("/home/pi/log.txt", "a");
    if(!fp)
        fp = fopen("/home/pi/log.txt", "w");
    if(!fp)
        return;
    fprintf(fp, (s + '\n').c_str());
    fclose(fp);
}

int ZyrloCamera::CalcLocalAjustedExposure(int nValue) const {
    int nSteps = int(float(nValue) / m_fExposureStep + 0.5f);
    int nMaxSteps = int(m_nMaxExpValue / m_fExposureStep);
    if(nSteps < 1)
        nSteps = 1;
    if(nSteps > nMaxSteps)
        nSteps = nMaxSteps;
    return int(nSteps * m_fExposureStep + 0.5f);
}

float ZyrloCamera::setEffectiveExposure(float fExposure) {
    int nAdjExp = CalcLocalAjustedExposure(int(fExposure + 0.5f));
    float fGain = fExposure / float(nAdjExp);
    int nGain = calcGainValue(fGain);
    setGain(nGain);
    setExposure(nAdjExp);
    return float(nAdjExp) * calcGain(nGain);
}

int ZyrloCamera::setExposure(int nValue) {
    //4 - 1759
    if(nValue > m_nMaxExpValue)
        nValue = m_nMaxExpValue;
    if(nValue < m_nMinExpValue)
        nValue = m_nMinExpValue;

    struct v4l2_control ctrl;
    ctrl.id = 0x00980911;
    ctrl.value = nValue;
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) < 0) {
        printf("VIDIOC_S_CTRL exposure error %d %s\n", errno, strerror(errno));
        return -1;
    }
    for(int exp = getExposure(), i = 0; exp != nValue; usleep(100000), exp = getExposure(), ++i)
        if(i == 100)
            return -1;
    //qDebug() << "setExposure" << nValue << Qt::endl;
    m_nCurrExp = nValue;
    return 0;
}

int ZyrloCamera::getExposure() const {
    struct v4l2_control ctrl;
    ctrl.id = 0x00980911;
    ctrl.value = 0;
    if(ioctl(m_fd, VIDIOC_G_CTRL, &ctrl) < 0) {
        printf("VIDIOC_S_CTRL exposure error %d %s\n", errno, strerror(errno));
        return -1;
    }
    char num[16];
    sprintf(num, "get_exp = %d", ctrl.value);
    WriteLog(num);
    return ctrl.value;
}

int ZyrloCamera::adjustColorGains() {
    struct v4l2_control ctrl;
    ctrl.id = 0x009e0904;	// red
    ctrl.value = 1023;
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL color gain error %d %s\n", errno, strerror(errno));
        return -1;
    }
    ctrl.id = 0x009e0904;
    ctrl.value = 10;	// green/r
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL color gain error %d %s\n", errno, strerror(errno));
        return -1;
    }
    ctrl.id = 0x009e0906;
    ctrl.value = 1023;	//blue
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL color gain error %d %s\n", errno, strerror(errno));
        return -1;
    }
    ctrl.id = 0x009e0904;
    ctrl.value = 10;	// green/b
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL color gain error %d %s\n", errno, strerror(errno));
        return -1;
    }

    printf("Color gains adjusted\n");
    return 0;
}

void ZyrloCamera::flashLed(int msecs) {
    pthread_t  h;
    static int time_out = msecs * 1000;
    pthread_create(&h, NULL, [](void* param){digitalWrite(21, 1); qDebug() << "Led ON\n"; usleep(*((int*)param));digitalWrite(21, 0); qDebug() << "Led OFF\n";return (void*)NULL;}, (void *)&time_out);
}

void ZyrloCamera::setLed(bool bOn) {
    digitalWrite(25, bOn ? 1 : 0);
}

int ZyrloCamera::snapImage() {
    m_bPictReq = true;
    return 0;
}


int ZyrloCamera::setGain(int nValue) {
    if(nValue > m_nMaxGainValue)
        nValue = m_nMaxGainValue;
    if(nValue < m_nMinGainValue)
        nValue = m_nMinGainValue;
    struct v4l2_control ctrl;
    ctrl.id = 0x009e0903;
    ctrl.value = nValue;
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL exposure error %d %s\n", errno, strerror(errno));
        return -1;
    }

    for(int gain = getGain(), i = 0; gain != nValue; usleep(100000), gain = getGain(), ++i)
        if(i == 100)
            return -1;

    return 0;
}

int ZyrloCamera::getGain() const {
    struct v4l2_control ctrl;
    ctrl.id = 0x009e0903;
    ctrl.value = 0;
    if(ioctl(m_fd, VIDIOC_G_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL exposure error %d %s\n", errno, strerror(errno));
        return -1;
    }
    char num[16];
    sprintf(num, "get_gain = %d", ctrl.value);
    WriteLog(num);
    return ctrl.value;
}

void ZyrloCamera::ReserExposureLimits() {
    m_nMinExp = m_nMaxExpValue;
    m_nMaxExp = m_nMaxExpValue;
}

int ZyrloCamera::AcquireFullResImage(float fEffectiveExposure, int indx) {
    //long int timeStamp = GetTickCount();

    //flashLed();
    setEffectiveExposure(fEffectiveExposure);
    SwitchMode(false);
    //char msg[512];
    //adjustColorGains();
    acquireBuffer(0);
    //    timeStamp = GetTickCount() - timeStamp;
    //    printf("PicTaken. Time: %ld\n", timeStamp);
    Mat(m_nCurrImgHeight, m_nCurrBytesPerLine , CV_8U, m_buffers[0].start).copyTo(m_vFullResRawImgs[indx]);
    //   char fname[256];
    //   sprintf(fname, "/home/pi/FullResRawImg_%d.bmp", indx);
    //   imwrite(fname, m_vFullResRawImgs[indx]);

    releaseBuffer(0);
    return 0;
}

const Mat & ZyrloCamera::GetImageForOcr() {
    qDebug() << "GetImageForOcr 0\n";
    for(auto i = m_vFullResRawImgs.begin(), j = m_vFullResImgs.begin(); i != m_vFullResRawImgs.end(); ++i, ++j) {
        adjustWb(*i);
        //cvtColor(*i, *j, COLOR_BayerBG2BGR_EA);
    }
    //imwrite("CVDEMOSAIC_0.bmp", m_vFullResImgs[0]);
    //imwrite("CVDEMOSAIC_1.bmp", m_vFullResImgs[1]);
    //   Ptr<MergeMertens> merge_mertens = createMergeMertens();
    //     DemosaicGrey(m0, images[0]);//dm0);
    //    DemosaicGrey(m1, images[1]);//dm1);
    //static Mat tmp, tmp1;
    //    merge_mertens->process(m_vFullResGreyImgs, tmp);
    //tmp.convertTo(m_ocrImg, CV_8U, 255.0);
    //imwrite("fusion1.png", fusion * 255);
    //cvtColor(m_vFullResImgs[0], m_ocrImg, COLOR_BGR2GRAY);
    qDebug() << "GetImageForOcr 1\n";
    return m_vFullResRawImgs[0];
}

float ZyrloCamera::LookForTarget(const Mat & fastPreviewImgBW, const Mat & targetBitmapBW, int nRadius) {
    //qDebug() << "LookForTarget " << targetBitmapBW.cols << targetBitmapBW.rows << "\n";
    if(m_bForceCorrelation)
        return 1.0f;
    int nX = 0, nY = 0;
    int W = fastPreviewImgBW.cols;
    int H = fastPreviewImgBW.rows;
    int w = targetBitmapBW.cols;
    int h = targetBitmapBW.rows;

    if (nRadius >= 0) {
        nX = max(0, m_targetPos.x - nRadius);
        nY = max(0, m_targetPos.y - nRadius);
        W = min(fastPreviewImgBW.cols - nX, targetBitmapBW.cols + (2 * nRadius));
        H = min(fastPreviewImgBW.rows - nY, targetBitmapBW.rows + (2 * nRadius));
    }
    Rect previewRect(nX, nY, W, H), targetCorrRect(w / 2, h / 2, W - w + 1, H - h + 1);
    Mat previewRoi = fastPreviewImgBW(previewRect);
    Mat targetCorrRoi = m_targetCorr(targetCorrRect);
    matchTemplate(previewRoi, targetBitmapBW, targetCorrRoi, CV_TM_CCOEFF_NORMED);

    double min_val, max_val;
    Point min_loc, max_loc;
    minMaxLoc(targetCorrRoi, &min_val, &max_val, &min_loc, &max_loc);

    if (nRadius < 0) {
        m_targetPos = max_loc;
    }
    //qDebug() << "LookForTarget " << targetBitmapBW.cols << targetBitmapBW.rows << "MaxVal = " << max_val << "\n";
    return (fabs(max_val - 1.0) < 1.0e-6) ? 0 : max_val;
}

static void wbRow(UCHAR *pRow, int nLength, int c) {
    for(UCHAR *pE = pRow + nLength; pRow < pE; pRow += 2)
        *pRow  = min((*pRow * c) >> 8, 255);
}


void  ZyrloCamera::adjustWb(Mat & bayer) {
    int W = (bayer.cols & ~1), H = (bayer.rows & ~1);
    for(int i = 0; i < H; i += 2) {
        wbRow(bayer.ptr(i), W, m_cr);
        wbRow(bayer.ptr(i + 1) + 1, W - 1, m_cb);
    }
}

static UINT GetNHighestVal(const UINT *pHist, int nN) {
    int nCount = 0;
    for(int i = 255; i >= 0; --i) {
        nCount += pHist[i];
        if(nCount >= nN) {
            return i;
        }
    }
    return 0;
}

static UINT GetNMinVal(const UINT *pHist, int nN) {
    int nCount = 0;
    for(int i = 0; i < 256; ++i) {
        nCount += pHist[i];
        if(nCount >= nN) {
            return i;
        }
    }
    return 0;
}

static float RegionDiff(const Mat &img1, const Mat & img2, int nStep, Rect region) {
    UINT pHist[256] = {0}, pDiffHist[256] = {0};

    const UCHAR *pC1, *pC2, *pC1e;
    int nPixDiff;
    region.x &= ~1;
    region.y &= ~1;
    region.width &= ~1;
    region.height &= ~1;

    Mat roi1 = img1(region), roi2 = img2(region);
    int nPixCount = 0;
    for(int i = 0; i < region.height; i += nStep) {
        pC1 = roi1.ptr(i);
        pC2 = roi2.ptr(i);
        pC1e = pC1 + region.width;
        for( ; pC1 < pC1e; pC1 += nStep, pC2 += nStep) {
            nPixDiff = abs(int(*pC1) - int(*pC2));
            ++pHist[*pC1];
            ++pDiffHist[nPixDiff];
            ++nPixCount;
        }
    }
    int nFraction = nPixCount / 20;
    int nBright = GetNHighestVal(pHist, nFraction);
    int nDiff = GetNHighestVal(pDiffHist, nFraction);
    if(nBright < 1) return 255.0f;
    return float(nDiff * nDiff) / float(nBright);
}

static float MaxRegionDiff(const Mat & img1, const Mat & img2, int nStep, int nRegionsX, int nRegionsY, int nMargins) {
    //WriteToLog("MaxRegionDiff %d", 1);
    Size s(img1.cols, img1.rows);
    int nRegionWidth = (s.width - nMargins * 2) / nRegionsX;
    int nRegionHeight = (s.height - nMargins * 2) / nRegionsY;

    float fMaxDiff = 0.0f, fDiff;
    for(int i = 0; i < nRegionsY; ++i) {
        for(int j = 0; j < nRegionsX; ++j) {
            Rect region(nMargins + j * nRegionWidth, nMargins + i * nRegionHeight, nRegionWidth, nRegionHeight);
            fDiff = RegionDiff(img1, img2, nStep, region);
            if(fDiff > fMaxDiff) {
                fMaxDiff = fDiff;
            }
        }
    }
    return fMaxDiff;
}

float ZyrloCamera::DetectImageChange(const Mat & img) {
    if(m_firstStableImg.empty()) {
        img.copyTo(m_firstStableImg);
        return 0.0f;
    }
    float fImgChange = MaxRegionDiff(m_firstStableImg, img, 2, 4, 3, 20);
    if(fImgChange < m_fImageChangeSensitivity) {
        ++m_nNoChange;
        m_nMotionDetected = 0;
        return fImgChange;
    }
    m_nNoChange = 0;
    ++m_nMotionDetected;
    img.copyTo(m_firstStableImg);
    return fImgChange;
}

bool ZyrloCamera::gesturesOn() const {
    return m_bEnableGestureUI;
}

void ZyrloCamera::setGesturesUi(bool bOn) {
    m_nWait = 20;
    m_nMotionPX = m_nMotionNX = 0;
    m_md.Clear();
    m_bEnableGestureUI = bOn;
}

void ZyrloCamera::setArmPosition(bool bOpen) {
    m_bArmOpen = bOpen;
    //    if(bOpen)
    //        SwitchMode(true);
    //    else
    //        stop_capturing();
}

void ZyrloCamera::LocalLightFreqTest(const Mat & img) {
    static int nCount = 0;
    static float fSum = 0.0f, fMax = 0;
    float fImgChange = DetectImageChange(img);
    fSum += fImgChange;
    if(fImgChange > fMax)
        fMax = fImgChange;
    if(++nCount == 100) {
        qDebug() << "Img Noise = " << fSum / float(nCount) << fMax;
        nCount = 0;
        fMax = fSum = 0.0f;
    }
}

void ZyrloCamera::setFullResPreview(bool bOn) {
    m_nSwitchFullResPreview = bOn ? 1 : -1;
}
