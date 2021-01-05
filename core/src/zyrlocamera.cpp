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

unsigned long long GetTickCount(void)
{
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now))
    return 0;
  return (unsigned long long)now.tv_sec * 1000 + (unsigned long long)now.tv_nsec / 1000000;
}

int BaseCommAdapter();

ZyrloCamera::ZyrloCamera() {
    m_fullResRawImg.create(FULLRES_HEIGHT, FULLRES_WIDTH, CV_8U);
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
    m_previewImg.release();
}

const Mat & ZyrloCamera::GetPreviewImg() const {
    return m_previewImgPyr2;
}

const Mat & ZyrloCamera::GetFullResRawImg() const {
    return m_fullResRawImg;
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

int ZyrloCamera::AcquireFrameStep() {
    if(AcquireImage() == 1) // Full res snapshot
        return 1;
    switch(m_eState) {
    case eLookingForTargetForCalibration:
        if(adjustExposure(m_previewImgPyr2) == 0)
            m_eState = eCalibrationExposure;
        break;
    case eCalibrationExposure:
        break;
    case eReadyOnTarget:
        break;
    case eLookingForStableimage:
        break;
    case eLookingForGestures:
        if(m_bEnableGestureUI) {
            Point2f mtn = GetMotion(m_previewImgPyr2);
            if(mtn.dot(mtn) >= 10.0f)
                printf("Motion: x = %f\ty = %f\n", mtn.x, mtn.y);
        }
        break;
    }
    return 0;
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
        return 1;
    }

    printf("Device opened: %s\n", DEVICE);

    struct v4l2_capability cap;

    if(ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        printf("Query capabilities error\n");
        return -1;
    }

    printf("Capabilities %x   %x\n", cap.capabilities, cap.device_caps);

    SetMode(true);
    setExposure(m_nExposure);
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

    if(m_nMaxExp - m_nMinExp <= 10)
        return 0;
    int pHist[256], i;
    const Rect frame(0, 0, img.cols, img.rows);
    build_histogram(img, frame, pHist);
    int nSum = 0;
    for(i = 0; i != 256; ++i)
        nSum += pHist[i];
    int nPercnt = nSum / 10, avg;
    for(i = 0, nSum = 0; i != 256 && nSum < nPercnt; ++i) {
        nSum += pHist[i];
        avg += i * pHist[i];
    }
    if(nSum < 1)
        return -1;
    avg = avg / nSum;
    if(avg < m_nAvgTargetBrightness) {
        m_nMinExp = m_nExposure;
    }
    else if(avg > m_nAvgTargetBrightness)
        m_nMaxExp = m_nExposure;
    else
        return 0;
    m_nExposure = (m_nMinExp +  m_nMaxExp) / 2;
    setExposure(m_nExposure);
    return 1;
}

int ZyrloCamera::AcquireImage() {
    if(m_bPictReq) {
        m_bPictReq = false;
        AcquireFullResImage();
        return 1;
    }

    acquireBuffer(m_nCamBufInd);

    Mat img(m_nCurrImgHeight, m_nCurrBytesPerLine , CV_8U, m_buffers[m_nCamBufInd].start);

    //BayerToDownsampledRG2BGR(img, imgSmall, 4);
    BayerToDownsampledRG2Grey(img);
    pyrDown(m_previewImg, m_previewImgPyr1);
    pyrDown(m_previewImgPyr1, m_previewImgPyr2);
    if(m_wb) {
        m_wb = 0;
        WB(img);
    }
    releaseBuffer(m_nCamBufInd);
    m_nCamBufInd = (m_nCamBufInd + 1) % 4;

//    if(++m_nCnt == 10) {
//        int fps = m_nCnt * 1000 /(GetTickCount() - m_timeStamp);
//        printf("FPS = %d\n", fps);
//        m_nCnt = 0;
//        m_timeStamp = GetTickCount();
//    }
    return 0;
}

int ZyrloCamera::setExposure(int nValue) {
    struct v4l2_control ctrl;
    ctrl.id = 0x00980911;
    ctrl.value = nValue;
    if(ioctl(m_fd, VIDIOC_S_CTRL, &ctrl) <0) {
        printf("VIDIOC_S_CTRL exposure error %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
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

void ZyrloCamera::flashLed() {
    unsigned long h;
    pthread_create(&h, NULL, [](void* param){digitalWrite(21, 1); qDebug() << "Led ON\n"; usleep(500000);digitalWrite(21, 0); qDebug() << "Led OFF\n";return (void*)NULL;}, (void *)NULL);
}

void ZyrloCamera::setLed(bool bOn) {
    digitalWrite(25, bOn ? 1 : 0);
}

int ZyrloCamera::snapImage() {
    m_bPictReq = true;
    return 0;
}


int ZyrloCamera::AcquireFullResImage() {
    flashLed();
    long int timeStamp = GetTickCount();

    flashLed();
    SwitchMode(false);
    setExposure(300);
    adjustColorGains();
    acquireBuffer(0);
    digitalWrite(21, 0);
    timeStamp = GetTickCount() - timeStamp;
    printf("PicTaken. Time: %ld\n", timeStamp);
    setExposure(m_nExposure);
    Mat(m_nCurrImgHeight, m_nCurrBytesPerLine , CV_8U, m_buffers[0].start).copyTo(m_fullResRawImg);

    imwrite("FullResRawImg.bmp", m_fullResRawImg);

    releaseBuffer(0);
    SwitchMode(true);
    return 0;
}
