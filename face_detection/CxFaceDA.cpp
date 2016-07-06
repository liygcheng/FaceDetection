/***************************************************************
// Author: Xuan Cheng, ArcSoft
// Email: chengxuan90@gmail.com
****************************************************************/
#include <ammem.h>
#include <arcsoft_face_detection.h>
#include <merror.h>
#include <face_outline_tracking.h>
#include "CxFaceDA.h"
#include "ImageRGB.h"
#include "ImageYUV.h"



#define IsSeparatedGrayChannel(format) (((format)>>8)>=6)
#define GET_WITHBYTES(width, bitCount) (((width)*(bitCount)+31)/32*4)

typedef struct _tag_KPoint2D {
	long x;
	long y;
} KPoint2D, *LPKPoint2D;

void CxFaceDA::ArcSoftFaceDetection(CMat& img3u, int faceNum, int faceScale, vecR& rects)
{
	assert(rects.size() == 0);

	ImageRGB* pImgRGB=new ImageRGB();
	pImgRGB->height = img3u.rows;
	pImgRGB->width = img3u.cols;
	pImgRGB->channels = img3u.channels();
	pImgRGB->widthBytes = GET_WITHBYTES(pImgRGB->width,pImgRGB->channels*8);
	pImgRGB->maxSize = pImgRGB->widthBytes * pImgRGB->height;
	pImgRGB->origin = 0;
	pImgRGB->pData = new Byte[pImgRGB->maxSize];

	for(int h=0; h<img3u.rows; h++){
		const Vec3b* ptr = img3u.ptr<Vec3b>(h);
		for(int w=0; w<img3u.cols; w++, ptr++){
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 0] = (*ptr)[0]; 
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 1] = (*ptr)[1]; 
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 2] = (*ptr)[2]; 
		}
	}
	ImageYUV* pImgYUV=new ImageYUV(pImgRGB->width, pImgRGB->height, ASVL_PAF_NV21);


	//cv::Mat* pYUV = new Mat(pImgRGB->width, pImgRGB->height, cv::COLOR_RGBA2YUV_YV12);



	ConvertImageRGB2YUV(pImgRGB, pImgYUV);



	AFD_FACERES faceBB={0};
	MHandle hMemMgr = 0;
	int hr = 0 ;
	AFD_ENGINE FaceEngine = NULL;
	MLong size_of_buffer =1024*1024*100; 
	void *membuffer=(void *)malloc(size_of_buffer);
	faceBB.lfaceOrient = (MLong*)malloc(sizeof(MLong)*faceNum);
	faceBB.rcFace = (MRECT*)malloc(sizeof(MRECT)*faceNum);

	hMemMgr=(MHandle)MMemMgrCreate(membuffer, size_of_buffer);

	//if (hMemMgr == NULL)
	//{
	//	if (faceBB.rcFace != NULL) free(faceBB.rcFace);
	//	if (faceBB.lfaceOrient != NULL) free(faceBB.lfaceOrient);
	//	delete pImgRGB;
	//	delete pImgYUV;
	//	return;
	//}

	hr = AFD_InitialFaceEngine(hMemMgr,&FaceEngine,AFD_OPF_0_HIGHER, faceScale, faceNum);

	do{
		hr=AFD_StillImageFaceDetection(hMemMgr,FaceEngine, (LPASVLOFFSCREEN)pImgYUV, &faceBB,NULL,NULL,NULL,NULL);
	}while(hr==MERR_USER_PAUSE);

	AFD_UninitialFaceEngine(hMemMgr,FaceEngine);
	//MMemMgrDestroy(hMemMgr);
	free(hMemMgr);

	rects.resize(faceBB.nFace);
	for(unsigned int i=0; i<rects.size(); i++){
		rects[i].x = faceBB.rcFace[i].left;
		rects[i].y = faceBB.rcFace[i].top;
		rects[i].width = faceBB.rcFace[i].right - faceBB.rcFace[i].left;
		rects[i].height = faceBB.rcFace[i].bottom - faceBB.rcFace[i].top;
	}

	if(faceBB.rcFace != NULL) free(faceBB.rcFace);
	if(faceBB.lfaceOrient != NULL) free(faceBB.lfaceOrient);
	delete pImgRGB;
	delete pImgYUV;
	return;
}
void CxFaceDA::ArcSoftFaceAlignment(CMat& img3u, int faceScale, vecP2d& landmarks)
{
	assert(landmarks.size() == 0);
	int faceNum = 1;   // assume only one face is detected

	ImageRGB* pImgRGB = new ImageRGB();
	pImgRGB->height = img3u.rows;
	pImgRGB->width = img3u.cols;
	pImgRGB->channels = img3u.channels();
	pImgRGB->widthBytes = GET_WITHBYTES(pImgRGB->width, pImgRGB->channels * 8);
	pImgRGB->maxSize = pImgRGB->widthBytes * pImgRGB->height;
	pImgRGB->origin = 0;
	pImgRGB->pData = new Byte[pImgRGB->maxSize];

	for (int h = 0; h < img3u.rows; h++){
		const Vec3b* ptr = img3u.ptr<Vec3b>(h);
		for (int w = 0; w < img3u.cols; w++, ptr++){
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 0] = (*ptr)[0];
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 1] = (*ptr)[1];
			((uchar *)(pImgRGB->pData + h*pImgRGB->widthBytes))[w*pImgRGB->channels + 2] = (*ptr)[2];
		}
	}
	ImageYUV* pImgYUV = new ImageYUV(pImgRGB->width, pImgRGB->height, ASVL_PAF_NV21);
	ConvertImageRGB2YUV(pImgRGB, pImgYUV);

	/*************************** face detection *****************************************/
	AFD_FACERES faceBB = { 0 };
	MHandle hMemMgr = 0;
	int hr = 0;
	AFD_ENGINE FaceEngine = NULL;
	MLong size_of_buffer = 1024 * 1024 * 100;
	void *membuffer = (void *)malloc(size_of_buffer);
	faceBB.lfaceOrient = (MLong*)malloc(sizeof(MLong)*faceNum);
	faceBB.rcFace = (MRECT*)malloc(sizeof(MRECT)*faceNum);

	hMemMgr = (MHandle)MMemMgrCreate(membuffer, size_of_buffer);
	hr = AFD_InitialFaceEngine(hMemMgr, &FaceEngine, AFD_OPF_0_HIGHER, faceScale, faceNum);

	do{
		hr = AFD_StillImageFaceDetection(hMemMgr, FaceEngine, (LPASVLOFFSCREEN)pImgYUV, &faceBB, NULL, NULL, NULL, NULL);
	} while (hr == MERR_USER_PAUSE);

	AFD_UninitialFaceEngine(hMemMgr, FaceEngine);
	/*********************************************************************************/

	/*************************** face alignment *****************************************/
	fot_img_info gii;
	gii.pBuff = ((LPASVLOFFSCREEN)pImgYUV)->ppu8Plane[0];
	gii.nFormat = ((LPASVLOFFSCREEN)pImgYUV)->u32PixelArrayFormat;
	if (IsSeparatedGrayChannel(((LPASVLOFFSCREEN)pImgYUV)->u32PixelArrayFormat))
		gii.nFormat = ASVL_PAF_GRAY;
	gii.width = ((LPASVLOFFSCREEN)pImgYUV)->i32Width;
	gii.height = ((LPASVLOFFSCREEN)pImgYUV)->i32Height;
	gii.widthStep = ((LPASVLOFFSCREEN)pImgYUV)->pi32Pitch[0];

	KPoint2D pLandmarks[FOT_LANDMARKS];

	MRECT *pFaceRect;
	pFaceRect = &(faceBB.rcFace[0]);     // we assume only one face has been detected
	fot_region facell;
	facell.x = pFaceRect->left;
	facell.y = pFaceRect->top;
	facell.w = pFaceRect->right - pFaceRect->left;
	facell.h = pFaceRect->bottom - pFaceRect->top;

	fot_faces face;
	face.rcFace = &facell;
	int orient = faceBB.lfaceOrient[0];
	face.lfaceOrient = &orient;
	face.nFace = 1;

	MHandle hFot = fot_create(hMemMgr);
	hr = fot_search(hMemMgr, hFot, &gii, &face, FOT_VIDEO_MODE, FOT_ALL_PNT_MODE, (fot_landmark*)pLandmarks, NULL);

	landmarks.resize(FOT_LANDMARKS);
	for (int i = 0; i < FOT_LANDMARKS; i++){
		landmarks[i][0] = pLandmarks[i].x;
		landmarks[i][1] = pLandmarks[i].y;
	}

	fot_release(hMemMgr, &hFot);
	/********************************************************************************************/

	MMemMgrDestroy(hMemMgr);
	if (faceBB.rcFace != NULL) free(faceBB.rcFace);
	if (faceBB.lfaceOrient != NULL) free(faceBB.lfaceOrient);
	delete pImgRGB;
	delete pImgYUV;
	return;
}
