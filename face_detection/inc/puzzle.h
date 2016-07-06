/*----------------------------------------------------------------------------------------------
*
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* copyright(c) 2007                                                 
*
* James Yan (jyan@arcsoft.com.cn)
*
*----------------------------------------------------------------------------------------------*/
#ifndef _PUZZLE_H_
#define _PUZZLE_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "gvs.h"
//#include "gwt.h"
//#include "geo.h"
//#include "_gvs_fdetector_guess.h"

// #ifndef MIN
// #define MIN(v1, v2) 	   ( ((v1) > (v2)) ? (v2) : (v1) )
// #endif
// 
// #ifndef MAX
// #define MAX(v1, v2) 	   ( ((v1) < (v2)) ? (v2) : (v1) )
// #endif	

//gvs regions data
//#define FaceMaxRegions 1
#define Puzzle_Landmarks 95


#include "amcomdef.h"
	//puzzle landmark struct
// struct _puzzle_landmark
// {
// 	MLong x;
// 	MLong y;
// };
// typedef struct _puzzle_landmark puzzle_landmark;

//puzzle region struct
// struct _puzzle_region
// {
// 	MLong x;
// 	MLong y;
// 	MLong w;
// 	MLong h;
// };
// typedef struct _puzzle_region puzzle_region;

// struct _region_landmark_list
// {	
// 	MLong          nRegions;
// 	puzzle_region   regions  [FaceMaxRegions];
// 	puzzle_landmark landmarks[FaceMaxRegions][Puzzle_Landmarks];
// };
//typedef struct _region_landmark_list region_landmark_list;



//MVoid GetFacialFeatures(MHandle memHandle, const gvs_img_info* pImgInfo, gvs_region FaceRec, MPOINT* feapnt);
//MVoid InitialPnt(region_landmark_list* pIn, puzzle_region FaceRec, MPOINT* fea_pnt/*, MPOINT* fea_pnt_gabor, MLong flag_gabor*/);
//MVoid Rect2Guess(gvs_region* pRect,gvs_landmark* pRet,const MLong* map_x,const MLong* map_y);
MVoid puzzle(puzzle_region* pRect,MPOINT* pRet);
//MLong GetFacialFeatures_Gabor(MHandle memHandle, const gvs_img_info* pImgInfo, gvs_region FaceRec, MPOINT* feapnt);       
//MVoid GetEyeLocation(MHandle memHandle, const gvs_img_info* pImgInfo, gvs_region FaceRec,MPOINT* EyeCenter, gvs_region* EyeRec);
//int RotateImg(const gvs_img_info* src, gvs_img_info* dst, MPOINT* EyeCenter, gvs_region FaceRec, MFloat* m);
//int RotateImg_YUV422(const gvs_img_info* src, gvs_img_info* dst, MPOINT* EyeCenter, gvs_region FaceRec, MFloat* m);
#ifdef __cplusplus
}
#endif

#endif