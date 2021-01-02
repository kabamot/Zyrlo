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

class ZyrloCamera {
        int m_nCurrImgWidth = 0, m_nCurrImgHeight = 0, m_nCurrBytesPerLine = 0;
        int m_fd = -1;
        int m_cr = 256, m_cb = 256;

        struct buffer {
                void *start;
                size_t length;
                int DMABUF_Id;
        };

        buffer *m_buffers;
        unsigned int m_nBuffers =  0;
        int m_nCamBufInd = 0, m_nCnt = 0, m_timeStamp = 0;
        bool m_bPictReq = false, m_wb = false;

        int m_PrintMessLocation = 0;
        FILE *m_PrintMessageFile = NULL;


        COFMotionDetector m_md;
        cv::Mat m_previewImg, m_previewImgPyr1, m_previewImgPyr2;
        bool m_bEnableGestureUI = true;
        float m_fLookForTargetHighThreshold = 0.8f;
        float m_fLookForTargetLowThreshold = 0.7f;

        typedef enum {
                eLookingForTargetForCalibration = 0,
                eCalibrationExposure,
                eReadyOnTarget,
                eLookingForStableimage,
                eLookingForGestures
        } ZcState;

        ZcState m_eState = eLookingForTargetForCalibration;

        void init_mmap();
        void uninit_mmap();
        int start_capturing();
        void PrintMessage(const char *format, ...);
        int SetMode(bool bModePreview);
        int acquireBuffer(int nBufferInd);
        int releaseBuffer(int nBufferInd);

public:
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
        void PreviewProcessStep(const cv::Mat &prevFrame);
        int AcquireImage();
};

#endif /* ZYRLOCAMERA_H_ */
