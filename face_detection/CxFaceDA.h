/***************************************************************
// Author: Xuan Cheng, ArcSoft
// Email: chengxuan90@gmail.com
****************************************************************/

#ifndef CXFACEDA_H
#define CXFACEDA_H

#include <cv.h>
//#pragma comment(lib,"opencv_core2413d.lib")  
//#pragma comment(lib,"opencv_highgui2413d.lib")  

using namespace cv;
typedef vector<string> vecS;
typedef vector<int> vecI;

typedef vector<float> vecF;
typedef vector<double> vecD;
typedef vector<Mat> vecM;
typedef vector<Rect> vecR;
typedef vector<Point> vecP;
typedef const Mat CMat;

typedef std::vector<cv::Vec2f> vecP2d;

/**
 * @brief  Face detection and alignment using ArcSoft's software.
 *
 */
class CxFaceDA
{
public:
	static void ArcSoftFaceDetection(CMat& img3u, int faceNum,  int faceScale,vecR& rects);
	static void ArcSoftFaceAlignment(CMat& img3u, int faceScale, vecP2d& landmarks);
};

#endif