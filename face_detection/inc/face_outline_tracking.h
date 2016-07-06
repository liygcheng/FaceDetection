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

#ifndef _FOT_H_
#define _FOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "amcomdef.h"
#include "asvloffscreen.h"

//fot error defines
#define GVS_NoErr         0
#define GVS_ErrUnknow     1
#define GVS_ErrMemo       2
#define GVS_ErrPnt        3
#define GVS_ErrFormat     4
#define GVS_ErrPara       5
#define GVS_ErrFile       6
#define GVS_ErrImg	7
#define GVS_ErrPntOut	8

#define FOT_LANDMARKS 95		//facial points number
#define FOT_LANDMARKS_SIMPLE 10		//facial points number
#define FOT_FOC_0 0x1 // 0 degree
#define FOT_FOC_90 0x2 // 90 degree
#define FOT_FOC_270 0x3 // 270 degre
#define FOT_FOC_180 0x4 // 180 degree

#define FOT_MAX_FACE 4	//max face number supported

#define FOT_IMG_MODE 0
#define FOT_VIDEO_MODE 1




//mode for different point numbers
#define FOT_SIMPLE_MODE 0
#define FOT_ALL_PNT_MODE 1


typedef MPVoid fot_handle;
//#define FOT_API __declspec(dllexport)
#define FOT_API

struct _fot_landmark
{
	MInt32 x;
	MInt32 y;
};
typedef struct _fot_landmark fot_landmark;


struct _fot_region
{
	MInt32 x;
	MInt32 y;
	MInt32 w;
	MInt32 h;
};
typedef struct _fot_region fot_region;


typedef struct _fot_faces
{
	fot_region			*rcFace;				// The bounding box of face
	MInt32					nFace;					// number of faces detected
	MInt32					*lfaceOrient;            // the angle of each face
}fot_faces;

struct _fot_img_info
{
	MInt32            nFormat;   /* image_fot buff format, can be (GVS_FormatBGR24/GVS_FormatGray) */
	MByte*           pBuff;     /* pointer to image_fot buff                                      */
	MInt32            width;     /* width for the image_fot buff                                   */
	MInt32            height;    /* height for the image_fot buff                                  */
	MInt32            widthStep; /* linebytes for the image_fot buff                               */
};
typedef struct _fot_img_info fot_img_info;

//fot version info
typedef struct 
{
	MInt32            lCodebase; 
	MInt32            lMajor;
	MInt32            lMinor;
	MInt32            lBuild;
	MTChar           Version[50];
	MTChar           BuildDate[20];
	MTChar           CopyRight[60];
} FOT_VERSION;


FOT_API fot_handle fot_create(MHandle memHandle);

FOT_API MVoid fot_release(MHandle memHandle, fot_handle* gh);

FOT_API MInt32 fot_search(MHandle memHandle,
								const fot_handle    gh,       /* fot handle                            */
								const fot_img_info* pImgInfo, /* image_fot info                            */		   
								fot_faces * faces,						/* face information	*/
								MInt32  mode,							/*	image_fot or video, 0- image, 1 - video, 2 - image on video */
								MInt32 model_type,
								fot_landmark *       pOut,     /* output landmarks                */
								MPVoid              pRsv      /* reserved (set it to NULL, always)     */
								); 

FOT_API const FOT_VERSION* FOT_GetVersion();

#ifdef __cplusplus
}
#endif

#endif