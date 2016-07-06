// ImageYUV.h: interface for the ImageYUV class.
// * History
// * 2014-04-23 Chen Yu (cy1969@arcsoft.com.cn)
// * - initial version
//////////////////////////////////////////////////////////////////////

#ifndef __IMAGEYUV_H__
#define __IMAGEYUV_H__

#ifndef ASVL_PAF_YUV

//	31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 

//													 R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B 
#define		ASVL_PAF_RGB16_B5G6R5		0x101
//													 X  R  R  R  R  R  G  G  G  G  G  B  B  B  B  B  
#define		ASVL_PAF_RGB16_B5G5R5		0x102
//													 X  X  X  X  R  R  R  R  G  G  G  G  B  B  B  B 
#define		ASVL_PAF_RGB16_B4G4R4		0x103
//													 T  R  R  R  R  R  G  G  G  G  G  B  B  B  B  B  
#define		ASVL_PAF_RGB16_B5G5R5T		0x104
//													 B  B  B  B  B  G  G  G  G  G  G  R  R  R  R  R  
#define		ASVL_PAF_RGB16_R5G6B5		0x105
//													 X  B  B  B  B  B  G  G  G  G  G  R  R  R  R  R   
#define		ASVL_PAF_RGB16_R5G5B5		0x106
//													 X  X  X  X  B  B  B  B  G  G  G  G  R  R  R  R  
#define		ASVL_PAF_RGB16_R4G4B4		0x107

//							 R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B 
#define		ASVL_PAF_RGB24_B8G8R8		0x201
//							 X	X  X  X	 X	X  R  R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B  B  
#define		ASVL_PAF_RGB24_B6G6R6		0x202
//							 X	X  X  X	 X	T  R  R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B  B  
#define		ASVL_PAF_RGB24_B6G6R6T		0x203
//							 B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R	R  R  R	 R	R  R  R 
#define		ASVL_PAF_RGB24_R8G8B8		0x204
//							 X	X  X  X	 X	X  B  B  B  B  B  B  G  G  G  G  G  G  R  R  R  R  R  R 
#define		ASVL_PAF_RGB24_R6G6B6		0x205
//          B0, G0, R0
#define     ASVL_PAF_RGB                ASVL_PAF_RGB24_B8G8R8

//	 X	X  X  X	 X	X  X  X	 R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B  
#define		ASVL_PAF_RGB32_B8G8R8		0x301
//	 A	A  A  A	 A	A  A  A	 R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B  
#define		ASVL_PAF_RGB32_B8G8R8A8		0x302
//	 X	X  X  X	 X	X  X  X	 B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R	R  R  R	 R	R  R  R  
#define		ASVL_PAF_RGB32_R8G8B8		0x303
//	 B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R	R  R  R	 R	R  R  R  A	A  A  A	 A	A  A  A
#define		ASVL_PAF_RGB32_A8R8G8B8		0x304

//			Y0, U0, V0																				
#define		ASVL_PAF_YUV				0x401
//			Y0, V0, U0																				
#define		ASVL_PAF_YVU				0x402
//			U0, V0, Y0																				
#define		ASVL_PAF_UVY				0x403
//			V0, U0, Y0																				
#define		ASVL_PAF_VUY				0x404

//			Y0, U0, Y1, V0																			
#define		ASVL_PAF_YUYV				0x501
//			Y0, V0, Y1, U0																			
#define		ASVL_PAF_YVYU				0x502
//			U0, Y0, V0, Y1																			
#define		ASVL_PAF_UYVY				0x503
//			V0, Y0, U0, Y1																			
#define		ASVL_PAF_VYUY				0x504
//			Y1, U0, Y0, V0																			
#define		ASVL_PAF_YUYV2				0x505
//			Y1, V0, Y0, U0																			
#define		ASVL_PAF_YVYU2				0x506
//			U0, Y1, V0, Y0																			
#define		ASVL_PAF_UYVY2				0x507
//			V0, Y1, U0, Y0																			
#define		ASVL_PAF_VYUY2				0x508

//8 bit Y plane followed by 8 bit 2x2 sub sampled U and V planes
#define		ASVL_PAF_I420				0x601
//8 bit Y plane followed by 8 bit 1x2 sub sampled U and V planes
#define		ASVL_PAF_I422V				0x602
//8 bit Y plane followed by 8 bit 2x1 sub sampled U and V planes
#define		ASVL_PAF_I422H				0x603
//8 bit Y plane followed by 8 bit U and V planes
#define		ASVL_PAF_I444				0x604
//8 bit Y plane followed by 8 bit 2x2 sub sampled V and U planes
#define		ASVL_PAF_YV12				0x605
//8 bit Y plane followed by 8 bit 1x2 sub sampled V and U planes	
#define		ASVL_PAF_YV16V				0x606
//8 bit Y plane followed by 8 bit 2x1 sub sampled V and U planes
#define		ASVL_PAF_YV16H				0x607
//8 bit Y plane followed by 8 bit V and U planes
#define		ASVL_PAF_YV24				0x608
//8 bit Y plane only
#define		ASVL_PAF_GRAY				0x701
#define     ASVL_PAF_RGN                0x702

//8 bit Y plane followed by 8 bit 2x2 sub sampled UV planes
#define		ASVL_PAF_NV12				0x801
//8 bit Y plane followed by 8 bit 2x2 sub sampled VU planes
#define		ASVL_PAF_NV21				0x802
//8 bit Y plane followed by 8 bit 2x1 sub sampled UV planes
#define		ASVL_PAF_LPI422H			0x803
//8 bit Y plane followed by 8 bit 2x1 subsampled UV planes
#define		ASVL_PAF_NV16				ASVL_PAF_LPI422H
//8 bit Y plane followed by 8 bit 2x1 subsampled VU planes
#define		ASVL_PAF_NV61				0x804
//8 bit Y plane followed by 8 bit 1x1 subsampled UV planes
#define		ASVL_PAF_NV24				0x805
//8 bit Y plane followed by 8 bit 1x1 subsampled VU planes
#define		ASVL_PAF_NV42				0x806

#endif // ASVL_PAF_YUV

#define		ASVL_PAF_BMP    			0x901 
#define		ASVL_PAF_JPG    			0x902 
#define		ASVL_PAF_PNG    			0x903 

#define		ASVL_PAF_AVI    			0xA01 
#include "ImageRGB.h"

#ifndef OUT
#define OUT
#endif

// typedef union RGB4
// {
//     struct {Byte b,g,r,a;};
//     struct {Byte v,u,y,b;};
//     UInt32 value;
// }RGB4, YUV4;
// typedef struct __tag_ASVL_OFFSCREEN
// {
//     MUInt32	u32PixelArrayFormat;
//     MInt32	i32Width;
//     MInt32	i32Height;
//     MUInt8*	ppu8Plane[4];
//     MInt32	pi32Pitch[4];
// }ASVLOFFSCREEN, *LPASVLOFFSCREEN;

typedef struct YuvInfo
{
	YuvInfo();
	int alignBytes;
	int milliRunTime;
	int frameNum;
	int idxFrame;
	const char* fileName;
}YuvInfo;

class ImageYUV
{
public:
	ImageYUV();
	~ImageYUV();
	ImageYUV(int width, int height, int yuvFormat, int alignBytes=4);
	ImageYUV* Resize(int width, int height, int yuvFormat=0, int alignBytes=4);
	ImageYUV* SetImageHeader(int width, int height, int format, int alignBytes=4, void* pData=0);
	ImageYUV* ResetWidthBytes(int alignBytes=4);
	ImageYUV* LoadImageYUV(const char* fileName, int width=0, int height=0, int yuvFormat=0, OUT int* pAlignBytes=0, int idxFrame=0);
	int ConvertToRGB(OUT Byte* pRGB, int rgbWidthBytes, int channel=3);
	int GetFrameNum(); // 调用LoadImageYUV之后再调用这个函数获取帧数
	ImageYUV* GetFrameYUV(int idxFrame=0); // 调用LoadImageYUV之后再调用这个函数获取第n帧数据
	// int GuessAlignBytes();
	int SaveImage(const char* pBasePath, const char* pPrefix, int idx, const char* pSuffix, int isSaveAsVideo=0);
	// char* CreateFileName(const char* pBasePath, const char* pPrefix, int idx, const char* pSuffix, OUT char* fileName);
	YUV4 GetYUV(int x, int y, OUT RGB4* pRGB=0);

	int format;
	int width;
	int height;
	Byte* ppData[4];
	int pWidthBytes[4];
};
#define GetYuvInfo(pImgYUV) ((YuvInfo*)pImgYUV->ppData[3])

inline void SafeFree(ImageYUV*& pImgYUV)
{
	if(pImgYUV)
		delete pImgYUV;
	pImgYUV=0;
}

char* GetYuvExtenName(int yuvFormat);
char* CreateYuvFileName(int width, int height, int format, const char* pBasePath, const char* pPrefix, int idx, const char* pSuffix, OUT char* fileName);

int GetImageDataSize(int width, int height, int yuvFormat, int alignBytes);
ImageRGB* LoadAsImageRGB(const char* fileName, int channel=3);

int ZoomImageBilinear(ImageYUV* pSrcImgYUV, OUT ImageYUV* pDstImgYUV);
int ZoomImageNN(ImageYUV* pSrcImgYUV, OUT ImageYUV* pDstImgYUV);

ImageYUV* ConvertImageRGB2YUV(const ImageRGB* pImgRGB, OUT ImageYUV* pImgYUV=0);
ImageRAW* ConvertImageRGB2RAW(const ImageRGB* pImgRGB, OUT ImageRAW* pImgRAW=0);
ImageRGB* ConvertImageYUV2RGB(const ImageYUV* pImgYUV, OUT ImageRGB* pImgRGB=0);
ImageRAW* ConvertImageYUV2RAW(const ImageYUV* pImgYUV, OUT ImageRAW* pImgRAW=0);
ImageRGB* ConvertImageRAW2RGB(const ImageRAW* pImgRAW, OUT ImageRGB* pImgRGB=0);
ImageYUV* ConvertImageRAW2YUV(const ImageRAW* pImgRAW, OUT ImageYUV* pImgYUV=0);

int cvtYUV2RGB24(OUT Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV);
int cvtYUV2RGB32(OUT Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV);
int cvtRGB2YUV(Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, OUT Byte** ppYUV);
int cvtRaw2YUV(Byte* pRaw, int rawWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, OUT Byte** ppYUV);
int cvtYUV2Raw(OUT Byte* pRaw, int rawWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV);

#endif 