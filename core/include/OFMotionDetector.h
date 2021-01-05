#ifndef __OFMOTIONDETECTOR__
#define __OFMOTIONDETECTOR__

#include "opencv2/opencv.hpp"

#define MAX_SHIFT 30
#define SPEED_RADIUS 3
#define MIN_VOTE 10

#include <vector>

using namespace std;

class COFMotionDetector {
	bool m_bInitialized = false;
	cv::Mat m_prevImg, m_diffImg, m_currImg;
	int m_pHist[256];
	int m_nPoints = 300, m_nW = 0, m_nH = 0;
	cv::Rect m_frame[9];

public:
	COFMotionDetector();

	~COFMotionDetector();

	void Init (const cv::Mat & grey);
	
	cv::Point2f GetMotion(const cv::Mat & grey);
	bool IsInitialized()const;
	void Clear();
};

void build_histogram(const cv::Mat & img, const cv::Rect & frame,int *pHist);

#endif
