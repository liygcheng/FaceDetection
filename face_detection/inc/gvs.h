/*----------------------------------------------------------------------------------------------
*
* GVS headfile
* GVS is a toolkit to locate landmarks on faces automaticlly.            
*
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* copyright(c) 2007                                                 
*
*----------------------------------------------------------------------------------------------*/

//#include "geo.h"


#ifndef _GVS_H_
#define _GVS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "amcomdef.h"
#include "asvloffscreen.h"
//#include "khas.h"

#define _FUJITSU

//_WIN32
#ifdef _WIN32
#ifdef GVS_EXPORTS
#define GVS_API __declspec(dllexport)
#else
#define GVS_API // __declspec(dllimport)
#endif
#endif 

//_MACOS
#ifdef _MACOS
#define GVS_API
#endif

//_COACH
#ifdef _COACH
#define GVS_API
#endif

//_FUJITSU
#ifdef _FUJITSU
#define GVS_API
#endif

//gvs handle
typedef MPVoid gvs_handle;

//gvs error defines
#define GVS_NoErr         0
#define GVS_ErrUnknow     1
#define GVS_ErrMemo       2
#define GVS_ErrPnt        3
#define GVS_ErrFormat     4
#define GVS_ErrPara       5
#define GVS_ErrFile       6

//gvs support image format
#define GVS_FormatBGR24   1
#define GVS_FormatGray    2

//gvs landmarks
#define GVS_Landmarks     95
#define GVS_EyeLandmarks  24
#define GVS_LeftEyeLandmarks 12
#define GVS_RightEyeLandmarks 12
#define GVS_MouthLandmarks  20
#define GVS_EyeBrowLandmarks  20
#define GVS_FaceLandmarks  19

#define AFF_FOC_0 0x1 // 0 degree
#define AFF_FOC_90 0x2 // 90 degree
#define AFF_FOC_270 0x3 // 270 degree
#define AFF_FOC_180 0x4 // 180 degree

//gvs landmark struct
struct _gvs_landmark
{
	MLong x;
	MLong y;
};
typedef struct _gvs_landmark gvs_landmark;

//gvs region struct
struct _gvs_region
{
	MLong x;
	MLong y;
	MLong w;
	MLong h;
};
typedef struct _gvs_region gvs_region;

struct _puzzle_region
{
	signed int x;
	signed int y;
	signed int w;
	signed int h;
};
typedef struct _puzzle_region puzzle_region;

struct _MPOINT32
{
	signed int x;
	signed int y;
};
typedef struct _MPOINT32 MPOINT32;

//gvs version info
typedef struct 
{
	MLong            lCodebase;   /* image buff format, can be (GVS_FormatBGR24/GVS_FormatGray) */
	MLong            lMajor;     /* pointer to image buff                                      */
	MLong            lMinor;     /* width for the image buff                                   */
	MLong            lBuild;    /* height for the image buff                                  */
    MTChar           Version[50]; /* linebytes for the image buff                               */
	MTChar           BuildDate[20];
	MTChar           CopyRight[60];
} GVS_VERSION;


//gvs image info
struct _gvs_img_info
{
	MLong            nFormat;   /* image buff format, can be (GVS_FormatBGR24/GVS_FormatGray) */
	MByte*           pBuff;     /* pointer to image buff                                      */
	MLong            width;     /* width for the image buff                                   */
	MLong            height;    /* height for the image buff                                  */
	MLong            widthStep; /* linebytes for the image buff                               */
};
typedef struct _gvs_img_info gvs_img_info;

//flag for key points  //显示的关键点
static const MLong gvs_keypnts[GVS_Landmarks] =
{       
// 	1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1, //double lines      (19)           
//     1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,       //brows             (16)           
//     1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,       //eyes              (16)           		           
//     1,0,0,1,0,1,1,0,1,0,0,1,               //nose              (12)           
//     1,0,0,1,0,0,1,0,0,1,0,0,               //mouth(extern lip) (12)           
//     0,0,1,0,0,0,1,0,                       //mouth(intern lip) (8)   
	1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,
	1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,
	1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,
	1,0,0,1,0,1,1,0,1,0,0,1,
	1,0,0,1,0,0,1,0,0,1,0,0,
	0,0,1,0,0,0,1,0,

// 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
// 	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
// 	0,0,0,0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,	


};

//create a gvs handle(return: gvs handle)
GVS_API gvs_handle gvs_create(MHandle memHandle);
GVS_API gvs_handle gvs_create_global(MHandle memHandle, MShort* data);
GVS_API gvs_handle gvs_create_eye(MHandle memHandle, MShort* data);
GVS_API gvs_handle gvs_create_lefteye(MHandle memHandle, MShort* data);
GVS_API gvs_handle gvs_create_righteye(MHandle memHandle, MShort* data);
GVS_API gvs_handle gvs_create_mouth(MHandle memHandle, MShort* data);
/* MHandle gvs_create(gvs_handle gh);*/
//release a gvs handle (return: N/A)
GVS_API MVoid gvs_release(gvs_handle* pgh, MHandle memHandle);           /* pointer to gvs handle                 */
GVS_API MVoid gvs_release_global(gvs_handle* pgh, MHandle memHandle);
GVS_API MVoid gvs_release_eye (gvs_handle* pgh, MHandle memHandle);
GVS_API MVoid gvs_release_lefteye(gvs_handle* pgh, MHandle memHandle);
GVS_API MVoid gvs_release_righteye(gvs_handle* pgh, MHandle memHandle);
GVS_API MVoid gvs_release_mouth(gvs_handle* pgh, MHandle memHandle);

//landmarks locating with gvs (return: error code)
// GVS_API MLong gvs_search(const gvs_handle    gh,       /* gvs handle                            */
// 					     MHandle memHandle,
// 						 const gvs_img_info* pImgInfo, /* image info                            */		   
// 					     gvs_region          FaceRec,
// 					     gvs_landmark*       pOut,     /* output landmarks                      */
// 					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
//                          MLong               MouthOpen
					
                         
						 /*khas_handle* khas*/
//					   );    
GVS_API MLong gvs_search_global(const gvs_handle    gh,       /* gvs handle                            */
					     MHandle memHandle,
						 const gvs_img_info* pImgInfo, /* image info                            */		   
					     puzzle_region       FaceRec,
					     gvs_landmark*       pOut,     /* output landmarks                      */
					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
                         MLong               MouthOpen,
						 MPOINT32*           puzzle_para,
						 MLong               ikey
						 /*khas_handle* khas*/
					   ); 
GVS_API MLong gvs_search_eye(const gvs_handle    gh,       /* gvs handle                            */
					     MHandle memHandle,
						 const gvs_img_info* pImgInfo, /* image info                            */		   
					     puzzle_region          FaceRec,
					     gvs_landmark*       pOut,     /* output landmarks                      */
					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
                         MLong               MouthOpen,
						 MLong               ikey
						 /*khas_handle* khas*/
					   );    
GVS_API MLong gvs_search_lefteye(const gvs_handle    gh,       /* gvs handle                            */
					     MHandle memHandle,
						 const gvs_img_info* pImgInfo, /* image info                            */		   
					     puzzle_region          FaceRec,
					     gvs_landmark*       pOut,     /* output landmarks                      */
					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
                         MLong               MouthOpen,
                         MLong               ikey
						 /*khas_handle* khas*/
					   );    
GVS_API MLong gvs_search_righteye(const gvs_handle    gh,       /* gvs handle                            */
					     MHandle memHandle,
						 const gvs_img_info* pImgInfo, /* image info                            */		   
					     puzzle_region          FaceRec,
					     gvs_landmark*       pOut,     /* output landmarks                      */
					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
                         MLong               MouthOpen,
						 MLong               ikey
						 /*khas_handle* khas*/
					   );  
GVS_API MLong gvs_search_mouth(const gvs_handle    gh,       /* gvs handle                            */
					     MHandle memHandle,
						 const gvs_img_info* pImgInfo, /* image info                            */		   
					     puzzle_region          FaceRec,
					     gvs_landmark*       pOut,     /* output landmarks                      */
					     MPVoid              pRsv,      /* reserved (set it to NULL, always)     */
                         MLong               MouthOpen,
						 MLong               ikey,
						 MLong* RotPara,
						 MLong* ScalePara
						 /*khas_handle* khas*/
					   );       
//gvs refine landmarks based on interaction
//  MLong gvs_refine(const gvs_handle    gh,       /* gvs handle                            */
// 			             MHandle memHandel,
// 						 const gvs_img_info* pImgInfo, /* image info                            */
// 			           const gvs_landmark* pIn,      /* input landmarks                       */
// 			           const MLong*          pLocked,  /* flags for those locked landmarks      */
// 			           const gvs_landmark* pInLocks, /* positions for locked landmarks        */
// 			           gvs_landmark*       pOut      /* output landmarks                      */
// 			           );
GVS_API const GVS_VERSION* GVS_GetVersion();
//gvs feature pnt
GVS_API MLong GetFacialFeatures(MHandle memHandle, const gvs_img_info* pImgInfo, puzzle_region FaceRec, MPOINT* feapnt);
// MVoid GetEyeLocation(MHandle memHandle, const gvs_img_info* pImgInfo, gvs_region FaceRec,MPOINT* EyeCenter, gvs_region* EyeRec);
GVS_API MLong GetFacialFeatures_Gabor(MHandle memHandle, const gvs_img_info* pImgInfo, gvs_region FaceRec, MPOINT* feapnt);

GVS_API MLong GVS_RotResize(MHandle memHandle, const LPASVLOFFSCREEN pSrcImgInfo,  gvs_img_info* pDstImgInfo, MLong RollAngle, gvs_region SrcFaceRec, puzzle_region* DstFaceRec, MLong* RotPara, MLong* ScalePara);
//int khas_motion(khas_handle KhasHandle,unsigned char *pBGR24Buff,khas_ani *pAni,khas_pnt *pPnts, khas_region FaceRec, int facepose);
// int khas_create(MHandle memHandle, khas_handle* pKhasHandle,int width,int height,WCHAR* model_file);
//void khas_release(MHandle memHandle, khas_handle* pKhasHandle);
#ifdef __cplusplus
}
#endif

#endif

