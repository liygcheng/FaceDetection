
#pragma warning(disable:4996)
#include "stdio.h"
#include "string.h"
// #include "Base.h"
// #include "windows.h"
#include "ImageRGB.h"
#include <cv.h>
#include <highgui.h>

#ifndef Assert
static void __assert(const char* expression, const char *fullPath, int lineNum)
{
    int len=0;
    char buf[256];
    while(fullPath[len]!=0)
        len++;
    while(len>=0 && fullPath[len]!='\\' && fullPath[len]!='/')
        len--;
    sprintf(buf, "File: %s\r\nLine: %d\r\n\r\n\tAssert Failed: %s\r\n\tDo you want abort\r\n",
        fullPath+len+1, lineNum, expression);
   // if(IDYES==MessageBox(0, buf, "AssertFailed", MB_YESNO))
    //    exit(-1);
}
#define Assert(x) (void)( (x) || (__assert(#x, __FILE__, __LINE__), 0) )
#define Alert(x) __assert(#x, __FILE__, __LINE__)
#endif

ImageRGB::ImageRGB(void)
{
	memset(this, 0, sizeof(ImageRGB));
	this->channels=3;
}
ImageRGB::ImageRGB(const char* fileName, int loadType)
{
	memset(this, 0, sizeof(ImageRGB));
	this->LoadImageRGB(fileName, loadType);
}
ImageRGB::~ImageRGB(void)
{
	if(this)
	{
		if(this->pData)
			delete[]this->pData;
		this->pData=0;
	}
}
#define GET_WITHBYTES(width, bitCount) (((width)*(bitCount)+31)/32*4)

ImageRGB::ImageRGB(int width, int height, int channels/* =3 */)
{
	Assert(width>0 && height>0 && 1<=channels && channels<=4);
	this->channels=channels;
	this->width=width;
	this->height=height;
	this->widthBytes=GET_WITHBYTES(width, channels*8);
	this->origin=TOP_LEFT;
	this->maxSize=this->widthBytes*height;
	this->pData=new Byte[this->maxSize];//(Byte*)malloc(this->maxSize*sizeof(Byte));
	memset(this->pData, 0xff, this->maxSize);
}
ImageRGB* ImageRGB::SetImageHeader(int width, int height, int channels/* =3 */, void* pData/* =0 */, int origin/* =0 */)
{
	this->width=width;
	this->height=height;
	this->channels=channels;
	this->widthBytes=GET_WITHBYTES(width, channels*8);
	this->maxSize=this->widthBytes*height;
	this->origin=origin;
	this->pData=(Byte*)pData;
	return this;
}
ImageRGB* ImageRGB::Resize(int width, int height, int channels/* =3 */)
{
	int widthBytes=GET_WITHBYTES(width, channels*8);
	int maxSize=widthBytes*height;
	Assert(width>0 && height>0 && 1<=channels && channels<=4);
	if(maxSize>this->maxSize || maxSize<this->maxSize/8)
	{
		if(this->pData)
			delete[]this->pData; // free(pImg->pData);
		this->pData=new Byte[maxSize]; // (Byte*)malloc(maxSize*sizeof(Byte));
		this->maxSize=maxSize;
	}
	this->width=width;
	this->height=height;
	this->widthBytes=widthBytes;
	this->channels=channels;
	return this;
}
ImageRGB* ImageRGB::LoadImageRGB(const char* fileName, int loadType)
{
	Assert(fileName && fileName[0]);
	if(fileName && fileName[0])
	{
		IplImage* pImg=cvLoadImage(fileName, loadType);
		Assert(pImg);
		if(pImg)
		{
// TIME_DECLARE();
// TIME_BEGIN();
			Resize(pImg->width, pImg->height, pImg->nChannels);
			int y;
			Byte *pDst=this->pData, *pSrc=(Byte*)pImg->imageData;
			for(y=0; y<pImg->height; y++, pDst+=this->widthBytes, pSrc+=pImg->widthStep)
				memcpy(pDst, pSrc, pImg->width*pImg->nChannels);
			cvReleaseImage(&pImg);
// TIME_END("LoadImageRGB");
			return this;
		}
	}
	return 0;
}
int ImageRGB::SaveAsBMP(const char* fileName)
{
	return ::SaveAsBMP(fileName, pData, width, height, channels);
}
static IplImage* setIplImageHeader(IplImage* image, Byte* pData, int width, int height, int channel, int origin)
{
    memset( image, 0, sizeof(*image) );
    image->nSize = sizeof(*image);
    
    image->width = width;
    image->height =height;
    
    image->nChannels = channel;
    image->depth = 8;
    image->widthStep = (width*channel+3)/4*4;
    image->origin = origin;
    image->imageSize = image->widthStep * image->height;
    image->imageData=(char*)pData;
    
    return image;
}
ImageRGB* ImageRGB::DrawLine(int x1, int y1, int x2, int y2, RGB4& rgb/* =BLACK */)
{
	if(this && this->pData)
	{
		IplImage img;
		setIplImageHeader(&img, this->pData, this->width, this->height, this->channels, this->origin);
		cvLine(&img, cvPoint(x1, y1), cvPoint(x2, y2), CV_RGB(rgb.r, rgb.g, rgb.b), 1, 8, 0);
		return this;
	}
	return 0;
}
ImageRGB* ImageRGB::DrawCircle(int cx, int cy, int radius, RGB4& rgb/* =BLACK */)
{
	if(this && this->pData)
	{
		IplImage img;
		setIplImageHeader(&img, this->pData, this->width, this->height, this->channels, this->origin);
		cvCircle(&img, cvPoint(cx, cy), radius, CV_RGB(rgb.r, rgb.g, rgb.b), 1, 8, 0);
		return this;
	}
	return 0;
}
ImageRGB* ImageRGB::DrawRect(int left, int top, int width, int height, RGB4& rgb/* =BLACK */)
{
	if(this && this->pData)
	{
		IplImage img;
		setIplImageHeader(&img, this->pData, this->width, this->height, this->channels, this->origin);
		cvRectangle(&img, cvPoint(left, top), cvPoint(left+width, top+height), CV_RGB(rgb.r, rgb.g, rgb.b), 1, 8, 0);
		return this;
	}
	return 0;
}
ImageRGB* ImageRGB::DrawCross(int cx, int cy, RGB4& rgb/* =BLACK */)
{
	if(this && this->pData)
	{
		SetRGB(cx-1, cy, rgb); SetRGB(cx+1, cy, rgb);
		SetRGB(cx, cy-1, rgb); SetRGB(cx, cy+1, rgb);
		SetRGB(cx, cy, rgb);
		return this;
	}
	return 0;
}
int ImageRGB::SaveImage(const char* fileName)
{
	if(this && this->pData)
	{
		IplImage img;
		setIplImageHeader(&img, pData, width, height, channels, origin);
		cvSaveImage(fileName, &img);
		return 1;
	}
	return 0;
}
RGB4 ImageRGB::SetRGB(int x, int y, RGB4& color/*=BLACK*/)
{
	if(0<=x && x<this->width && 0<=y && y<this->height)
    {
        if(this->channels==1)
            this->pData[y*this->widthBytes+x]=(color.r+color.g+color.b)/3;
        else if(this->channels==3)
        {
            Byte* pData=this->pData+y*this->widthBytes+3*x;
            pData[0]=color.b;
            pData[1]=color.g;
            pData[2]=color.r;
        }
    }
    return color;
}
RGB4 ImageRGB::GetRGB(int x, int y)
{
	RGB4 color;
    color.rgb=0;
    if(0<=x && x<this->width && 0<=y && y<this->height)
    {
        if(this->channels==1)
            (color.r=color.g=color.b=this->pData[y*this->widthBytes+x], color.a=0);
        else if(this->channels==3)
        {
            Byte* pData=this->pData+y*this->widthBytes+3*x;
            color.b=pData[0];
            color.g=pData[1];
            color.r=pData[2];
            color.a=0;
        }
    }
    return color;
}
int ImageRGB::GetCharWidth(const char* ch)
{
	return GetNumWidth((char*)ch);
}
void ImageRGB::SetChar(int left, int top, char* ch, RGB4& rgb/*=BLACK*/)
{
	::SetChar(this, left, top, ch, rgb.r, rgb.g, rgb.b);
}
inline RECT* setRect(RECT* pRect, int left, int top, int right, int bottom, int width, int height)
{
	pRect->left=MAX(0, left);
	pRect->top=MAX(0, top);
	pRect->right=MIN(right, width);
	pRect->bottom=MIN(bottom, height);
	return pRect;
}
inline void calcOverlapRect(RECT* pSrcRect, RECT* pDstRect)
{
	int srcW=pSrcRect->right-pSrcRect->left, srcH=pSrcRect->bottom-pSrcRect->top;
	int dstW=pDstRect->right-pDstRect->left, dstH=pDstRect->bottom-pDstRect->top;
	int rectW=MIN(srcW, dstW), rectH=MIN(srcH, dstH);
	pSrcRect->right=pSrcRect->left+rectW; pSrcRect->bottom=pSrcRect->top+rectH;
	pDstRect->right=pDstRect->left+rectW; pDstRect->bottom=pDstRect->top+rectH;
}
int CopyImageRGB(const ImageRGB* pSrcImg, RECT* pSrcRect, ImageRGB* pDstImg, RECT* pDstRect)
{
	int width=MIN(pSrcImg->width, pDstImg->width), height=MIN(pSrcImg->height, pDstImg->height);
	RECT srcRect={0, 0, width, height}, dstRect={0, 0, width, height};
	if(pSrcRect)
		setRect(&srcRect, pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom, pSrcImg->width, pSrcImg->height);
	if(pDstRect)
		setRect(&dstRect, pDstRect->left, pDstRect->top, pDstRect->right, pDstRect->bottom, pDstImg->width, pDstImg->height);
	calcOverlapRect(&srcRect, &dstRect);

	Assert(pSrcImg->channels==pDstImg->channels);
	int channels=pSrcImg->channels;
	int rectW=srcRect.right-srcRect.left, rectH=srcRect.bottom-srcRect.top;
	Byte *pSrcRGB=(Byte*)pSrcImg->pData+srcRect.top*pSrcImg->widthBytes+srcRect.left*channels;
	Byte *pDstRGB=pDstImg->pData+dstRect.top*pDstImg->widthBytes+dstRect.left*channels;
	int y;
	for(y=0; y<rectH; y++, pSrcRGB+=pSrcImg->widthBytes, pDstRGB+=pDstImg->widthBytes)
	{
		memcpy(pDstRGB, pSrcRGB, rectW*channels);
	}
	return 1;
}
int CopyImageRGB(const ImageRGB* pSrcImg, ImageRGB* pDstImg)
{
	return CopyImageRGB(pSrcImg, 0, pDstImg, 0);
}
int GetImageChannelFromRGBA(const ImageRGB* pSrcImg, RECT* pSrcRect, ImageRGB* pDstImg, RECT* pDstRect, int type)
{
	int width=MIN(pSrcImg->width, pDstImg->width), height=MIN(pSrcImg->height, pDstImg->height);
	RECT srcRect={0, 0, width, height}, dstRect={0, 0, width, height};
	if(pSrcRect)
		setRect(&srcRect, pSrcRect->left, pSrcRect->top, pSrcRect->right, pSrcRect->bottom, pSrcImg->width, pSrcImg->height);
	if(pDstRect)
		setRect(&dstRect, pDstRect->left, pDstRect->top, pDstRect->right, pDstRect->bottom, pDstImg->width, pDstImg->height);
	calcOverlapRect(&srcRect, &dstRect);

	Assert(pSrcImg->channels==4);
	int channels=pSrcImg->channels;
	int rectW=srcRect.right-srcRect.left, rectH=srcRect.bottom-srcRect.top;
	Byte *pSrcRGB=(Byte*)pSrcImg->pData+srcRect.top*pSrcImg->widthBytes+srcRect.left*channels;
	Byte *pDstRGB=pDstImg->pData+dstRect.top*pDstImg->widthBytes+dstRect.left*channels;
	int x, y;
	Byte *pSrcRGBTmp;
	Byte *pDstRGBTmp;
	if (type == IMG_CHANNEL_TYPE_RGB)
	{
		for(y=0; y<rectH; y++, pSrcRGB+=pSrcImg->widthBytes, pDstRGB+=pDstImg->widthBytes)
		{
			pSrcRGBTmp = pSrcRGB;
			pDstRGBTmp = pDstRGB;
			for(x=0; x<rectW; x++, pSrcRGBTmp+=4, pDstRGBTmp+=3)
			{
				pDstRGBTmp[0] = pSrcRGBTmp[0];
				pDstRGBTmp[1] = pSrcRGBTmp[1];
				pDstRGBTmp[2] = pSrcRGBTmp[2];
			}
		}
	}
	else
	{
		for(y=0; y<rectH; y++, pSrcRGB+=pSrcImg->widthBytes, pDstRGB+=pDstImg->widthBytes)
		{
			pSrcRGBTmp = pSrcRGB;
			pDstRGBTmp = pDstRGB;
			for(x=0; x<rectW; x++, pSrcRGBTmp+=4, pDstRGBTmp++)
			{
				pDstRGBTmp[0] = pSrcRGBTmp[3];
			}
		}
	}
	return 1;
}
int GetImageChannelFromRGBA(const ImageRGB* pSrcImg, ImageRGB* pDstImg, int type)
{
	return GetImageChannelFromRGBA(pSrcImg, 0, pDstImg, 0, type);
}

void SetRGB(ImageRGB* pImg, int x, int y, int r, int g, int b)
{
    if(0<=x && x<pImg->width && 0<=y && y<pImg->height)
    {
        if(pImg->channels==1)
            pImg->pData[y*pImg->widthBytes+x]=(Byte)((r+g+b)/3);
        else if(pImg->channels==3)
        {
            Byte* pData=pImg->pData+y*pImg->widthBytes+3*x;
            pData[0]=(Byte)b;
            pData[1]=(Byte)g;
            pData[2]=(Byte)r;
        }
    }
}
RGB4 SetRGB(ImageRGB* pImg, int x, int y, RGB4& color)
{
    if(0<=x && x<pImg->width && 0<=y && y<pImg->height)
    {
        if(pImg->channels==1)
            pImg->pData[y*pImg->widthBytes+x]=(color.r+color.g+color.b)/3;
        else if(pImg->channels==3)
        {
            Byte* pData=pImg->pData+y*pImg->widthBytes+3*x;
            pData[0]=color.b;
            pData[1]=color.g;
            pData[2]=color.r;
        }
    }
    return color;
}
RGB4 GetRGB(ImageRGB* pImg, int x, int y)
{
    RGB4 color;
    color.rgb=0;
    if(0<=x && x<pImg->width && 0<=y && y<pImg->height)
    {
        if(pImg->channels==1)
            (color.r=color.g=color.b=pImg->pData[y*pImg->widthBytes+x], color.a=0);
        else if(pImg->channels==3)
        {
            Byte* pData=pImg->pData+y*pImg->widthBytes+3*x;
            color.b=pData[0];
            color.g=pData[1];
            color.r=pData[2];
            color.a=0;
        }
    }
    return color;
}

void CleanImageRGB(ImageRGB* pImg, int r, int g, int b)
{
    int x, y;
    for(y=0; y<pImg->height; y++)
    {
        for(x=0; x<pImg->width; x++)
            SetRGB(pImg, x, y, r, g, b);
    }
}
void FillCircle(ImageRGB* pImg, int cx, int cy, int radius, int r, int g, int b)
{
    int x,y,k;
    int widthBytes=pImg->widthBytes;
    int r1,g1,b1;
    Byte* pData=pImg->pData+(cy-radius)*widthBytes+3*(cx-radius);
    int yMin=MAX(0, cy-radius), yMax=MIN(pImg->height, cy+radius);
    int xMin=MAX(0, cx-radius), xMax=MIN(pImg->width, cx+radius);
    int radius2=radius*radius;
    for(y=yMin; y<=yMax; y++, pData+=widthBytes)
    {
        int dy=y-cy;
        for(x=xMin, k=0; x<=xMax; x++, k+=3)
        {
            int dx=x-cx;
            if(dx*dx+dy*dy<=radius2)
            {
                r1=pData[k+2]; g1=pData[k+1]; b1=pData[k+0];
                if(r1!=255 || g1!=255 || b1!=255)
                {
                    pData[k+0]=MAX(b,b1);
                    pData[k+1]=MAX(g,g1);
                    pData[k+2]=MAX(r,r1);
                }
                else
                {
                    pData[k+0]=b;
                    pData[k+1]=g;
                    pData[k+2]=r;
                }
            }
        }
    }
}
void FillCircle(ImageRGB* pImg, int cx, int cy, int radius, RGB4 rgb)
{
	FillCircle(pImg, cx, cy, radius, rgb.r, rgb.g, rgb.b);
}
int GetNumWidth(int xValue)
{
	char xChar[64];
	int widthPerNum=3;
	int len=sprintf(xChar,"%d",xValue);
	return (1+widthPerNum)*len+1;
}
int GetNumWidth(char* ch)
{
    int len=strlen(ch);
    int widthPerNum=3;
    return (1+widthPerNum)*len+1;
}

static Byte gBackColorR=255, gBackColorG=255, gBackColorB=255;

static void SetFitBackColor(Byte r, Byte g, Byte b)
{
    Byte lum=((r*306+g*601+b*117+512)>>10);
    if(lum>200)
    {
        gBackColorR=255-r;
        gBackColorG=255-g;
        gBackColorB=255-b;
    }
    else
    {
        gBackColorR=gBackColorG=gBackColorB=255;
    }
}

void SetChar(ImageRGB* pImgRGB, int left, int top, char* ch, Byte r, Byte g, Byte b)
{
	char numChar[][16]={"111101101101111","010010010010010","111001111100111",
		"111001111001111","101101111001001","111100111001111",
		"111100111101111","111001001001001",
		"111101111101111","111101111001111",
		"101001010100101","000000000000010","000000111000000", // % . -
		"000010111010000","000101010101000","001001010100100", // + * /
		"000000000000000", // ' '
		"010111101111101","110101110101110","011100100100011", // A B C
		"110101101101110","111100111100111","111100111100100", // D E F
		"111100101101111","101101111101101","111010010010111", // G H I
		"001001001101111","101101110101101","100100100100111", // J K L
		"101111111101101","101111111111101","010101101101010", // M N O
		"111101111100100","111101111001001","111101110101101", // P Q R
		"011100010001110","111010010010010","101101101101011", // S T U
		"101101101010010","101101111111101","101101010101101","101101010010010", // V W X Y
		"111001010100111","001010100010001","100010001010100", // Z < >
		"000111000111000","011010010010011","110010010010110"   //= [ ]
	};
	unsigned char index[256];
	int xWidth,xHeight=7,len,widthPerNum=3;
	int row,col,i,k,rowStart,colStart,rowEnd,colEnd;

	for(i=0;i<256;i++)
		index[i]=0;// 空白
	for(i=0;i<10;i++)
		index[i+'0']=i;
	index['%']=10;
	index['.']=11;
	index['-']=12;
	index['+']=13;
	index['*']=14;
	index['/']=15;
	index[' ']=16;
	for(i='A'; i<='Z'; i++)
		index[i]=index['a'+i-'A']=i-'A'+17;
	index['<']=17+26;
	index['>']=18+26;
	index['=']=19+26;
	index['[']=20+26;
	index[']']=21+26;
	len=strlen(ch);
// 	int xWidth,xHeight=7,len,widthPerNum=3;
// 	int row,col,i,k,rowStart,colStart,rowEnd,colEnd;
// 	char xChar[64];
// 	len=sprintf(xChar,"%d",xValue);
	xWidth=(1+widthPerNum)*len+1;
    SetFitBackColor(r, g, b);
	for(row=top; row<top+xHeight; row++)
	{
		for(col=left; col<left+xWidth; col++)
			SetRGB(pImgRGB,col,row,gBackColorR,gBackColorG,gBackColorB); // set the background as white
	}
	rowStart=top+1;
	rowEnd=top+xHeight-1;
	for(i=0; i<len; i++)
	{
		colStart=left+(1+widthPerNum)*i+1;
		colEnd=colStart+widthPerNum;
		for(row=rowStart; row<rowEnd; row++)
		{
			for(col=colStart; col<colEnd; col++)
			{
                if(pImgRGB->origin==TOP_LEFT)
                    k=widthPerNum*(row-rowStart)+(col-colStart);
                else k=widthPerNum*(rowEnd-1-row)+(col-colStart); 
				if(numChar[index[((unsigned char*)ch)[i]]][k]=='1')
					SetRGB(pImgRGB,col,row,r,g,b);
			}
		}
	}
}
#define FIXED_SHIFT 10
typedef struct Fraction
{
	int sx;
	int dx;
}Fraction;
inline void initFixedTable(int srcW, int dstW, Fraction* xMap, int cn)
{
	int x;
	int dstW2=dstW*2;
	Assert(srcW>=2);
	for(x=0; x<dstW; x++)
	{
		int t, sx, dx;
// 		t=x*srcW;
// 		sx=t/dstW;
// 		dx=((t%dstW)<<FIXED_SHIFT)/dstW;
// 		if(sx>=srcW-1)
// 		{
// 			sx=srcW-2;
// 			dx=1<<FIXED_SHIFT;
// 		}
		t=(2*x+1)*srcW-dstW;
		sx=t/dstW2;
		dx=((t%dstW2)<<FIXED_SHIFT)/dstW2;
		if(t<0)
			dx=0, sx=0;
		else if(sx>=srcW-1)
		{
			sx=srcW-2;
			dx=1<<FIXED_SHIFT;
		}
		xMap[x].sx=sx*cn;
		xMap[x].dx=dx;
	}
}
#define SWAP(x, y, t) (t=x, x=y, y=t)
inline void calcBilinearLineGray(const Byte* pSrcY, int width, Fraction* xMap, OUT int* pLine)
{
	int x;
	for(x=0; x<width; x++)
	{
		const Byte* pSrcPix=pSrcY+xMap[x].sx;
		int dx=xMap[x].dx;
		pLine[x]=(pSrcPix[0]<<FIXED_SHIFT)+(pSrcPix[1]-pSrcPix[0])*dx;
	}
}
static void zoomImageBilinear_Gray(const void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
{
	const Byte *pSrcY=(const Byte*)pSrcData,*pSrcY0=pSrcY, *pSrcY1;
	Byte  *pDstY=(Byte*)pDstData;
	int xDst, yDst;
	int dy, yCurr, yPrev=-2;
	int bufSize=(dstW+dstH)*sizeof(Fraction)+dstW*2*sizeof(int);
	char *pBuf=new char[bufSize];
	Fraction *xMap=(Fraction*)pBuf, *yMap=xMap+dstW;
	int *pBuf0=(int*)(yMap+dstH), *pBuf1=pBuf0+dstW, *pTemp;
	// TIME_DECLARE();
	// TIME_BEGIN();
	initFixedTable(srcW, dstW, xMap, 1);
	initFixedTable(srcH, dstH, yMap, 1);
	// TIME_END("initFixedTable");
	// TIME_BEGIN();
	for(yDst=0; yDst<dstH; yDst++, pDstY+=dstWidthBytes)
	{
		dy=yMap[yDst].dx;
		yCurr=yMap[yDst].sx;
		pSrcY0=pSrcY+yCurr*srcWidthBytes;
		pSrcY1=pSrcY0+srcWidthBytes;
		if(yCurr==yPrev+1)
		{
			SWAP(pBuf0, pBuf1, pTemp);
			calcBilinearLineGray(pSrcY1, dstW, xMap, pBuf1);
		}
		else if(yCurr!=yPrev)
		{
			calcBilinearLineGray(pSrcY0, dstW, xMap, pBuf0);
			calcBilinearLineGray(pSrcY1, dstW, xMap, pBuf1);
		}
		for(xDst=0; xDst<dstW; xDst++)
		{
			pDstY[xDst]=(Byte)(((pBuf0[xDst]<<FIXED_SHIFT)+(pBuf1[xDst]-pBuf0[xDst])*dy)>>(2*FIXED_SHIFT));
		}
		yPrev=yCurr;
	}
	// TIME_END("Main loop");
	delete[]pBuf;
}
inline void calcBilinearLineRGB(const Byte* pSrcRGB, int width, Fraction* xMap, OUT int* pLine)
{
	int x, k;
	for(x=0, k=0; x<width; x++, k+=3)
	{
		const Byte* pSrcPix=pSrcRGB+xMap[x].sx;
		int dx=xMap[x].dx;
		pLine[k+0]=(pSrcPix[0]<<FIXED_SHIFT)+(pSrcPix[3]-pSrcPix[0])*dx;
		pLine[k+1]=(pSrcPix[1]<<FIXED_SHIFT)+(pSrcPix[4]-pSrcPix[1])*dx;
		pLine[k+2]=(pSrcPix[2]<<FIXED_SHIFT)+(pSrcPix[5]-pSrcPix[2])*dx;
	}
}
static void zoomImageBilinear_RGB(const void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
{
	const Byte *pSrcRGB=(const Byte*)pSrcData,*pSrcRGB0, *pSrcRGB1;
	Byte  *pDstRGB=(Byte*)pDstData;
	int yDst, k, dstW3=dstW*3;
	int dy, yPrev=-2, yCurr=0;
	int bufSize=(dstW+dstH)*sizeof(Fraction)+3*dstW*2*sizeof(int);
	char *pBuf=new char[bufSize];
	Fraction *xMap=(Fraction*)pBuf, *yMap=xMap+dstW;
	int *pBuf0=(int*)(yMap+dstH), *pBuf1=pBuf0+dstW*3, *pTemp=pBuf0;
// TIME_DECLARE();
// TIME_BEGIN();
	initFixedTable(srcW, dstW, xMap, 3);
	initFixedTable(srcH, dstH, yMap, 1);
// TIME_END("initFixedTable");
// TIME_BEGIN();
	for(yDst=0; yDst<dstH; yDst++, pDstRGB+=dstWidthBytes)
	{
		if(yDst==dstH-1)
			yDst=yDst;
		dy=yMap[yDst].dx;
		yCurr=yMap[yDst].sx;
		pSrcRGB0=pSrcRGB+yCurr*srcWidthBytes;
		pSrcRGB1=pSrcRGB0+srcWidthBytes;
		
		if(yCurr==yPrev+1)
		{
			SWAP(pBuf0, pBuf1, pTemp);
			calcBilinearLineRGB(pSrcRGB1, dstW, xMap, pBuf1);
		}
		else if(yCurr!=yPrev)
		{
			calcBilinearLineRGB(pSrcRGB0, dstW, xMap, pBuf0);
			calcBilinearLineRGB(pSrcRGB1, dstW, xMap, pBuf1);
		}
		for(k=0; k<dstW3; k++)
			pDstRGB[k]=(Byte)(((pBuf0[k]<<FIXED_SHIFT)+(pBuf1[k]-pBuf0[k])*dy)>>(2*FIXED_SHIFT));
		yPrev=yCurr;
	}
// TIME_END("Main loop");
	delete[]pBuf;
}
// void zoomImageBilinear_RGB(const void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
// {
// 	const Byte *pSrcRGB=(const Byte*)pSrcData,*pSrcRGB0=pSrcRGB, *pSrcRGB1, *pSrcPix;
// 	Byte  *pDstRGB=(Byte*)pDstData;
// 	int xDst, yDst, k, dstW3=dstW*3;
// 	int dx, dy, yPrev=0, yCurr=0;
// 	int bufSize=(dstW+dstH)*sizeof(Fraction)+3*dstW*2*sizeof(int);
// 	char *pBuf=new char[bufSize];
// 	Fraction *xMap=(Fraction*)pBuf, *yMap=xMap+dstW;
// 	int *pBuf0=(int*)(yMap+dstH), *pBuf1=pBuf0+dstW*3, *pTemp=pBuf0;
// 	// TIME_DECLARE();
// 	// TIME_BEGIN();
// 	initFixedTable(srcW, dstW, xMap, 3);
// 	initFixedTable(srcH, dstH, yMap, 1);
// 	// TIME_END("initFixedTable");
// 	// TIME_BEGIN();
// // 	for(xDst=0, k=0; xDst<dstW; xDst++, k+=3)
// // 	{
// // 		dx=xMap[xDst].dx;
// // 		pSrcPix=pSrcRGB0+xMap[xDst].sx;
// // 		pBuf0[k+0]=(pSrcPix[0]<<FIXED_SHIFT)+(pSrcPix[3]-pSrcPix[0])*dx;
// // 		pBuf0[k+1]=(pSrcPix[1]<<FIXED_SHIFT)+(pSrcPix[4]-pSrcPix[1])*dx;
// // 		pBuf0[k+2]=(pSrcPix[2]<<FIXED_SHIFT)+(pSrcPix[5]-pSrcPix[2])*dx;
// // 	}
// 	for(yDst=0; yDst<dstH; yDst++, pDstRGB+=dstWidthBytes)
// 	{
// 		dy=yMap[yDst].dx;
// 		yCurr=yMap[yDst].sx;
// 		pSrcRGB0=pSrcRGB+yCurr*srcWidthBytes;
// 		pSrcRGB1=pSrcRGB+(yCurr+1)*srcWidthBytes;
// 		for(xDst=0, k=0; xDst<dstW; xDst++, k+=3)
// 		{
// 			dx=xMap[xDst].dx;
// 			pSrcPix=pSrcRGB0+xMap[xDst].sx;
// 			pBuf0[k+0]=(pSrcPix[0]<<FIXED_SHIFT)+(pSrcPix[3]-pSrcPix[0])*dx;
// 			pBuf0[k+1]=(pSrcPix[1]<<FIXED_SHIFT)+(pSrcPix[4]-pSrcPix[1])*dx;
// 			pBuf0[k+2]=(pSrcPix[2]<<FIXED_SHIFT)+(pSrcPix[5]-pSrcPix[2])*dx;
// 			pSrcPix=pSrcRGB1+xMap[xDst].sx;
// 			pBuf1[k+0]=(pSrcPix[0]<<FIXED_SHIFT)+(pSrcPix[3]-pSrcPix[0])*dx;
// 			pBuf1[k+1]=(pSrcPix[1]<<FIXED_SHIFT)+(pSrcPix[4]-pSrcPix[1])*dx;
// 			pBuf1[k+2]=(pSrcPix[2]<<FIXED_SHIFT)+(pSrcPix[5]-pSrcPix[2])*dx;
// 		}
// 		for(k=0; k<dstW3; k++)
// 			pDstRGB[k]=(Byte)(((pBuf0[k]<<FIXED_SHIFT)+(pBuf1[k]-pBuf0[k])*dy)>>(2*FIXED_SHIFT));
// 		// SWAP(pBuf0, pBuf1, pTemp);
// 	}
// 	// TIME_END("Main loop");
// 	delete[]pBuf;
// }
// void zoomImageNN_Gray(void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
// {
// 	Byte *pSrcY=(Byte*)pSrcData, *pDstY=(Byte*)pDstData;
// 	Byte *pSrcY0;
// 	int x0, y0, xDst, yDst;
// 	double xScale=(double)srcW/dstW, yScale=(double)srcH/dstH;
// 	double fx, fy;
// 	for(yDst=0; yDst<dstH; yDst++, pDstY+=dstWidthBytes)
// 	{
// 		fy=yDst*yScale;
// 		y0=(int)(fy);
// 		pSrcY0=pSrcY+y0*srcWidthBytes;
// 		for(xDst=0; xDst<dstW; xDst++)
// 		{
// 			fx=xDst*xScale;
// 			x0=(int)(fx);
// 			pDstY[xDst]=pSrcY0[x0];
// 		}
// 	}
// }
static void zoomImageNN_Gray(void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
{
	Byte *pSrcY=(Byte*)pSrcData, *pDstY=(Byte*)pDstData;
	Byte *pSrcY0;
	int y0, xDst, yDst;
	int *xSrcMap=new int[dstW+1];
	for(xDst=0; xDst<dstW; xDst++)
	{
		int t=(srcW*xDst*2+MIN(srcW, dstW)-1)/(dstW*2);
		xSrcMap[xDst]=(t-(t>=srcW));
		// xSrcMap[xDst]=xDst*srcW/dstW*3;
	}
	for(yDst=0; yDst<dstH; yDst++, pDstY+=dstWidthBytes)
	{
		// y0=yDst*srcH/dstH;
		y0=(srcH*yDst*2+MIN(srcH, dstH)-1)/(dstH*2);
		y0=y0-(y0>=srcH);
		pSrcY0=pSrcY+y0*srcWidthBytes;
		for(xDst=0; xDst<dstW; xDst++)
		{
			pDstY[xDst]=pSrcY0[xSrcMap[xDst]];
		}
	}
	delete[]xSrcMap;
}
// void zoomImageNN_RGB(void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
// {
// 	Byte *pSrcRGB=(Byte*)pSrcData, *pDstRGB=(Byte*)pDstData;
// 	Byte *pSrcRGB0;
// 	int x0, y0, xDst, yDst, k0, k;
// 	double xScale=(double)srcW/dstW, yScale=(double)srcH/dstH;
// 	double fx, fy;
// 	for(yDst=0; yDst<dstH; yDst++, pDstRGB+=dstWidthBytes)
// 	{
// 		fy=yDst*yScale;
// 		y0=(int)(fy);
// 		pSrcRGB0=pSrcRGB+y0*srcWidthBytes;
// 		for(xDst=0, k=0; xDst<dstW; xDst++, k+=3)
// 		{
// 			fx=xDst*xScale;
// 			x0=(int)(fx);
// 			k0=3*x0;
// 			pDstRGB[k+0]=pSrcRGB0[k0+0];
// 			pDstRGB[k+1]=pSrcRGB0[k0+1];
// 			pDstRGB[k+2]=pSrcRGB0[k0+2];
// 		}
// 	}
// }
inline void calcLineNN_RGB(Byte* pSrcLine, int* xMap, int dstW, Byte* pDstLine)
{
	Byte buf[12];
	int x, k, k0, k1, k2, k3, dstW4=dstW/4*4;
	for(x=0, k=0; x<dstW4; x+=4, k+=3*4)
	{
		k0=xMap[x]; k1=xMap[x+1]; k2=xMap[x+2]; k3=xMap[x+3];
		buf[0]=pSrcLine[k0+0]; buf[1]=pSrcLine[k0+1]; buf[2]=pSrcLine[k0+2];
		buf[3]=pSrcLine[k1+0]; buf[4]=pSrcLine[k1+1]; buf[5]=pSrcLine[k1+2];
		buf[6]=pSrcLine[k2+0]; buf[7]=pSrcLine[k2+1]; buf[8]=pSrcLine[k2+2];
		buf[9]=pSrcLine[k3+0]; buf[10]=pSrcLine[k3+1]; buf[11]=pSrcLine[k3+2];
		// memcpy(pDstLine+k, buf, 12);
		*((unsigned int*)(pDstLine+k+0))=*((unsigned int*)buf+0);
		*((unsigned int*)(pDstLine+k+4))=*((unsigned int*)buf+4);
		*((unsigned int*)(pDstLine+k+8))=*((unsigned int*)buf+8);
	}
	for(x=dstW4; x<dstW; x++, k+=3)
	{
		k0=xMap[x];
		pDstLine[k+0]=pSrcLine[k0+0];
		pDstLine[k+1]=pSrcLine[k0+1];
		pDstLine[k+2]=pSrcLine[k0+2];
	}
}
#define AlignArray(pA, alignBytes, type) (type*)(((unsigned int)pA+(alignBytes)-1)/(alignBytes)*(alignBytes))
static void zoomImageNN_RGB(void* pSrcData, int srcWidthBytes, int srcW, int srcH, OUT void* pDstData, int dstWidthBytes, int dstW, int dstH)
{
	Byte *pSrcRGB=(Byte*)pSrcData, *pDstRGB=(Byte*)pDstData;
	Byte *pSrcRGB0;
	int y0, xDst, yDst;
	// int *_xSrcMap=new int[dstW+16*2+1];
	int *xSrcMap=new int[dstW+1]; //AlignArray(_xSrcMap, 16, int);
	for(xDst=0; xDst<dstW; xDst++)
	{
		int t=(srcW*xDst*2+MIN(srcW, dstW)-1)/(dstW*2);
		xSrcMap[xDst]=(3*(t-(t>=srcW)));
		// xSrcMap[xDst]=xDst*srcW/dstW*3;
	}
	for(yDst=0; yDst<dstH; yDst++, pDstRGB+=dstWidthBytes)
	{
		Byte* pDstPix=pDstRGB;
		// y0=yDst*srcH/dstH;
		y0=(srcH*yDst*2+MIN(srcH, dstH)-1)/(dstH*2);
		y0=y0-(y0>=srcH);
		pSrcRGB0=pSrcRGB+y0*srcWidthBytes;
		for(xDst=0; xDst<dstW; xDst++, pDstPix+=3)
		{
			Byte* pSrcPix=pSrcRGB0+xSrcMap[xDst];
			pDstPix[0]=pSrcPix[0];
			pDstPix[1]=pSrcPix[1];
			pDstPix[2]=pSrcPix[2];
		}
// 		calcLineNN_RGB(pSrcRGB0, xSrcMap, dstW, pDstRGB);
	}
	delete[]xSrcMap;
}
int ZoomImageBilinear(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg)
{
	if(pSrcImg->channels==1 && pDstImg->channels==1)
	{
		zoomImageBilinear_Gray(pSrcImg->pData, pSrcImg->widthBytes, pSrcImg->width, pSrcImg->height, 
			pDstImg->pData, pDstImg->widthBytes, pDstImg->width, pDstImg->height);
		return 1;
	}
	else if(pSrcImg->channels==3 && pDstImg->channels==3)
	{
		zoomImageBilinear_RGB(pSrcImg->pData, pSrcImg->widthBytes, pSrcImg->width, pSrcImg->height, 
			pDstImg->pData, pDstImg->widthBytes, pDstImg->width, pDstImg->height);
		return 3;
	}
	return 0;
}
int ZoomImageNN(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg)
{
	if(pSrcImg->channels==1 && pDstImg->channels==1)
	{
		zoomImageNN_Gray(pSrcImg->pData, pSrcImg->widthBytes, pSrcImg->width, pSrcImg->height, 
			pDstImg->pData, pDstImg->widthBytes, pDstImg->width, pDstImg->height);
		return 1;
	}
	else if(pSrcImg->channels==3 && pDstImg->channels==3)
	{
		zoomImageNN_RGB(pSrcImg->pData, pSrcImg->widthBytes, pSrcImg->width, pSrcImg->height, 
			pDstImg->pData, pDstImg->widthBytes, pDstImg->width, pDstImg->height);
		return 3;
	}
	return 0;
}
#define LIMITE_RATIO_NN 2
inline int isUseNN(int oldWidth, int oldHeight, int newWidth, int newHeight)
{
	if((newWidth%oldWidth==0 && newHeight%oldHeight==0) || (oldWidth%newWidth==0 && oldHeight%newHeight==0))
		return 1;
	if(oldWidth/newWidth>LIMITE_RATIO_NN && oldHeight/newHeight>LIMITE_RATIO_NN)
		return 1;
	return 0;
}
#define MAX_WIDTH  (1024*1024)
#define MAX_HEIGHT (1024*1024)
int ZoomImageRGB(ImageRGB* pSrcImg, OUT ImageRGB* pDstImg, int zoomType/* =IMG_ZOOM_AUTO */)
{
	if(pSrcImg==0 || pDstImg==0)
		return 0;
	int width=pSrcImg->width, height=pSrcImg->height, newWidth=pDstImg->width, newHeight=pDstImg->height;
	int channels=pSrcImg->channels;
	Assert(0<=newWidth && newWidth<=MAX_WIDTH && 0<=newHeight && newHeight<MAX_HEIGHT);
	Assert(zoomType==IMG_ZOOM_NONE || zoomType==IMG_ZOOM_BILINEAR || zoomType==IMG_ZOOM_NN || zoomType==IMG_ZOOM_AUTO);
	if(newWidth<=0 || newHeight<=0)
		return 0;
	if(zoomType==IMG_ZOOM_NONE || (newWidth==width && newHeight==height))
		CopyImageRGB(pSrcImg, 0, pDstImg, 0);
	if(zoomType==IMG_ZOOM_NN)
		ZoomImageNN(pSrcImg, pDstImg);
	else if(zoomType==IMG_ZOOM_BILINEAR)
		ZoomImageBilinear(pSrcImg, pDstImg);
	else if(zoomType==IMG_ZOOM_AUTO)
	{
		if(isUseNN(width, height, newWidth, newHeight))
			ZoomImageNN(pSrcImg, pDstImg);
		else ZoomImageBilinear(pSrcImg, pDstImg);
	}
	return 1;
}
static Byte* rotate90(Byte* pData, int widthBytes, int width, int height, int channels, OUT Byte* pDstData, int dstWidthBytes)
{
	int x, y, k;
	int dstWidth=height, dstHeight=width;
	// int dstWidthBytes=(height*channels+3)/4*4;
	Assert(channels==1 || channels==3);
	if(channels==1)
	{
		Byte *pSrcY, *pDstY=pDstData;
		for(y=0; y<dstHeight; y++, pDstY+=dstWidthBytes)
		{
			pSrcY=pData+(height-1)*widthBytes+y;
			for(x=0; x<dstWidth; x++, pSrcY-=widthBytes)
				pDstY[x]=pSrcY[0];
		}
	}
	else
	{
		Byte *pSrcRGB, *pDstRGB=pDstData;
		for(y=0; y<dstHeight; y++, pDstRGB+=dstWidthBytes)
		{
			pSrcRGB=pData+(height-1)*widthBytes+y*channels;
			for(x=0, k=0; x<dstWidth; x++, k+=3, pSrcRGB-=widthBytes)
				pDstRGB[k+0]=pSrcRGB[0], pDstRGB[k+1]=pSrcRGB[1], pDstRGB[k+2]=pSrcRGB[2];
		}
	}
	return pDstData;
}
static Byte* rotate180(Byte* pData, int widthBytes, int width, int height, int channels, OUT Byte* pDstData, int dstWidthBytes)
{
	int x, y, k;
	int dstWidth=width, dstHeight=height;
	// int dstWidthBytes=(height*channels+3)/4*4;
	Assert(channels==1 || channels==3);
	if(channels==1)
	{
		Byte *pSrcY, *pDstY=pDstData;
		for(y=0; y<dstHeight; y++, pDstY+=dstWidthBytes)
		{
			pSrcY=pData+(height-1-y)*widthBytes+(width-1);
			for(x=0; x<dstWidth; x++, pSrcY--)
				pDstY[x]=pSrcY[0];
		}
	}
	else
	{
		Byte *pSrcRGB, *pDstRGB=pDstData;
		for(y=0; y<dstHeight; y++, pDstRGB+=dstWidthBytes)
		{
			pSrcRGB=pData+(height-1-y)*widthBytes+(width-1)*channels;
			for(x=0, k=0; x<dstWidth; x++, k+=3, pSrcRGB-=3)
				pDstRGB[k+0]=pSrcRGB[0], pDstRGB[k+1]=pSrcRGB[1], pDstRGB[k+2]=pSrcRGB[2];
		}
	}
	return pDstData;
}
#define SWAP(a, b, t) (t=a, a=b, b=t)
static Byte* rotate180(Byte* pData, int widthBytes, int width, int height, int channels)
{
	int x, y, k, i;
	Assert(channels==1 || channels==3);
	if(channels==1)
	{
		Byte *pSrcY=pData+(height-1)*widthBytes, *pDstY=pData;
		Byte t;
		for(y=0; y<height/2; y++, pSrcY-=widthBytes, pDstY+=widthBytes)
		{
			for(x=0, i=width-1; x<width; x++, i--)
				SWAP(pSrcY[i], pDstY[x], t);
		}
		if(height%2==1)
		{
			pDstY=pData+(height/2)*widthBytes;
			for(x=0, i=width-1; x<width/2; x++, i--)
				SWAP(pDstY[x], pDstY[i], t);
		}
	}
	else
	{
		Byte *pSrcRGB=pData+(height-1)*widthBytes, *pDstRGB=pData;
		Byte t0, t1, t2;
		for(y=0; y<height/2; y++, pSrcRGB-=widthBytes, pDstRGB+=widthBytes)
		{
			for(x=0, k=0, i=(width-1)*3; x<width; x++, k+=3, i-=3)
			{
				SWAP(pSrcRGB[i+0], pDstRGB[k+0], t0);
				SWAP(pSrcRGB[i+1], pDstRGB[k+1], t1);
				SWAP(pSrcRGB[i+2], pDstRGB[k+2], t2);
			}
		}
		if(height%2==1)
		{
			pDstRGB=pData+(height/2)*widthBytes;
			for(x=0, k=0, i=(width-1)*3; x<width/2; x++, k+=3, i-=3)
			{
				SWAP(pDstRGB[i+0], pDstRGB[k+0], t0);
				SWAP(pDstRGB[i+1], pDstRGB[k+1], t1);
				SWAP(pDstRGB[i+2], pDstRGB[k+2], t2);
			}
		}
	}
	return pData;
}
static Byte* rotate270(Byte* pData, int widthBytes, int width, int height, int channels, OUT Byte* pDstData, int dstWidthBytes)
{
	int x, y, k;
	int dstWidth=height, dstHeight=width;
	// int dstWidthBytes=(height*channels+3)/4*4;
	Assert(channels==1 || channels==3);
	if(channels==1)
	{
		Byte *pSrcY, *pDstY=pDstData;
		for(y=0; y<dstHeight; y++, pDstY+=dstWidthBytes)
		{
			pSrcY=pData+(width-1-y);
			for(x=0; x<dstWidth; x++, pSrcY+=widthBytes)
				pDstY[x]=pSrcY[0];
		}
	}
	else
	{
		Byte *pSrcRGB, *pDstRGB=pDstData;
		for(y=0; y<dstHeight; y++, pDstRGB+=dstWidthBytes)
		{
			pSrcRGB=pData+(width-1-y)*channels;
			for(x=0, k=0; x<dstWidth; x++, k+=3, pSrcRGB+=widthBytes)
				pDstRGB[k+0]=pSrcRGB[0], pDstRGB[k+1]=pSrcRGB[1], pDstRGB[k+2]=pSrcRGB[2];
		}
	}
	return pDstData;
}
static Byte* flip(Byte* pData, int widthBytes, int width, int height)
{
	UInt32* pDataT=(UInt32*)pData;
	UInt32* pDataB=(UInt32*)(pData+widthBytes*(height-1));
	int halfHeight=height/2;
	int x,y;
	Assert(widthBytes%4==0);
	width=widthBytes/sizeof(UInt32);
	for(y=0; y<halfHeight; y++, pDataT+=width, pDataB-=width)
	{
		for(x=0; x<width; x++)
		{
			UInt32 temp=pDataT[x];
			pDataT[x]=pDataB[x];
			pDataB[x]=temp;
		}
	}
	return pData;
}
ImageRGB* ImageRGB::Rotate(int rotateType/* =IMG_ROTATE0 */)
{
	if(this==0 || pData==0)
		return 0;
	if(rotateType==IMG_ROTATE0)
		return this;
	if(rotateType==IMG_ROTATE90)
	{
		ImageRGB* pDstImg=new ImageRGB(height, width, channels);
		rotate90(pData, widthBytes, width, height, channels, pDstImg->pData, pDstImg->widthBytes);
		delete[] pData;
		*this=*pDstImg;
		pDstImg->pData=0; // 防止数据块被析构
		delete pDstImg;
	}
	else if(rotateType==IMG_ROTATE180)
	{
		rotate180(pData, widthBytes, width, height, channels);
	}
	else if(rotateType==IMG_ROTATE270)
	{
		ImageRGB* pDstImg=new ImageRGB(height, width, channels);
		rotate270(pData, widthBytes, width, height, channels, pDstImg->pData, pDstImg->widthBytes);
		delete[] pData;
		*this=*pDstImg;
		pDstImg->pData=0; // 防止数据块被析构
		delete pDstImg;
	}
	else if(rotateType==IMG_FLIP180)
	{
		flip(pData, widthBytes, width, height);
	}
	else
	{
		Alert("Do not support this rotate type");
	}
	return this;
}

ImageRGB* ImageRGB::Zoom(int newWidth, int newHeight, int zoomType/* =IMG_ZOOM_AUTO */)
{
	if(this==0 || pData==0 || width==0 || height==0)
		return 0;
	Assert(0<=newWidth && newWidth<=MAX_WIDTH && 0<=newHeight && newHeight<MAX_HEIGHT);
	Assert(zoomType==IMG_ZOOM_NONE || zoomType==IMG_ZOOM_BILINEAR || zoomType==IMG_ZOOM_NN || zoomType==IMG_ZOOM_AUTO);
	if(zoomType==IMG_ZOOM_NONE || newWidth<=0 || newHeight<=0 || (newWidth==width && newHeight==height))
		return this;
	ImageRGB* pDstImg=new ImageRGB(newWidth, newHeight, channels);
	if(zoomType==IMG_ZOOM_NN)
		ZoomImageNN(this, pDstImg);
	else if(zoomType==IMG_ZOOM_BILINEAR)
		ZoomImageBilinear(this, pDstImg);
	else if(zoomType==IMG_ZOOM_AUTO)
	{
		if(isUseNN(width, height, newWidth, newHeight))
			ZoomImageNN(this, pDstImg);
		else ZoomImageBilinear(this, pDstImg);
	}
	delete[] pData;
	*this=*pDstImg;
	pDstImg->pData=0;
	delete pDstImg;
	return this;
}
#include "io.h"
int IsFileExist(const char* pFullFileName)
{
	if(_access(pFullFileName, 0)!=0)
		return 0;
	return 1;
}

void* GetHBitmapData(HBITMAP hBmp, OUT SIZE* pSize, OUT int* pChannel)
{
	BITMAP bmp;
	GdiFlush();
	if(pSize) 
		pSize->cx=pSize->cy=0;
	if(hBmp==NULL)
		return 0;
	if(GetObject(hBmp, sizeof(bmp), &bmp)==0)
		return 0;
	if(pSize)
	{
		pSize->cx=ABS(bmp.bmWidth);
		pSize->cy=ABS(bmp.bmHeight);
	}
	if(pChannel)
		*pChannel=bmp.bmBitsPixel/8;
	return bmp.bmBits;
}

// static void FillBitmapInfo(BITMAPINFO* bmi, int width, int height, int bitCount, int origin)
// { 
// 	BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);
// 	// Assert( bmi && width >= 0 && height >= 0 && (bitCount == 8 || bitCount == 24 || bitCount == 32));
// 	memset( bmih, 0, sizeof(*bmih));
// 	bmih->biSize = sizeof(BITMAPINFOHEADER);
// 	bmih->biWidth = width;
// 	bmih->biHeight = origin ? ABS(height) : -ABS(height);
// 	bmih->biPlanes = 1;
// 	bmih->biBitCount = (unsigned short)bitCount;
// 	bmih->biCompression = BI_RGB;
// 
// 	if(bitCount==8)
// 	{
// 		RGBQUAD* palette=bmi->bmiColors;
// 		int i;
// 		for( i=0; i<256; i++ )
// 		{
// 			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
// 			palette[i].rgbReserved = 0;
// 		}
// 	}
// }

// HBITMAP CreateHBitmapByImageRGB(ImageRGB* pImgRGB)
// {
// 	HBITMAP hBmp;
// 	Byte* pData;
// 	unsigned char buf[sizeof(BITMAPINFO)+256*sizeof(RGBQUAD)];
// 	BITMAPINFO* pBmpInfo=(BITMAPINFO*)buf;
// 	FillBitmapInfo(pBmpInfo, pImgRGB->width, pImgRGB->height, pImgRGB->channels*8, pImgRGB->origin);
// 	hBmp=CreateDIBSection(0, pBmpInfo, DIB_RGB_COLORS, (void**)&pData, 0, 0);
// 	memcpy(pData, pImgRGB->pData, pImgRGB->widthBytes*pImgRGB->height);
// 	return hBmp;
// }
// HBITMAP CreateHBitmap(int width, int height, int channel/* =3 */, int origin/* =0 */)
// {
// 	HBITMAP hBmp;
// 	Byte* pData;
// 	unsigned char buf[sizeof(BITMAPINFO)+256*sizeof(RGBQUAD)];
// 	BITMAPINFO* pBmpInfo=(BITMAPINFO*)buf;
// 	FillBitmapInfo(pBmpInfo, width, height, channel*8, origin);
// 	hBmp=CreateDIBSection(0, pBmpInfo, DIB_RGB_COLORS, (void**)&pData, 0, 0);
// 	// memcpy(pData, pImgRGB->pData, pImgRGB->widthBytes*pImgRGB->height);
// 	return hBmp;
// }
// HBITMAP ResizeHBitmap(HBITMAP hBmp, int width, int height, int channel/* =3 */, int origin/* =0 */)
// {
// 	if(hBmp==0)
// 	{
// 		hBmp=CreateHBitmap(width, height, channel, origin);
// 	}
// 	else
// 	{
// 		SIZE size={0,0};
// 		int oldChannel=1;
// 		GetHBitmapData(hBmp, &size, &oldChannel);
// 		if(size.cx!=width || size.cy!=height || oldChannel!=channel)
// 		{
// 			DeleteObject(hBmp);
// 			hBmp=CreateHBitmap(width, height, channel, origin);
// 		}
// 	}
// 	return hBmp;
// }

// channel=3,4
int SaveAsBMP(const char* fileName, Byte* pData, int width, int height, int channel)
{
	FILE* fp;
	int widthBytes=(width*channel+3)/4*4;

	// BitmapFileHeader : 3*4+2=14
	Word  fileType='B'|('M'<<8);              // file type, where is 'BM'
	DWord fileSize=54+widthBytes*height;              // the size of the whole file
	Word  fReserved1=0,fReserved2=0; // the reserved value, current value is 0
	DWord dataOffBytes=54;          // the offset of image data from head of the file 

	// BitmapInfoHeader
	DWord structSize=40;   // sizeof(BitmapInfoHeader)
	Word  planes=1;       // this value must be set to 1
	Word  bitCount=channel*8;     // bits of bitmap, the value is 1,4,8,16,24,32
	DWord compression=0;  // the default value is 0 
	DWord dataSize=widthBytes*height;     // size of data domain
	int xPelsPerMeter=0,yPelsPerMeter=0; // the default value is 0
	DWord colorUsed=0;                   // the default value is 0, if colorUsed is not 0 or 2^bitCount, then the palette size is colorUsed*sizeof(DWord)
	DWord colorImportant=0;              // the default value is 0
	if(fileName==0 || pData==0 || width==0 || height==0)
		return 0;

	fp=fopen(fileName, "wb");
	if(fp==NULL)
	{
		// printf("Can not create '%s'\n",fileName);
		return 0;
	}
	fwrite(&fileType, sizeof(fileType), 1, fp);
	fwrite(&fileSize, sizeof(fileSize), 1, fp);
	fwrite(&fReserved1, sizeof(fReserved1), 2, fp);
	fwrite(&dataOffBytes, sizeof(dataOffBytes), 1, fp);
	fwrite(&structSize, sizeof(structSize), 1, fp);
	fwrite(&width, sizeof(width), 1, fp);
	fwrite(&height, sizeof(height), 1, fp);
	fwrite(&planes, sizeof(planes), 1, fp);
	fwrite(&bitCount, sizeof(bitCount), 1, fp);
	fwrite(&compression, sizeof(compression), 1, fp);
	fwrite(&dataSize, sizeof(dataSize), 1, fp);
	fwrite(&xPelsPerMeter, sizeof(xPelsPerMeter), 4, fp);
	flip(pData, widthBytes, width, height);
	fwrite(pData, sizeof(Byte), dataSize, fp);
	flip(pData, widthBytes, width, height);
	fclose(fp);

	return 1;
}
