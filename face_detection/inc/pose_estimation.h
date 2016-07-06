/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and 		
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employees 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an intended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/


#ifndef _FACE_ATTRIBUTES_H_
#define _FACE_ATTRIBUTES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ammem.h"
#include "asvloffscreen.h"

#define POSE_NoErr         0
#define POSE_ErrUnknow     1
#define POSE_ErrMemo       2
#define POSE_ErrPnt        3
#define POSE_ErrFormat     4
#define POSE_ErrPara       5
#define POSE_ErrFile       6
#define POSE_ErrImg	7
#define POSE_ErrUnSuprt	8

#define POSE_FOC_0 0x1 // 0 degree
#define POSE_FOC_90 0x2 // 90 degree
#define POSE_FOC_270 0x3 // 270 degre
#define POSE_FOC_180 0x4 // 180 degree

#define POSE_MAX_FACE 10	//max face number supported

#define POSE_IMG_MODE 0
#define POSE_VIDEO_MODE 1
#define POSE_LANDMARKS 95

typedef MPVoid pose_handle;

//#define FOT_API __declspec(dllexport)
#define POSE_API

// face rectangle
struct _pose_region
{
	MInt32 x;
	MInt32 y;
	MInt32 w;
	MInt32 h;
};
typedef struct _pose_region pose_region;


typedef struct _pose_faces
{
	pose_region			*rcFace;				// The bounding box of face
	MInt32					nFace;					// number of faces detected
	MInt32					*lfaceOrient;            // the angle of each face
}pose_faces;



typedef struct _pose_landmark
{
	MInt32 x;
	MInt32 y;
}pose_landmark;

//fat version info
typedef struct 
{
	MInt32            lCodebase; 
	MInt32            lMajor;
	MInt32            lMinor;
	MInt32            lBuild;
	MTChar           Version[50];
	MTChar           BuildDate[20];
	MTChar           CopyRight[60];
} POSE_VERSION;


POSE_API pose_handle pose_create(MHandle memHandle);

POSE_API MVoid pose_release(MHandle memHandle, pose_handle* gh);

POSE_API MInt32 pose_estimate(MHandle memHandle,
						  const pose_handle    gh,       /* fat handle                            */
						  pose_landmark * pose_ld, /* input landmarks*/
						  MInt32 nface,						/* face number */
						  MFloat * pAngle					/* output angles (roll, yaw, pitch) */
						  ); 

POSE_API const POSE_VERSION* POSE_GetVersion();


#ifdef __cplusplus
}
#endif

#endif