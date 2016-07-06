// ImageRGB.h: interface for the ImageRGB class.
// * History
// * 2014-04-23 Chen Yu (cy1969@arcsoft.com.cn)
// * - initial version
//////////////////////////////////////////////////////////////////////

#ifndef __IMAGERGB_H__
#define __IMAGERGB_H__

typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned int  UInt32, DWord;

#ifndef OUT
#define OUT
#endif

#ifndef MAX
#define MAX(a,b)  ((a)<(b)?(b):(a))
#define MIN(a,b)  ((a)<(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(x)    ((x)<0?-(x):(x))
#endif

#include "windows.h"

typedef union RGB4
{
	struct {Byte b,g,r,a;};
	UInt32 rgb;
	RGB4(){r=g=b=a=0;}
	RGB4(int r, int g, int b){this->r=(Byte)r;this->g=(Byte)g;this->b=(Byte)b;this->a=0;}
}RGB4;
typedef union YUV4
{
	struct {Byte v,u,y,c;};
	UInt32 yuv;
	YUV4(){y=u=v=c=0;}
	YUV4(int y, int u, int v){this->y=(Byte)y;this->u=(Byte)u;this->v=(Byte)v;this->c=0;}
}YUV4;

#define BLACK      RGB4(0,0,0)
#define WHITE      RGB4(255,255,255)
#define GREEN      RGB4(0,255,0)
#define RED        RGB4(255,0,0)
#define BLUE       RGB4(0,0,255)
#define YELLOW     RGB4(255,255,0)
#define MAGENTA    RGB4(255,0,255)
#define AQUA       RGB4(0,255,255)

#define TOP_LEFT 0
#define BOTTOM_LEFT 1

#define CHAR_HEIGHT 7

#define IMG_ROTATE0   0 // 不旋转
#define IMG_ROTATE90  1 // 顺时针旋转90°
#define IMG_ROTATE180 2 // 顺时针旋转180°
#define IMG_ROTATE270 3 // 顺时针旋转270°（逆时针旋转90°）
#define IMG_FLIP180   4 // 水平翻转

#define IMG_ZOOM_NONE     0
#define IMG_ZOOM_BILINEAR 1
#define IMG_ZOOM_NN       2
#define IMG_ZOOM_AUTO     3

/* 8bit, color or not */
#define IMG_LOAD_UNCHANGED  -1
/* 8bit, gray */
#define IMG_LOAD_GRAYSCALE   0
/* ?, color */
#define IMG_LOAD_COLOR       1
/* any depth, ? */ 
#define IMG_LOAD_ANYDEPTH    2
/* ?, any color */
#define IMG_LOAD_ANYCOLOR    4

class ImageRGB
{
public:
	ImageRGB(void);
	ImageRGB(int width, int height, int channels=3);
	ImageRGB(const char* fileName, int loadType=IMG_LOAD_COLOR);
	~ImageRGB(void);
	ImageRGB* SetImageHeader(int width, int height, int channels=3, void* pData=0, int origin=0);
	ImageRGB* Resize(int width, int height, int channels=3);
	int SaveAsBMP(const char* fileName);
	int SaveImage(const char* fileName);
	RGB4 SetRGB(int x, int y, RGB4& rgb=BLACK);
	RGB4 GetRGB(int x, int y);
	int GetCharWidth(const char* ch);
	int GetCharHeight(){return CHAR_HEIGHT;}
	void SetChar(int left, int top, char* ch, RGB4& rgb=BLACK);
	ImageRGB* LoadImageRGB(const char* fileName, int loadType=IMG_LOAD_COLOR);
	ImageRGB* Rotate(int rotateType=IMG_ROTATE0);
	ImageRGB* Zoom(int newWidth=0, int newHeight=0, int zoomType=IMG_ZOOM_AUTO);
	ImageRGB* DrawLine(int x1, int y1, int x2, int y2, RGB4& rgb=BLACK);
	ImageRGB* DrawCircle(int cx, int cy, int radius, RGB4& rgb=BLACK);
	ImageRGB* DrawRect(int left, int top, int width, int height, RGB4& rgb=BLACK);
	ImageRGB* DrawCross(int cx, int cy, RGB4& rgb=BLACK);
// 	void Release();
// 	ImageRGB* ZoomImageNN(int newWidth, int newHeight);
// 	ImageRGB* ZoomImageBilinear(int newWidth, int newHeight);

	int width,height,widthBytes;
	int maxSize;
	int channels;
	int origin;
	Byte* pData;
};
typedef ImageRGB ImageRAW;

inline void SafeFree(ImageRGB*& pImgRGB)
{
	if(pImgRGB)
		delete pImgRGB;
	pImgRGB=0;
}

int IsFileExist(const char* pFullFileName);
int SaveAsBMP(const char* fileName, Byte* pData, int width, int height, int channel);
void SetRGB(ImageRGB* pImg, int x, int y, int r, int g, int b);
RGB4 SetRGB(ImageRGB* pImg, int x, int y, RGB4& color);
RGB4 GetRGB(ImageRGB* pImg, int x, int y);
void CleanImageRGB(ImageRGB* pImg, int r, int g, int b);
void FillCircle(ImageRGB* pImg, int cx, int cy, int radius, int r, int g, int b);
void FillCircle(ImageRGB* pImg, int cx, int cy, int radius, RGB4 rgb);
int GetNumWidth(int xValue);
int GetNumWidth(char* ch);
void SetChar(ImageRGB* pImgRGB, int left, int top, char* ch, Byte r, Byte g, Byte b);
// void zoomImageBilinear_RGB(const void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH);
// void zoomImageNN_RGB(void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH);
int ZoomImageBilinear(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg);
int ZoomImageNN(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg);
int ZoomImageRGB(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg, int zoomType=IMG_ZOOM_AUTO);

int CopyImageRGB(const ImageRGB* pSrcImg, RECT* pSrcRect, ImageRGB* pDstImg, RECT* pDstRect);
int CopyImageRGB(const ImageRGB* pSrcImg, ImageRGB* pDstImg);

#define IMG_CHANNEL_TYPE_RGB   0
#define IMG_CHANNEL_TYPE_ALPHA 1
int GetImageChannelFromRGBA(const ImageRGB* pSrcImg, ImageRGB* pDstImg, int type);

#endif

