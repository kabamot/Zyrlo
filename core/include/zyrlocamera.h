/*
 * ZyrloCamera.h
 *
 *  Created on: Dec 24, 2020
 *      Author: lev
 */

#ifndef ZYRLOCAMERA_H_
#define ZYRLOCAMERA_H_

#include <opencv2/opencv.hpp>
#include "OFMotionDetector.h"

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

#define IMAGE_CHANGE_SENSITIVITY_F 						2.0f//0.3f //0.15f
#define MOTION_DETECTOR_STEADY_STATE_COUNT_PREVIEW		10 //15
#define MOTION_DETECTOR_STEADY_STATE_COUNT_FULLRES		3

class ZyrloCamera {
        const int m_nFullResImgNum = 1, m_nMinExpValue = 4, m_nMaxExpValue = 1800, m_nAvgTargetBrightness = 150;
        int m_nCurrImgWidth = 0, m_nCurrImgHeight = 0, m_nCurrBytesPerLine = 0;
        int m_fd = -1;
        int m_cr = 256, m_cb = 256;
        int m_nMinExp = m_nMinExpValue, m_nMaxExp = m_nMaxExpValue;
        int m_nDelayCount = 30;

        struct buffer {
                void *start;
                size_t length;
                int DMABUF_Id;
        };

        buffer *m_buffers;
        unsigned int m_nBuffers =  0;
        int m_nGain = 200, m_nExposure = 300;
        int m_nCamBufInd = 0, m_nCnt = 0, m_timeStamp = 0;
        bool m_bPictReq = false, m_wb = false, m_bCameraPause = true, m_bModePreview = true;

        int m_PrintMessLocation = 0;
        FILE *m_PrintMessageFile = NULL;


        COFMotionDetector m_md;
        cv::Mat m_previewImg, m_previewImgPyr1, m_previewImgPyr2, m_ocrImg, m_targetImg, m_targetCorr, m_firstStableImg;
        vector<cv::Mat> m_vFullResRawImgs, m_vFullResImgs, m_vFullResGreyImgs;
        bool m_bEnableGestureUI = true;
        float m_fLookForTargetHighThreshold = 0.8f;
        float m_fLookForTargetLowThreshold = 0.7f;
        float m_fImageChangeSensitivity = IMAGE_CHANGE_SENSITIVITY_F;
        int m_nLookingForTargetCount = 0, m_nMaxLookingForTargetCount = 100;
        cv::Point m_targetPos;
        bool m_bForceCorrelation = false;

        int m_nNoChange = 0, m_nMotionDetected = 0;

        typedef enum {
                eCalibration = 0,
                eLookinForTarget,
                eReadyOnTarget,
                eLookingForStableImage,
                eLookingForGestures
        } ZcState;

        ZcState m_eState = eCalibration;

        void init_mmap();
        void uninit_mmap();
        int start_capturing();
        int stop_capturing();

        void PrintMessage(const char *format, ...);
        int SetMode(bool bModePreview);
        int SwitchMode(bool bModePreview);
        int acquireBuffer(int nBufferInd);
        int releaseBuffer(int nBufferInd);
        int adjustExposure(const cv::Mat & img);
        int setGain(int nValue);
        void ReserExposureLimits();
        float LookForTarget(const cv::Mat & fastPreviewImgBW, const cv::Mat & targetBitmapBW, int nRadius);
        void adjustWb(cv::Mat & bayer);
        bool DetectImageChange(const cv::Mat & img);

public:
        typedef enum {
            eShowPreviewImge = 0,
            eTargetNotFound,
            eReaderReady,
            eStartOcr
        } Zcevent;

        ZyrloCamera();
        int initCamera();
        virtual ~ZyrloCamera();
        void BayerToDownsampledRG2BGR(const cv::Mat & bayer, cv::Mat & BGR, int step) const;
        void BayerToDownsampledRG2Grey(const cv::Mat & bayer, cv::Mat & grey, int step) const;
        void BayerToDownsampledRG2Grey(const cv::Mat & bayer);
        void WB(const cv::Mat & bayer);
        cv::Point2f GetMotion(const cv::Mat & grey);
        void Clear();
        const cv::Mat & GetPreviewImg() const;
        const cv::Mat & GetFullResRawImg(int indx = 0) const;
        const cv::Mat & GetImageForOcr();
        Zcevent AcquireFrameStep();
        int AcquireImage();
        int setExposure(int nValue);
        int adjustColorGains();
        int snapImage();
        void flashLed(int msecs);
        void setLed(bool bOn);
        int AcquireFullResImage(int nGain, int nExposure, int indx);
};

#endif /* ZYRLOCAMERA_H_ */
