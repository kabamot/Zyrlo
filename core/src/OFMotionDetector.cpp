#include "OFMotionDetector.h"
//#include "PreprocFuncs.h"

using namespace cv;
using namespace std;

typedef unsigned char UCHAR;

void build_diff(const Mat & img0, const Mat & img1,
		Mat & imgDiff, int *pHist, Rect &roi){
	memset(pHist, 0, sizeof(int) * 256);
	imgDiff.setTo(0);
	const Mat & img0Roi = img0(roi), & img1Roi = img1(roi);
	Mat imgDiffRoi = imgDiff(roi);
	for(int i = 0; i != roi.height; ++i) {
		const UCHAR *pI0 = img0Roi.ptr(i), *pI1 = img1Roi.ptr(i);
		const UCHAR *pI0End = pI0 + roi.width;
		UCHAR *pD = imgDiffRoi.ptr(i);
		for(; pI0 < pI0End; ++pI0, ++pI1, ++pD) {
			*pD = abs(int(*pI1) - int(*pI0));
			++pHist[*pD];
		}
	}
}

UCHAR find_threshold(const int *pHist, int N){
	int nSum = 0;
	int i=255;
	while(nSum < N && i>=1){
		nSum += pHist[i];
		i--;
	}
	return i;
}

void findOF(float ggxx,float ggyy,float ggxy, float gdx,float gdy,float & dx,float & dy){
	float det = (ggxx * ggyy - ggxy * ggxy);
	if(fabs(det) > 1.0){
		det = 1.0f/det;
		dx = (gdx * ggyy - gdy * ggxy) * det;
		dy = (gdx * ggxy - gdy * ggxx) * det;
	}
	else{
		dx=dy=0.0;
	}
}

COFMotionDetector::COFMotionDetector () {
}

COFMotionDetector::~COFMotionDetector () {
}

void COFMotionDetector::Init (const Mat & grey) {
	m_nW = grey.cols;
	m_nH = grey.rows;
	m_prevImg.create(m_nH, m_nW, CV_8U);
	m_diffImg .create(m_nH, m_nW, CV_8U);
	m_currImg.create(m_nH, m_nW, CV_8U);
	
	grey.copyTo(m_currImg);
	Rect roi(0, 0, m_nW, m_nH);
	build_diff(m_prevImg, m_currImg, m_diffImg, m_pHist, roi);
	m_currImg.copyTo(m_prevImg);
	
	int cellSize = 40;
	int halfCell = cellSize / 2;
	static float cx = float(m_nW/2),cy = float(m_nH/2);
	int i,j;
	for(i=0;i<3;i++){
		for(j=0;j<3;j++){
			int ind = i*3+j;
			m_frame[ind].x = cx-halfCell+(j-1)*cellSize;
			m_frame[ind].y = cy-halfCell+(i-1)*cellSize;
			m_frame[ind].width = m_frame[ind].height = cellSize;
		}
	}
	m_bInitialized = true;
}

bool COFMotionDetector::IsInitialized()const { return m_bInitialized;}

void COFMotionDetector::Clear(){
	if(!m_prevImg.empty())m_prevImg.setTo(0);
	if(!m_diffImg.empty())m_diffImg.setTo(0);
	if(!m_currImg.empty())m_currImg.setTo(0);
}

bool CalcMotionInRect(const Mat & currImg, const Mat & prevImg, const Mat & diffImg,
					   const Rect &rect, int nPoints,
					   float &fdx, float &fdy, float fSensitivity, bool bNoDir);
	
Point2f COFMotionDetector::GetMotion(const Mat & grey){
	if(!m_bInitialized) {
		Init(grey);
	}
	grey.copyTo(m_currImg);
	Rect roi(0, 0, m_currImg.cols, m_currImg.rows);
	build_diff(m_prevImg, m_currImg, m_diffImg, m_pHist, roi);
	float dX = 0, dY = 0, fDX[9], fDY[9], maxD = 0, cMax = 0;
	int maxInd = 0;
	Rect vmrectBig(1, 1, m_currImg.cols - 2, m_currImg.rows - 2);
	if(CalcMotionInRect(m_currImg, m_prevImg, m_diffImg,
		vmrectBig, m_nPoints, dX, dY, 0.5, true)){
		for(int i=0; i<9; ++i){
			fDX[i] = dX;
			fDY[i] = dY;
			if(CalcMotionInRect(m_currImg, m_prevImg, m_diffImg, m_frame[i], m_nPoints,
				fDX[i], fDY[i], 0.5, false)){
				if((cMax = fDX[i] * fDX[i] + fDY[i] * fDY[i]) > maxD){
					maxD = cMax;
					maxInd = i;
				}
			}
			
		}
	}
	if(maxD > 1e-6){
		dX = fDX[maxInd];
		dY = fDY[maxInd];
	}
	else{
		dX = dY = 0.0f;
	}
	m_currImg.copyTo(m_prevImg);
	return Point2f(dX, dY);
}



void build_histogram(const Mat & img, const Rect & frame,int *pHist){
	memset(pHist,0,sizeof(int)*256);
	
	Rect roi = Rect(1, 1, img.cols - 2, img.rows - 2) & frame;
	const Mat & imgRoi = img(roi);

	for(int i= 0; i != imgRoi.rows; ++i)
		for(const UCHAR *p = img.ptr(i),*pe = p + img.cols; p != pe; ++p)
			pHist[*p]++;
}

bool CalcMotionInRect(const Mat & currImg, const Mat & prevImg, const Mat & diffImg, const Rect & rect, int nPoints,
					   float &fdx, float &fdy, float fSensitivity, bool bNoDir){
	
	static int pHist[256];
	build_histogram(diffImg, rect, pHist);
	unsigned char threshold = find_threshold(pHist,nPoints);
	Rect roi = Rect(1, 1, currImg.cols - 2, currImg.rows - 2) & rect;

	const Mat & currImgRoi = currImg(roi), & prevImgRoi = prevImg(roi), & diffImgRoi = diffImg(roi);
	const UCHAR *pCI, *pPI, *pDI, *pLEnd;
	int nRP = 0, nWP = 0; 
	int dx, dy, diff, nCurrWidth = currImg.step[0], nPrevWidth = prevImg.step[0];
	float ggxx = 0.0f, ggyy = 0.0f, ggxy = 0.0f, gdx = 0.0f, gdy = 0.0f;
	for(int i = 0; i != currImgRoi.rows; ++i) {
		pCI = currImgRoi.ptr(i);
		pPI = prevImgRoi.ptr(i);
		pDI = diffImgRoi.ptr(i);
		for(pLEnd = pCI + currImgRoi.cols; pCI != pLEnd; ++pCI, ++pPI, ++pDI){
			if(*pDI >= threshold){
				dx = (int(*(pPI+1)) - int(*(pPI-1)) + int(*(pCI+1)) - int(*(pCI-1)));
				dy = (int(*(pPI+nPrevWidth)) - int(*(pPI-nPrevWidth)) + int(*(pCI+nCurrWidth)) - int(*(pCI-nCurrWidth)));
				diff = int(*pCI) - int(*pPI);
				if(bNoDir || (dx*fdx+dy*fdy)*diff>0.0){
					ggxx += dx*dx;
					ggyy += dy*dy;
					ggxy += dx*dy;
					gdx += diff*dx;
					gdy += diff*dy;
					++nRP;
				}
				else{
					++nWP;
				}
			}

		}
	}
	float fP = (nRP-nPoints / 2) * fSensitivity;
    if(bNoDir || fP * fP >= float(nPoints)){
		findOF(ggxx,ggyy,ggxy,gdx,gdy,fdx,fdy);
		fdx *= 4.0f;
		fdy *= 4.0f;
		return true;
	}
	fdx = fdy = 0;
	return false;
}
