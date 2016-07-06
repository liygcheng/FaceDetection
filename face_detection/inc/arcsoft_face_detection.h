  /*******************************************************************************
Copyright(c) ArcSoft, All right reserved.

This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary 
and confidential information. 

The information and code contained in this file is only for authorized ArcSoft 
employees to design, create, modify, or review.

DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER 
AUTHORIZATION.

If you are not an intended recipient of this file, you must not copy, 
distribute, modify, or take any action in reliance on it. 

If you have received this file in error, please immediately notify ArcSoft and 
permanently delete the original and any copy of any file and any printout 
thereof.
*******************************************************************************/

#ifndef _ARCSOFT_FACEDETECTION_H_
#define _ARCSOFT_FACEDETECTION_H_

#include "amcomdef.h"
#include "asvloffscreen.h"

#ifdef __cplusplus
extern "C" {
#endif
	

typedef MVoid**				AFD_ENGINE;	
typedef MLong				AFD_ORIENTPRI;
typedef MLong				AFD_FACEOUTPRI;

//orientation priority
enum AFD_OrientPriority{
	AFD_OPF_0_HIGHER		= 0x2,		/* 0 higher priority: 0,90,0,270,0,90...*/
	AFD_OPF_0_ONLY			= 0x3,		/* 0 only:0,0...*/
	AFD_OPF_90_ONLY			= 0x4,		/* 90 only:90,90...*/
	AFD_OPF_270_ONLY		= 0x5		/* 270 only:270,270...*/
};


//face orientation code
enum AFD_OrientCode{
	AFD_FOC_0				= 0x1,		// 0 degree
	AFD_FOC_90				= 0x2,		// 90 degree
	AFD_FOC_270				= 0x3,		// 270 degree
	AFD_FOC_180				= 0x4,		// 180 degree
	AFD_FOC_30				= 0x5,		// 30 degree
	AFD_FOC_60				= 0x6,		// 60 degree
	AFD_FOC_120				= 0x7,		// 120 degree
	AFD_FOC_150				= 0x8,		// 150 degree
	AFD_FOC_210				= 0x9,		// 210 degree
	AFD_FOC_240				= 0xa,		// 240 degree
	AFD_FOC_300				= 0xb,		// 300 degree
	AFD_FOC_330				= 0xc		// 330 degree
};

/*Face output priority */
enum AFD_FaceOutPriority{
	AFD_FOP_SIMILARITY		= 0x0,		// similarity priority
	AFD_FOP_CENTER			= 0x1,		// center priority
	AFD_FOP_SIZE			= 0x2		// size priority
};


typedef struct{
	MRECT 					*rcFace;				// The bounding box of face
	MLong					nFace;					// number of faces detected
	MLong					*lfaceOrient;            // the angle of each face
} AFD_FACERES, *LPAFD_FACERES;

typedef struct
{
	MLong lCodebase;			// Codebase version number 
	MLong lMajor;				// major version number 
	MLong lMinor;				// minor version number
	MLong lBuild;				// Build version number, increasable only
	const MTChar *Version;		// version in string form
	const MTChar *BuildDate;	// latest build Date
	const MTChar *CopyRight;	// copyright 
} ArcSoft_Face_Detection_Version;

/************************************************************************
 * This function is implemented by the caller and will be called 
 * when faces are detected.
 ************************************************************************/
typedef MRESULT (*AFD_FNPROGRESS) (
	MVoid       *pParam1,					// detection result data.
	MVoid		*pParam2					// Caller-defined data
);

/************************************************************************
 * The function used to detect face in the input image. 
 ************************************************************************/
MRESULT AFD_StillImageFaceDetection(			        // Return MOK if successes, return MERR_USER_PAUSE if on process, otherwise fails
	 MHandle				hMemMgr,		                // [in]  User defined memory manager
	 AFD_ENGINE				hEngine,		                // [in]	 Detect engine/
	 LPASVLOFFSCREEN		pImgData,		                // [in]  The original image data
	 LPAFD_FACERES			pFaceRes,		                // [out] The detection result
	 AFD_FNPROGRESS			fnGetPartialFaceRes,	        // [in]  The callback function
	 MVoid					*pGetPartialFaceResParam,		// [in]  Caller-specific data that will be passed into the callback function
	 AFD_FNPROGRESS			fnTimeOut,	                    // [in]  The callback function
	 MVoid					*pTimeOutParam		            // [in]  Caller-specific data that will be passed into the callback function
);

/************************************************************************
 * The function used to Initialize the face detection engine. 
 ************************************************************************/
MRESULT AFD_InitialFaceEngine(	 
	MHandle					hMemMgr,		// [in]	User defined memory manager
	AFD_ENGINE				*pEngine,		// [out] pointing to the AFD_ENGINE structure containing the detection engine.
	AFD_ORIENTPRI			iOrientsFlag,	// [in] Defining the priority of face orientation.
	MLong					nScale,			// [in]	An integer defining the minimal face to detect relative to the maximum of image width and height.
	MLong					nMaxFaceNum		// [in]	An integer defining the number of max faces to detection
);

/************************************************************************
 * The function used to Uninitialize the detection module. 
 ************************************************************************/
MRESULT AFD_UninitialFaceEngine(
	MHandle					hMemMgr,					// [in]	User defined memory manager
	AFD_ENGINE				pEngine				// [in] pointing to the AFD_ENGINE structure containing the detection engine.
);


/************************************************************************
 * The function used to set face priority. 
 ************************************************************************/
MRESULT AFD_SetFaceOutPriority(
	 AFD_ENGINE				hEngine,			// [in]	 Face Detection engine
	 AFD_FACEOUTPRI			lPrioity			// [in]  Face priority
);
/************************************************************************
 * The function used to get version information of face detection library. 
 ************************************************************************/
const ArcSoft_Face_Detection_Version * ArcSoft_Face_Detection_GetVersion();


#ifdef __cplusplus
}
#endif

#endif //_ARCSOFT_FACEDETECTION_H_
