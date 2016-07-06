
#include "stdio.h"
#include "string.h"
#include "ImageYUV.h"
#include "windows.h"
// #include "Base.h"
// #include "UnicodeToAscii.h"

#pragma warning(disable:4996)

#ifndef WriteLog
#define WriteLog // (format, ...)  
#endif

#undef MessageBox
#define MessageBox MessageBoxA
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
    if(IDYES==MessageBox(0, buf, "AssertFailed", MB_YESNO))
        exit(-1);
}
#define Assert(x) (void)( (x) || (__assert(#x, __FILE__, __LINE__), 0) )
#define Alert(x) __assert(#x, __FILE__, __LINE__)
#endif


YuvInfo::YuvInfo()
{
	memset(this, 0, sizeof(YuvInfo));
}
ImageYUV::ImageYUV()
{
	memset(this, 0, sizeof(ImageYUV));
}
ImageYUV::~ImageYUV()
{
	if(this)
	{
		if(this->ppData[0])
			delete[]this->ppData[0];
		if(this->ppData[3])
		{
			YuvInfo* pYuvInfo=GetYuvInfo(this);
			delete pYuvInfo;
		}
	}
}

static int isHadPlane(int yuvFormat, int idxPlane)
{
	int highByte=(yuvFormat>>8)&0x00ff;
	//     int lowByte=yuvFormat&0x00ff;
	if(idxPlane==0)
		return 1;
	if(idxPlane==1)
	{
		if(highByte==6 || highByte==8)
			return 1;
		return 0;
	}
	if(idxPlane==2)
	{
		if(highByte==6)
			return 1;
		return 0;
	}
	return 0;
}
static int isHalfHeight(int yuvFormat, int idxPlane)
{
	int highByte=(yuvFormat>>8)&0x00ff;
	int lowByte=yuvFormat&0x00ff;
	if(idxPlane==0)
		return 0;
	if(idxPlane==1)
	{
		if((highByte==6 && (lowByte==1 || lowByte==2 || lowByte==5 || lowByte==6)) || (highByte==8 && lowByte<=2))
			return 1;
		return 0;
	}
	if(idxPlane==2)
	{
		if((highByte==6 && (lowByte==1 || lowByte==2 || lowByte==5 || lowByte==6)))
			return 1;
		return 0;
	}
	return 0;
}
#define MAX_ALIGNBYTES 256
#define GET_WIDTHBYTES(width, alignBytes) (((width)+(alignBytes)-1)/(alignBytes)*(alignBytes))
static int SetWidthBytes(int width, int yuvFormat, OUT int pWidthBytes[4], int alignBytes)
{
	int highByte=(yuvFormat>>8)&0x00ff, lowByte=yuvFormat&0x00ff;
	if(alignBytes>MAX_ALIGNBYTES)
		return 0;
	pWidthBytes[0]=GET_WIDTHBYTES(width, alignBytes);
	pWidthBytes[1]=pWidthBytes[2]=0;
	if(1<=highByte && highByte<=3)
		pWidthBytes[0]=GET_WIDTHBYTES(width*(highByte+1), alignBytes);
	else if(highByte==4)
		pWidthBytes[0]=GET_WIDTHBYTES(3*width, alignBytes);
	else if(highByte==5)
		pWidthBytes[0]=GET_WIDTHBYTES(width/2*4, alignBytes);
	else if(highByte==6)
	{
		if(lowByte==1 || lowByte==5)
			pWidthBytes[1]=pWidthBytes[2]=GET_WIDTHBYTES(width/2, alignBytes);
		else if(lowByte==2 || lowByte==6)
			pWidthBytes[1]=pWidthBytes[2]=GET_WIDTHBYTES(width, alignBytes);
		else if(lowByte==3 || lowByte==7)
			pWidthBytes[1]=pWidthBytes[2]=GET_WIDTHBYTES(width/2, alignBytes);
		else if(lowByte==4 || lowByte==8)
			pWidthBytes[1]=pWidthBytes[2]=GET_WIDTHBYTES(width, alignBytes);
	}
	else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
		pWidthBytes[1]=GET_WIDTHBYTES(width/2*2, alignBytes);
	else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
        pWidthBytes[1]=GET_WIDTHBYTES(width/2*2, alignBytes);
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
		pWidthBytes[1]=GET_WIDTHBYTES(width*2, alignBytes);
	if(yuvFormat==ASVL_PAF_RGN)
		pWidthBytes[0]=width*2;
	return 1;
}
ImageYUV* ImageYUV::SetImageHeader(int width, int height, int format, int alignBytes/* =4 */, void* pData/* =0 */)
{
	int i;
	memset(this, 0, sizeof(ImageYUV));
	this->width=width;
	this->height=height;
	this->format=format;
	SetWidthBytes(width, format, this->pWidthBytes, alignBytes);
	if(pData!=0)
	{
		Byte* pYUV=(Byte*)pData;
		for(i=0; i<3; i++)
		{
			if(!isHadPlane(format, i))
				break;
			this->ppData[i]=pYUV;
			if(isHalfHeight(format, i))
				pYUV+=this->pWidthBytes[i]*(height/2);
			else pYUV+=this->pWidthBytes[i]*height;
		}
	}
	this->pWidthBytes[3]=GetImageDataSize(width, height, format, alignBytes);
	return this;
}
ImageYUV* ImageYUV::ResetWidthBytes(int alignBytes/* =4 */)
{
	Byte* pYUV=this->ppData[0];
	int i, width=this->width, height=this->height, yuvForamt=this->format;
	if(SetWidthBytes(width, yuvForamt, this->pWidthBytes, alignBytes)==0)
		return 0;
	this->ppData[1]=this->ppData[2]=0;
	for(i=0; i<3; i++)
	{
		if(!isHadPlane(yuvForamt, i))
			break;
		this->ppData[i]=pYUV;
		if(isHalfHeight(yuvForamt, i))
			pYUV+=this->pWidthBytes[i]*(height/2);
		else pYUV+=this->pWidthBytes[i]*height;
	}
	return this;
}
ImageYUV* ImageYUV::Resize(int width, int height, int yuvFormat, int alignBytes/* =4 */)
{
	if(yuvFormat==0)
		yuvFormat=this->format;
	int maxDataSize=this->pWidthBytes[3];	
	int dataSize=GetImageDataSize(width, height, yuvFormat, MAX_ALIGNBYTES);
	if(this->ppData[0]==0 || dataSize>maxDataSize || dataSize<maxDataSize/4)
	{
		if(this->ppData[0])
			delete[]this->ppData[0];
		this->pWidthBytes[3]=dataSize;
		this->ppData[0]=new Byte[dataSize];//(Byte*)malloc(dataSize*sizeof(Byte));
	}
	if(this->ppData[3]==0)
	{
		YuvInfo* pYuvInfo=new YuvInfo();
		this->ppData[3]=(Byte*)pYuvInfo;
	}
	this->width=width;
	this->height=height;
	this->format=yuvFormat;
	this->ResetWidthBytes(alignBytes);
	return this;
}

int GetImageDataSize(int width, int height, int yuvFormat, int alignBytes)
{
	int pW[3]={0,0,0}, pH[3]={0,0,0};
	int highByte=(yuvFormat>>8)&0x00ff, lowByte=yuvFormat&0x00ff;
	pW[0]=GET_WIDTHBYTES(width, alignBytes);
	pH[0]=height;
	if(1<=highByte && highByte<=3)
		pW[0]=GET_WIDTHBYTES(width*(highByte+1), alignBytes);
	else if(highByte==4)
		pW[0]=GET_WIDTHBYTES(3*width, alignBytes);
	else if(highByte==5)
		pW[0]=GET_WIDTHBYTES(width/2*4, alignBytes);
	else if(highByte==6)
	{
		if(lowByte==1 || lowByte==5)
			pW[1]=pW[2]=GET_WIDTHBYTES(width/2, alignBytes), pH[1]=pH[2]=height/2;
		else if(lowByte==2 || lowByte==6)
			pW[1]=pW[2]=GET_WIDTHBYTES(width, alignBytes), pH[1]=pH[2]=height/2;
		else if(lowByte==3 || lowByte==7)
			pW[1]=pW[2]=GET_WIDTHBYTES(width/2, alignBytes), pH[1]=pH[2]=height;
		else if(lowByte==4 || lowByte==8)
			pW[1]=pW[2]=GET_WIDTHBYTES(width, alignBytes), pH[1]=pH[2]=height;
	}
	else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
		pW[1]=GET_WIDTHBYTES(width/2*2, alignBytes), pH[1]=height/2;
	else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
        pW[1]=GET_WIDTHBYTES(width/2*2, alignBytes), pH[1]=height;
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
		pW[1]=GET_WIDTHBYTES(width*2, alignBytes), pH[1]=height;
	if(yuvFormat==ASVL_PAF_RGN)
		pW[0]=width*2;
	return pW[0]*pH[0]+pW[1]*pH[1]+pW[2]*pH[2];
}
inline int isTypeYUV(int yuvFormat)
{
	int highByte=(yuvFormat>>8)&0x00ff, lowByte=yuvFormat&0x00ff;
	if(lowByte<=0)
		return 0;
	if(4<=highByte && highByte<=8)
	{
		if(highByte==4 && lowByte>4)
			return 0;
		if(highByte==5 && lowByte>8)
			return 0;
		if(highByte==6 && lowByte>8)
			return 0;
		if(highByte==7 && lowByte>2)
			return 0;
		if(highByte==8 && lowByte>6)
			return 0;
		return 1;
	}
	return 0;
}
inline int isTypeRGB(int yuvFormat)
{
	int highByte=(yuvFormat>>8)&0x00ff, lowByte=yuvFormat&0x00ff;
	if(lowByte<=0)
		return 0;
	if(1<=highByte && highByte<=3)
	{
		if(highByte==1 && lowByte>7)
			return 0;
		if(highByte==2 && lowByte>5)
			return 0;
		if(highByte==3 && lowByte>4)
			return 0;
		return 1;
	}
	return 0;
}
ImageYUV::ImageYUV(int width, int height, int yuvFormat, int alignBytes/* =4 */)
{
	memset(this, 0, sizeof(ImageYUV));
	if(yuvFormat!=ASVL_PAF_I444 && yuvFormat!=ASVL_PAF_GRAY && yuvFormat!=ASVL_PAF_RGN)
	{
		width=width/2*2;
		height=height/2*2;
	}
	this->width=width;
	this->height=height;
	this->format=yuvFormat;

// 	if(yuvFormat==ASVL_PAF_GRAY || yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24 || yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21
// 		|| yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12 || yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU ||
// 		yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2 || yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V
// 		|| yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H || yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY
// 		|| yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_RGB)
	if(isTypeYUV(yuvFormat) || isTypeRGB(yuvFormat))
	{
		this->pWidthBytes[3]=GetImageDataSize(width, height, yuvFormat, MAX_ALIGNBYTES)*sizeof(Byte);
		this->ppData[0]=new Byte[this->pWidthBytes[3]];// (Byte*)malloc(this->pWidthBytes[3]);
		//Assert(this->ppData[0]);
		// ResetImageYuvWidthBytes(pFrame, alignBytes);
		this->ResetWidthBytes(alignBytes);
	}
	YuvInfo* pYuvInfo=new YuvInfo();
	this->ppData[3]=(Byte*)pYuvInfo;
	pYuvInfo->alignBytes=alignBytes;
// 	else
// 	{
// 		Alert("CreateImageYUV: Can not support this YUV format\r\n");
// 		exit(-1);
// 	}
}

static const Byte pTabRound256[768]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 
92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 
132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 
170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 
246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
#ifndef ROUND
#define ROUND(x)       pTabRound256[(x)+256]
// #define ROUND(x) ((Byte)((x)<0?0:((x)>255?255:(x))))
#endif // ROUND
// #define RGB2Y(r, g, b) ROUND(((r)*306+(g)*601+(b)*117+512)>>10)
// #define RGB2U(b, y)    ROUND(((((b)-(y))*578+512)>>10)+128)
// #define RGB2V(r, y)    ROUND(((((r)-(y))*730+512)>>10)+128)
// #define RGB2YUV(r,g,b,y,u,v) (y=RGB2Y(r,g,b),u=RGB2U(b,y),v=RGB2V(r,y))
// #define YUV2R(y, v)    ROUND((((y)<<15)                   +45941*((v)-(1<<7))+(1<<14))>>15)
// #define YUV2G(y,u,v)   ROUND((((y)<<15)-11277*((u)-(1<<7))-23401*((v)-(1<<7))+(1<<14))>>15)
// #define YUV2B(y, u)    ROUND((((y)<<15)+58065*((u)-(1<<7))                   +(1<<14))>>15)
// #define YUV2RGB(y,u,v,r,g,b) (r=YUV2R(y,v),g=YUV2G(y,u,v),b=YUV2B(y,u))

#define SHIFT           12
#define SHIFT_FLOOR(x)  ((int)((x)*(1<<SHIFT)+0.5))
#define YR              SHIFT_FLOOR(0.299)
#define YG              SHIFT_FLOOR(0.587)
#define YB              SHIFT_FLOOR(0.114)
#define UB              SHIFT_FLOOR(0.564)
#define VR              SHIFT_FLOOR(0.713)
#define RGB2Y(r, g, b)  ROUND(((r)*YR+(g)*YG+(b)*YB+(1<<(SHIFT-1)))>>SHIFT)
#define RGB2U(b, y)     ROUND(((((b)-(y))*UB+(1<<(SHIFT-1)))>>SHIFT)+128)
#define RGB2V(r, y)     ROUND(((((r)-(y))*VR+(1<<(SHIFT-1)))>>SHIFT)+128)
#define RGB2YUV(r,g,b,y,u,v) (y=RGB2Y(r,g,b),u=RGB2U(b,y),v=RGB2V(r,y))

#define RV              SHIFT_FLOOR(1.4029) // SHIFT_FLOOR(1.402524544)
#define GU              SHIFT_FLOOR(0.3443) // SHIFT_FLOOR(0.344340136)
#define GV              SHIFT_FLOOR(0.7147) // SHIFT_FLOOR(0.714403473)
#define BU              SHIFT_FLOOR(1.7734) // SHIFT_FLOOR(1.773049645)
#define YUV2R(y, v)     ROUND((((y)<<SHIFT)             +RV*((v)-128)+(1<<(SHIFT-1)))>>SHIFT)
#define YUV2G(y,u,v)    ROUND((((y)<<SHIFT)-GU*((u)-128)-GV*((v)-128)+(1<<(SHIFT-1)))>>SHIFT)
#define YUV2B(y, u)     ROUND((((y)<<SHIFT)+BU*((u)-128)             +(1<<(SHIFT-1)))>>SHIFT)
#define YUV2RGB(y,u,v,r,g,b) (r=YUV2R(y,v),g=YUV2G(y,u,v),b=YUV2B(y,u))

#define IDX_Y0 0
#define IDX_U0 1
#define IDX_V0 2
#define IDX_Y1 3
#define IDX_U1 4
#define IDX_V1 5
#define Raw2RGB(pRaw, k, y, u, v) (y=pRaw[k+IDX_Y0], u=pRaw[k+IDX_U0], v=pRaw[k+IDX_V0], YUV2RGB(y, u, v, pRaw[k+2], pRaw[k+1], pRaw[k+0]))
#define RawToYUV(pRaw, k, y, u, v) (y=pRaw[k+IDX_Y0], u=pRaw[k+IDX_U0], v=pRaw[k+IDX_V0])
#define RawTo420Y(pRaw1, pRaw2, k, y00, y01, y10, y11) (y00=pRaw1[k+IDX_Y0], y01=pRaw1[k+IDX_Y1], y10=pRaw2[k+IDX_Y0], y11=pRaw2[k+IDX_Y1])
#define RawTo420U(pRaw1, pRaw2, k, u0) (u0=(pRaw1[k+IDX_U0]+pRaw1[k+IDX_U1]+pRaw2[k+IDX_U0]+pRaw2[k+IDX_U1]+2)/4)
#define RawTo420V(pRaw1, pRaw2, k, v0) (v0=(pRaw1[k+IDX_V0]+pRaw1[k+IDX_V1]+pRaw2[k+IDX_V0]+pRaw2[k+IDX_V1]+2)/4)
#define RawTo420(pRaw1, pRaw2, k, y00, y01, y10, y11, u0, v0) (RawTo420Y(pRaw1, pRaw2, k, y00, y01, y10, y11), RawTo420U(pRaw1, pRaw2, k, u0), RawTo420V(pRaw1, pRaw2, k, v0))
//8 bit Y plane followed by 8 bit 1x2 sub sampled U and V planes
#define RawTo422V(pRaw1, pRaw2, k, y0, y1, u0, v0) (y0=pRaw1[k+IDX_Y0], y1=pRaw2[k+IDX_Y0], u0=(pRaw1[k+IDX_U0]+pRaw2[k+IDX_U0]+1)/2, v0=(pRaw1[k+IDX_V0]+pRaw2[k+IDX_V0]+1)/2)
//8 bit Y plane followed by 8 bit 2x1 sub sampled U and V planes
#define RawTo422H(pRaw, k, y0, y1, u0, v0) (y0=pRaw[k+IDX_Y0], y1=pRaw[k+IDX_Y1], u0=(pRaw[k+IDX_U0]+pRaw[k+IDX_U1]+1)/2, v0=(pRaw[k+IDX_V0]+pRaw[k+IDX_V1]+1)/2)
// Note: pRGB is 3 channels image data

#define YUVToRaw(y, u, v, pRaw, k) (pRaw[k+IDX_Y0]=y, pRaw[k+IDX_U0]=u, pRaw[k+IDX_V0]=v)

// Note: pRGB is 3 channels image data
int cvtYUV2RGB24(OUT Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV) 
{
	int x, y, k, i;
	if(yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24)
	{
		Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
		if(yuvFormat==ASVL_PAF_YV24)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=3)
				YUV2RGB(pY[x], pU[x], pV[x], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
		}
	}
	else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pUV=ppYUV[1];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV21)
			(idxU=1, idxV=0);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1])
		{
			for(x=k=0; x<width; x+=2, k+=6)
			{
				YUV2RGB(pY1[x+0], pUV[x+idxU], pUV[x+idxV], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY1[x+1], pUV[x+idxU], pUV[x+idxV], pRGB1[k+5], pRGB1[k+4], pRGB1[k+3]);
				YUV2RGB(pY2[x+0], pUV[x+idxU], pUV[x+idxV], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
				YUV2RGB(pY2[x+1], pUV[x+idxU], pUV[x+idxV], pRGB2[k+5], pRGB2[k+4], pRGB2[k+3]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}

	else if(yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pU=ppYUV[1], *pV=ppYUV[2];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		if(yuvFormat==ASVL_PAF_YV12)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=i=0; x<width; x+=2, k+=6, i++)
			{
				YUV2RGB(pY1[x+0], pU[i], pV[i], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY1[x+1], pU[i], pV[i], pRGB1[k+5], pRGB1[k+4], pRGB1[k+3]);
				YUV2RGB(pY2[x+0], pU[i], pV[i], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
				YUV2RGB(pY2[x+1], pU[i], pV[i], pRGB2[k+5], pRGB2[k+4], pRGB2[k+3]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU 
		|| yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2)
	{
		Byte *pYUV=ppYUV[0];
		int x2, idxY0=1, idxY1=3, idxU0=0, idxV0=2;
		if(yuvFormat==ASVL_PAF_VYUY)
            (idxY0=1, idxY1=3, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV)
            (idxY0=0, idxY1=2, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU)
            (idxY0=0, idxY1=2, idxU0=3, idxV0=1);
        else if(yuvFormat==ASVL_PAF_UYVY2)
            (idxY0=3, idxY1=1, idxU0=0, idxV0=2);
        else if(yuvFormat==ASVL_PAF_VYUY2)
            (idxY0=3, idxY1=1, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV2)
            (idxY0=2, idxY1=0, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU2)
            (idxY0=2, idxY1=0, idxU0=3, idxV0=1);
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x+=2, k+=6)
			{
				x2=x*2;
				YUV2RGB(pYUV[x2+idxY0], pYUV[x2+idxU0], pYUV[x2+idxV0], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
				YUV2RGB(pYUV[x2+idxY1], pYUV[x2+idxU0], pYUV[x2+idxV0], pRGB[k+5], pRGB[k+4], pRGB[k+3]);
			}
		}
	}
	else if(yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pU=ppYUV[1], *pV=ppYUV[2];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		if(yuvFormat==ASVL_PAF_YV16V)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=0; x<width; x++, k+=3)
			{
				YUV2RGB(pY1[x], pU[x], pV[x], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY2[x], pU[x], pV[x], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H)
	{
		Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
		if(yuvFormat==ASVL_PAF_YV16H)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=i=0; x<width; x+=2, k+=6, i++)
			{
				YUV2RGB(pY[x+0], pU[i], pV[i], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
				YUV2RGB(pY[x+1], pU[i], pV[i], pRGB[k+5], pRGB[k+4], pRGB[k+3]);
			}
			pRGB+=rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY)
	{
		Byte *pYUV=ppYUV[0];
		int idxY=0, idxU=1, idxV=2;
		if(yuvFormat==ASVL_PAF_YVU)
			(idxV=1, idxU=2);
		else if(yuvFormat==ASVL_PAF_UVY)
			(idxU=0, idxV=1, idxY=2);
		else if(yuvFormat==ASVL_PAF_VUY)
			(idxV=0, idxU=1, idxY=2);
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=3)
				YUV2RGB(pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV], pRGB[k+2], pRGB[k+1], pRGB[k]);
		}
	}
	else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV61)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                YUV2RGB(pY[x+0], pUV[x+idxU], pUV[x+idxV], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
                YUV2RGB(pY[x+1], pUV[x+idxU], pUV[x+idxV], pRGB[k+5], pRGB[k+4], pRGB[k+3]);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV42)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
            {
				int x2=x<<1;
                YUV2RGB(pY[x+0], pUV[x2+idxU], pUV[x2+idxV], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_GRAY)
	{
		Byte* pY=ppYUV[0];
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=3)
				pRGB[k+2]=pRGB[k+1]=pRGB[k]=pY[x];
		}
	}
	else if(yuvFormat==ASVL_PAF_RGB)
	{
		Byte* pYUV=ppYUV[0];
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			memcpy(pRGB, pYUV, width*3);
		}
	}
	else
	{
		Alert("cvtYUV2RGB24: Do not support this YUV format\r\n");
		// exit(-1);
		return 0;
	}

	return 1;
}
// Note: pRGB is 4 channels image data
int cvtYUV2RGB32(OUT Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV) 
{
	int x, y, k, i;
	if(yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24)
	{
		Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
		if(yuvFormat==ASVL_PAF_YV24)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=4)
				YUV2RGB(pY[x], pU[x], pV[x], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
		}
	}
	else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pUV=ppYUV[1];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV21)
			(idxU=1, idxV=0);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1])
		{
			for(x=k=0; x<width; x+=2, k+=8)
			{
				YUV2RGB(pY1[x+0], pUV[x+idxU], pUV[x+idxV], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY1[x+1], pUV[x+idxU], pUV[x+idxV], pRGB1[k+6], pRGB1[k+5], pRGB1[k+4]);
				YUV2RGB(pY2[x+0], pUV[x+idxU], pUV[x+idxV], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
				YUV2RGB(pY2[x+1], pUV[x+idxU], pUV[x+idxV], pRGB2[k+6], pRGB2[k+5], pRGB2[k+4]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}

	else if(yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pU=ppYUV[1], *pV=ppYUV[2];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		if(yuvFormat==ASVL_PAF_YV12)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=i=0; x<width; x+=2, k+=8, i++)
			{
				YUV2RGB(pY1[x+0], pU[i], pV[i], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY1[x+1], pU[i], pV[i], pRGB1[k+6], pRGB1[k+5], pRGB1[k+4]);
				YUV2RGB(pY2[x+0], pU[i], pV[i], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
				YUV2RGB(pY2[x+1], pU[i], pV[i], pRGB2[k+6], pRGB2[k+5], pRGB2[k+4]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU 
		|| yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2)
	{
		Byte *pYUV=ppYUV[0];
		int x2, idxY0=1, idxY1=3, idxU0=0, idxV0=2;
		if(yuvFormat==ASVL_PAF_VYUY)
            (idxY0=1, idxY1=3, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV)
            (idxY0=0, idxY1=2, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU)
            (idxY0=0, idxY1=2, idxU0=3, idxV0=1);
        else if(yuvFormat==ASVL_PAF_UYVY2)
            (idxY0=3, idxY1=1, idxU0=0, idxV0=2);
        else if(yuvFormat==ASVL_PAF_VYUY2)
            (idxY0=3, idxY1=1, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV2)
            (idxY0=2, idxY1=0, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU2)
            (idxY0=2, idxY1=0, idxU0=3, idxV0=1);
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x+=2, k+=8)
			{
				x2=x<<1;
				YUV2RGB(pYUV[x2+idxY0], pYUV[x2+idxU0], pYUV[x2+idxV0], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
				YUV2RGB(pYUV[x2+idxY1], pYUV[x2+idxU0], pYUV[x2+idxV0], pRGB[k+6], pRGB[k+5], pRGB[k+4]);
			}
		}
	}
	else if(yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V)
	{
		Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
		Byte *pU=ppYUV[1], *pV=ppYUV[2];
		Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
		if(yuvFormat==ASVL_PAF_YV16V)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=0; x<width; x++, k+=4)
			{
				YUV2RGB(pY1[x], pU[x], pV[x], pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
				YUV2RGB(pY2[x], pU[x], pV[x], pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
			}
			pRGB1=pRGB2+rgbWidthBytes;
			pRGB2=pRGB1+rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H)
	{
		Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
		if(yuvFormat==ASVL_PAF_YV16H)
			(pU=ppYUV[2], pV=ppYUV[1]);
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
		{
			for(x=k=i=0; x<width; x+=2, k+=8, i++)
			{
				YUV2RGB(pY[x+0], pU[i], pV[i], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
				YUV2RGB(pY[x+1], pU[i], pV[i], pRGB[k+6], pRGB[k+5], pRGB[k+4]);
			}
			pRGB+=rgbWidthBytes;
		}
	}
	else if(yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY)
	{
		Byte *pYUV=ppYUV[0];
		int idxY=0, idxU=1, idxV=2;
		if(yuvFormat==ASVL_PAF_YVU)
			(idxV=1, idxU=2);
		else if(yuvFormat==ASVL_PAF_UVY)
			(idxU=0, idxV=1, idxY=2);
		else if(yuvFormat==ASVL_PAF_VUY)
			(idxV=0, idxU=1, idxY=2);
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=4)
				YUV2RGB(pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV], pRGB[k+2], pRGB[k+1], pRGB[k]);
		}
	}
	else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV61)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=8)
            {
                YUV2RGB(pY[x+0], pUV[x+idxU], pUV[x+idxV], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
                YUV2RGB(pY[x+1], pUV[x+idxU], pUV[x+idxV], pRGB[k+6], pRGB[k+5], pRGB[k+4]);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV42)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=4)
            {
				int x2=x<<1;
                YUV2RGB(pY[x+0], pUV[x2+idxU], pUV[x2+idxV], pRGB[k+2], pRGB[k+1], pRGB[k+0]);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_GRAY)
	{
		Byte* pY=ppYUV[0];
		for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=k=0; x<width; x++, k+=4)
				pRGB[k+2]=pRGB[k+1]=pRGB[k]=pY[x];
		}
	}
	else if(yuvFormat==ASVL_PAF_RGB)
	{
		Byte* pYUV=ppYUV[0];
		int i;
		for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
		{
			for(x=0, k=0, i=0; x<width; x++, k+=3, i+=4)
			{
				pRGB[i+0]=pYUV[k+0];
				pRGB[i+1]=pYUV[k+1];
				pRGB[i+2]=pYUV[k+2];
			}
		}
	}
	else
	{
		Alert("cvtYUV2RGB32: Do not support this YUV format\r\n");
		// exit(-1);
		return 0;
	}

	return 1;
}
int cvtRGB2YUV(Byte* pRGB, int rgbWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, OUT Byte** ppYUV) 
{
    int x, y, k, i;
    if(yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV24)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                RGB2YUV(pRGB[k+2], pRGB[k+1], pRGB[k], pY[x], pU[x], pV[x]);
        }
    }
    else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pUV=ppYUV[1];
        Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
        int idxU=0, idxV=1;
        if(yuvFormat==ASVL_PAF_NV21)
            (idxU=1, idxV=0);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1])
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                int yuvY,b,r;
                pY1[x+0]=RGB2Y(pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]); pY1[x+1]=RGB2Y(pRGB1[k+5], pRGB1[k+4], pRGB1[k+3]);
                pY2[x+0]=RGB2Y(pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]); pY2[x+1]=RGB2Y(pRGB2[k+5], pRGB2[k+4], pRGB2[k+3]);
                yuvY=(pY1[x]+pY1[x+1]+pY2[x]+pY2[x+1])/4;
                b=(pRGB1[k+0]+pRGB1[k+3]+pRGB2[k+0]+pRGB2[k+3])/4;
                r=(pRGB1[k+2]+pRGB1[k+5]+pRGB2[k+2]+pRGB2[k+5])/4;
                pUV[x+idxU]=RGB2U(b, yuvY);
                pUV[x+idxV]=RGB2V(r, yuvY);
            }
            pRGB1=pRGB2+rgbWidthBytes;
            pRGB2=pRGB1+rgbWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
        if(yuvFormat==ASVL_PAF_YV12)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
            {
                int yuvY,b,r;
                pY1[x+0]=RGB2Y(pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]); pY1[x+1]=RGB2Y(pRGB1[k+5], pRGB1[k+4], pRGB1[k+3]);
                pY2[x+0]=RGB2Y(pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]); pY2[x+1]=RGB2Y(pRGB2[k+5], pRGB2[k+4], pRGB2[k+3]);
                yuvY=(pY1[x]+pY1[x+1]+pY2[x]+pY2[x+1])/4;
                b=(pRGB1[k+0]+pRGB1[k+3]+pRGB2[k+0]+pRGB2[k+3])/4;
                r=(pRGB1[k+2]+pRGB1[k+5]+pRGB2[k+2]+pRGB2[k+5])/4;
                pU[i]=RGB2U(b, yuvY);
                pV[i]=RGB2V(r, yuvY);
            }
            pRGB1=pRGB2+rgbWidthBytes;
            pRGB2=pRGB1+rgbWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU 
        || yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2)
    {
        Byte *pYUV=ppYUV[0];
        int x2, idxY0=1, idxY1=3, idxU0=0, idxV0=2;
        if(yuvFormat==ASVL_PAF_VYUY)
            (idxY0=1, idxY1=3, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV)
            (idxY0=0, idxY1=2, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU)
            (idxY0=0, idxY1=2, idxU0=3, idxV0=1);
        else if(yuvFormat==ASVL_PAF_UYVY2)
            (idxY0=3, idxY1=1, idxU0=0, idxV0=2);
        else if(yuvFormat==ASVL_PAF_VYUY2)
            (idxY0=3, idxY1=1, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV2)
            (idxY0=2, idxY1=0, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU2)
            (idxY0=2, idxY1=0, idxU0=3, idxV0=1);

        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                int yuvY,b,r;
                x2=x<<1;
                pYUV[x2+idxY0]=RGB2Y(pRGB[k+2], pRGB[k+1], pRGB[k+0]); 
                pYUV[x2+idxY1]=RGB2Y(pRGB[k+5], pRGB[k+4], pRGB[k+3]);
                yuvY=(pYUV[x2+idxY0]+pYUV[x2+idxY1])>>1;
                b=(pRGB[k+0]+pRGB[k+3])>>1;
                r=(pRGB[k+2]+pRGB[k+5])>>1;
                pYUV[x2+idxU0]=RGB2U(b, yuvY);
                pYUV[x2+idxV0]=RGB2V(r, yuvY);
            }
        }
    }
    else if(yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRGB1=pRGB, *pRGB2=pRGB1+rgbWidthBytes;
        if(yuvFormat==ASVL_PAF_YV16V)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=0; x<width; x++, k+=3)
            {
                int yuvY,b,r;
                pY1[x]=RGB2Y(pRGB1[k+2], pRGB1[k+1], pRGB1[k+0]);
                pY2[x]=RGB2Y(pRGB2[k+2], pRGB2[k+1], pRGB2[k+0]);
                yuvY=(pY1[x]+pY2[x])/2;
                b=(pRGB1[k+0]+pRGB2[k+0])/2;
                r=(pRGB1[k+2]+pRGB2[k+2])/2;
                pU[x]=RGB2U(b, yuvY);
                pV[x]=RGB2V(r, yuvY);
            }
            pRGB1=pRGB2+rgbWidthBytes;
            pRGB2=pRGB1+rgbWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV16H)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
            {
                int yuvY,b,r;
                pY[x+0]=RGB2Y(pRGB[k+2], pRGB[k+1], pRGB[k+0]); 
                pY[x+1]=RGB2Y(pRGB[k+5], pRGB[k+4], pRGB[k+3]);
                yuvY=(pY[x+0]+pY[x+1])/2;
                b=(pRGB[k+0]+pRGB[k+3])/2;
                r=(pRGB[k+2]+pRGB[k+5])/2;
                pU[i]=RGB2U(b, yuvY);
                pV[i]=RGB2V(r, yuvY);
            }
            pRGB+=rgbWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY)
    {
        Byte *pYUV=ppYUV[0];
        int idxY=0, idxU=1, idxV=2;
        if(yuvFormat==ASVL_PAF_YVU)
            (idxV=1, idxU=2);
        else if(yuvFormat==ASVL_PAF_UVY)
            (idxU=0, idxV=1, idxY=2);
        else if(yuvFormat==ASVL_PAF_VUY)
            (idxV=0, idxU=1, idxY=2);
        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                RGB2YUV(pRGB[k+2], pRGB[k+1], pRGB[k], pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV]);
        }
    }
	else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV61)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                int yuvY,b,r;
                pY[x+0]=RGB2Y(pRGB[k+2], pRGB[k+1], pRGB[k+0]); pY[x+1]=RGB2Y(pRGB[k+5], pRGB[k+4], pRGB[k+3]);
                yuvY=(pY[x]+pY[x+1])/2;
                b=(pRGB[k+0]+pRGB[k+3])/2;
                r=(pRGB[k+2]+pRGB[k+5])/2;
                pUV[x+idxU]=RGB2U(b, yuvY);
                pUV[x+idxV]=RGB2V(r, yuvY);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_NV42 || yuvFormat==ASVL_PAF_NV24)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int x2, idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV42)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRGB+=rgbWidthBytes)
        {
            for(x=k=0, x2=0; x<width; x++, k+=3, x2+=2)
            {
                pY[x]=RGB2Y(pRGB[k+2], pRGB[k+1], pRGB[k+0]);
                pUV[x2+idxU]=RGB2U(pRGB[k+0], pY[x]);
                pUV[x2+idxV]=RGB2V(pRGB[k+2], pY[x]);
            }
        }
    }
    else if(yuvFormat==ASVL_PAF_GRAY)
    {
        Byte* pY=ppYUV[0];
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pRGB+=rgbWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                pY[x]=RGB2Y(pRGB[k+2], pRGB[k+1], pRGB[k+0]);
        }
    }
    else
    {
        Alert("cvtRGB2YUV: Do not support this YUV format\r\n");
        // exit(-1);
		return 0;
    }
    return 1;
}
int cvtRaw2YUV(Byte* pRaw, int rawWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, OUT Byte** ppYUV) 
{
    int x, y, k, i;
    if(yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV24)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                RawToYUV(pRaw, k, pY[x], pU[x], pV[x]);
                // RGB2YUV(pRGB[k+2], pRGB[k+1], pRGB[k], pY[x], pU[x], pV[x]);
        }
    }
    else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pUV=ppYUV[1];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        int idxU=0, idxV=1;
        if(yuvFormat==ASVL_PAF_NV21)
            (idxU=1, idxV=0);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1])
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                RawTo420(pRaw1, pRaw2, k, pY1[x+0], pY1[x+1], pY2[x+0], pY2[x+1], pUV[x+idxU], pUV[x+idxV]);
            }
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        if(yuvFormat==ASVL_PAF_YV12)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
                RawTo420(pRaw1, pRaw2, k, pY1[x+0], pY1[x+1], pY2[x+0], pY2[x+1], pU[i], pV[i]);
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU 
        || yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2)
    {
        Byte *pYUV=ppYUV[0];
        int x2, idxY0=1, idxY1=3, idxU0=0, idxV0=2;
        if(yuvFormat==ASVL_PAF_VYUY)
            (idxY0=1, idxY1=3, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV)
            (idxY0=0, idxY1=2, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU)
            (idxY0=0, idxY1=2, idxU0=3, idxV0=1);
        else if(yuvFormat==ASVL_PAF_UYVY2)
            (idxY0=3, idxY1=1, idxU0=0, idxV0=2);
        else if(yuvFormat==ASVL_PAF_VYUY2)
            (idxY0=3, idxY1=1, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV2)
            (idxY0=2, idxY1=0, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU2)
            (idxY0=2, idxY1=0, idxU0=3, idxV0=1);

        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=x2=0; x<width; x+=2, x2+=4, k+=6)
                RawTo422H(pRaw, k, pYUV[x2+idxY0], pYUV[x2+idxY1], pYUV[x2+idxU0], pYUV[x2+idxV0]);
        }
    }
    else if(yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        if(yuvFormat==ASVL_PAF_YV16V)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=0; x<width; x++, k+=3)
                RawTo422V(pRaw1, pRaw2, k, pY1[x], pY2[x], pU[x], pV[x]);
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV16H)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
                RawTo422H(pRaw, k, pY[x+0], pY[x+1], pU[i], pV[i]);
            pRaw+=rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY)
    {
        Byte *pYUV=ppYUV[0];
        int idxY=0, idxU=1, idxV=2;
        if(yuvFormat==ASVL_PAF_YVU)
            (idxV=1, idxU=2);
        else if(yuvFormat==ASVL_PAF_UVY)
            (idxU=0, idxV=1, idxY=2);
        else if(yuvFormat==ASVL_PAF_VUY)
            (idxV=0, idxU=1, idxY=2);
        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                RawToYUV(pRaw, k, pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV]);
                // RGB2YUV(pRGB[k+2], pRGB[k+1], pRGB[k], pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV]);
        }
    }
    else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV61)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                RawTo422H(pRaw, k, pY[x+0], pY[x+1], pUV[x+idxU], pUV[x+idxV]);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int x2, idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV42)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRaw+=rawWidthBytes)
        {
            for(x=k=x2=0; x<width; x++, k+=3, x2+=2)
            {
                RawToYUV(pRaw, k, pY[x], pUV[x2+idxU], pUV[x2+idxV]);
            }
        }
    }
    else if(yuvFormat==ASVL_PAF_GRAY)
    {
        Byte* pY=ppYUV[0];
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                pY[x]=pRaw[k+IDX_Y0];
        }
    }
    else
    {
        Alert("cvtRaw2YUV: Do not support this YUV format\r\n");
        // exit(-1);
		return 0;
    }
    return 1;
}
int cvtYUV2Raw(OUT Byte* pRaw, int rawWidthBytes, int width, int height, int yuvFormat, int *pYuvWidthBytes, Byte** ppYUV) 
{
    int x, y, k, i;
    if(yuvFormat==ASVL_PAF_I444 || yuvFormat==ASVL_PAF_YV24)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV24)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                YUVToRaw(pY[x], pU[x], pV[x], pRaw, k);
                // YUV2RGB(pY[x], pU[x], pV[x], pRaw[k+2], pRaw[k+1], pRaw[k+0]);
        }
    }
    else if(yuvFormat==ASVL_PAF_NV12 || yuvFormat==ASVL_PAF_NV21)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pUV=ppYUV[1];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        int idxU=0, idxV=1;
        if(yuvFormat==ASVL_PAF_NV21)
            (idxU=1, idxV=0);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1])
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                YUVToRaw(pY1[x+0], pUV[x+idxU], pUV[x+idxV], pRaw1, k+0);
                YUVToRaw(pY1[x+1], pUV[x+idxU], pUV[x+idxV], pRaw1, k+3);
                YUVToRaw(pY2[x+0], pUV[x+idxU], pUV[x+idxV], pRaw2, k+0);
                YUVToRaw(pY2[x+1], pUV[x+idxU], pUV[x+idxV], pRaw2, k+3);
            }
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    
    else if(yuvFormat==ASVL_PAF_I420 || yuvFormat==ASVL_PAF_YV12)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        if(yuvFormat==ASVL_PAF_YV12)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
            {
                YUVToRaw(pY1[x+0], pU[i], pV[i], pRaw1, k+0);
                YUVToRaw(pY1[x+1], pU[i], pV[i], pRaw1, k+3);
                YUVToRaw(pY2[x+0], pU[i], pV[i], pRaw2, k+0);
                YUVToRaw(pY2[x+1], pU[i], pV[i], pRaw2, k+3);
            }
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_UYVY || yuvFormat==ASVL_PAF_VYUY || yuvFormat==ASVL_PAF_YUYV || yuvFormat==ASVL_PAF_YVYU 
        || yuvFormat==ASVL_PAF_UYVY2 || yuvFormat==ASVL_PAF_VYUY2 || yuvFormat==ASVL_PAF_YUYV2 || yuvFormat==ASVL_PAF_YVYU2)
    {
        Byte *pYUV=ppYUV[0];
        int x2, idxY0=1, idxY1=3, idxU0=0, idxV0=2;
        if(yuvFormat==ASVL_PAF_VYUY)
            (idxY0=1, idxY1=3, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV)
            (idxY0=0, idxY1=2, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU)
            (idxY0=0, idxY1=2, idxU0=3, idxV0=1);
        else if(yuvFormat==ASVL_PAF_UYVY2)
            (idxY0=3, idxY1=1, idxU0=0, idxV0=2);
        else if(yuvFormat==ASVL_PAF_VYUY2)
            (idxY0=3, idxY1=1, idxU0=2, idxV0=0);
        else if(yuvFormat==ASVL_PAF_YUYV2)
            (idxY0=2, idxY1=0, idxU0=1, idxV0=3);
        else if(yuvFormat==ASVL_PAF_YVYU2)
            (idxY0=2, idxY1=0, idxU0=3, idxV0=1);
        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
                x2=x*2;
                YUVToRaw(pYUV[x2+idxY0], pYUV[x2+idxU0], pYUV[x2+idxV0], pRaw, k+0);
                YUVToRaw(pYUV[x2+idxY1], pYUV[x2+idxU0], pYUV[x2+idxV0], pRaw, k+3);
            }
        }
    }
    else if(yuvFormat==ASVL_PAF_I422V || yuvFormat==ASVL_PAF_YV16V)
    {
        Byte *pY1=ppYUV[0], *pY2=pY1+pYuvWidthBytes[0];
        Byte *pU=ppYUV[1], *pV=ppYUV[2];
        Byte *pRaw1=pRaw, *pRaw2=pRaw1+rawWidthBytes;
        if(yuvFormat==ASVL_PAF_YV16V)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y+=2, pY1=pY2+pYuvWidthBytes[0], pY2=pY1+pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=0; x<width; x++, k+=3)
            {
                YUVToRaw(pY1[x], pU[x], pV[x], pRaw1, k);
                YUVToRaw(pY2[x], pU[x], pV[x], pRaw2, k);
            }
            pRaw1=pRaw2+rawWidthBytes;
            pRaw2=pRaw1+rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_I422H || yuvFormat==ASVL_PAF_YV16H)
    {
        Byte *pY=ppYUV[0], *pU=ppYUV[1], *pV=ppYUV[2];
        if(yuvFormat==ASVL_PAF_YV16H)
            (pU=ppYUV[2], pV=ppYUV[1]);
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pU+=pYuvWidthBytes[1], pV+=pYuvWidthBytes[2])
        {
            for(x=k=i=0; x<width; x+=2, k+=6, i++)
            {
                YUVToRaw(pY[x+0], pU[i], pV[i], pRaw, k+0);
                YUVToRaw(pY[x+1], pU[i], pV[i], pRaw, k+3);
            }
            pRaw+=rawWidthBytes;
        }
    }
    else if(yuvFormat==ASVL_PAF_YUV || yuvFormat==ASVL_PAF_YVU || yuvFormat==ASVL_PAF_UVY || yuvFormat==ASVL_PAF_VUY)
    {
        Byte *pYUV=ppYUV[0];
        int idxY=0, idxU=1, idxV=2;
        if(yuvFormat==ASVL_PAF_YVU)
            (idxV=1, idxU=2);
        else if(yuvFormat==ASVL_PAF_UVY)
            (idxU=0, idxV=1, idxY=2);
        else if(yuvFormat==ASVL_PAF_VUY)
            (idxV=0, idxU=1, idxY=2);
        for(y=0; y<height; y++, pYUV+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                YUVToRaw(pYUV[k+idxY], pYUV[k+idxU], pYUV[k+idxV], pRaw, k);
        }
    }
    else if(yuvFormat==ASVL_PAF_LPI422H || yuvFormat==ASVL_PAF_NV61)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV61)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x+=2, k+=6)
            {
				//                 YUV2RGB(pY[x+0], pUV[x+idxU], pUV[x+idxV], pRaw[k+2], pRaw[k+1], pRaw[k+0]);
				//                 YUV2RGB(pY[x+1], pUV[x+idxU], pUV[x+idxV], pRaw[k+5], pRaw[k+4], pRaw[k+3]);
                YUVToRaw(pY[x+0], pUV[x+idxU], pUV[x+idxV], pRaw, k+0);
                YUVToRaw(pY[x+1], pUV[x+idxU], pUV[x+idxV], pRaw, k+3);
            }
        }
    }
	else if(yuvFormat==ASVL_PAF_NV24 || yuvFormat==ASVL_PAF_NV42)
    {
        Byte *pY=ppYUV[0], *pUV=ppYUV[1];
        int x2, idxU=0, idxV=1;
		if(yuvFormat==ASVL_PAF_NV42)
			idxU=1, idxV=0;
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pUV+=pYuvWidthBytes[1], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
            {
				x2=x<<1;
				//                 YUV2RGB(pY[x+0], pUV[x+idxU], pUV[x+idxV], pRaw[k+2], pRaw[k+1], pRaw[k+0]);
				//                 YUV2RGB(pY[x+1], pUV[x+idxU], pUV[x+idxV], pRaw[k+5], pRaw[k+4], pRaw[k+3]);
                YUVToRaw(pY[x+0], pUV[x2+idxU], pUV[x2+idxV], pRaw, k+0);
            }
        }
    }
    else if(yuvFormat==ASVL_PAF_GRAY)
    {
        Byte* pY=ppYUV[0];
        for(y=0; y<height; y++, pY+=pYuvWidthBytes[0], pRaw+=rawWidthBytes)
        {
            for(x=k=0; x<width; x++, k+=3)
                (pRaw[k+IDX_Y0]=pY[x], pRaw[k+IDX_U0]=pRaw[k+IDX_V0]=128);
        }
    }
    else
    {
        Alert("cvtYUV2Raw: Do not support this YUV format\r\n");
        // exit(-1);
		return 0;
    }
    return 1;
}
int ZoomImageBilinear(ImageYUV* pSrcImgYUV, OUT ImageYUV* pDstImgYUV)
{
	int srcW=pSrcImgYUV->width, srcH=pSrcImgYUV->height, dstW=pDstImgYUV->width, dstH=pDstImgYUV->height;
	ImageRGB *pSrcImgRGB, *pDstImgRGB;
	pSrcImgRGB=new ImageRGB(srcW, srcH, 3);
	pDstImgRGB=new ImageRGB(dstW, dstH, 3);
	cvtYUV2Raw(pSrcImgRGB->pData, pSrcImgRGB->widthBytes, srcW, srcH, pSrcImgYUV->format, pSrcImgYUV->pWidthBytes, pSrcImgYUV->ppData);
	// cvtYUV2Raw(pDstImgRGB->pData, pDstImgRGB->widthBytes, dstW, dstH, pDstImgYUV->format, pDstImgYUV->pWidthBytes, pDstImgYUV->ppData);
	ZoomImageBilinear(pSrcImgRGB, pDstImgRGB);
	cvtRaw2YUV(pDstImgRGB->pData, pDstImgRGB->widthBytes, dstW, dstH, pDstImgYUV->format, pDstImgYUV->pWidthBytes, pDstImgYUV->ppData);
	delete pSrcImgRGB;
	delete pDstImgRGB;
	return 1;
}
int ZoomImageNN(ImageYUV* pSrcImgYUV, OUT ImageYUV* pDstImgYUV)
{
	int srcW=pSrcImgYUV->width, srcH=pSrcImgYUV->height, dstW=pDstImgYUV->width, dstH=pDstImgYUV->height;
	ImageRGB *pSrcImgRGB, *pDstImgRGB;
	pSrcImgRGB=new ImageRGB(srcW, srcH, 3);
	pDstImgRGB=new ImageRGB(dstW, dstH, 3);
	cvtYUV2Raw(pSrcImgRGB->pData, pSrcImgRGB->widthBytes, srcW, srcH, pSrcImgYUV->format, pSrcImgYUV->pWidthBytes, pSrcImgYUV->ppData);
	// cvtYUV2Raw(pDstImgRGB->pData, pDstImgRGB->widthBytes, dstW, dstH, pDstImgYUV->format, pDstImgYUV->pWidthBytes, pDstImgYUV->ppData);
	ZoomImageNN(pSrcImgRGB, pDstImgRGB);
	cvtRaw2YUV(pDstImgRGB->pData, pDstImgRGB->widthBytes, dstW, dstH, pDstImgYUV->format, pDstImgYUV->pWidthBytes, pDstImgYUV->ppData);
	delete pSrcImgRGB;
	delete pDstImgRGB;
	return 1;
}
inline void CopyData(void* pSrcData, int srcWidthBytes, OUT void* pDstData, int dstWidthBytes, int height)
{
	int y;
	Byte *pSrcLine=(Byte*)pSrcData, *pDstLine=(Byte*)pDstData;
	for(y=0; y<height; y++, pSrcLine+=srcWidthBytes, pDstLine+=dstWidthBytes)
		memcpy(pDstLine, pSrcLine, MIN(srcWidthBytes, dstWidthBytes));
}
ImageRGB* ConvertImageYUV2RGB(const ImageYUV* pImgYUV, OUT ImageRGB* pImgRGB/* =0 */)
{
	int width=pImgYUV->width, height=pImgYUV->height;
	if(pImgRGB==0)
		pImgRGB=new ImageRGB(width, height, 3);
	else pImgRGB->Resize(width, height, 3);
	if(isTypeYUV(pImgYUV->format))
		cvtYUV2RGB24(pImgRGB->pData, pImgRGB->widthBytes, width, height, pImgYUV->format, (int*)pImgYUV->pWidthBytes, (Byte**)pImgYUV->ppData);
	else if(isTypeRGB(pImgYUV->format) && pImgYUV->format==ASVL_PAF_RGB24_B8G8R8 || pImgYUV->format==ASVL_PAF_RGB24_R8G8B8)
		CopyData(pImgYUV->ppData[0], pImgYUV->pWidthBytes[0], pImgRGB->pData, pImgRGB->widthBytes, height);
	else
	{
		Alert("ConvertImageYUV2RGB do not support this format");
		return 0;
	}
	return pImgRGB;
}
ImageRAW* ConvertImageYUV2RAW(const ImageYUV* pImgYUV, OUT ImageRAW* pImgRAW/*=0*/)
{
	int width=pImgYUV->width, height=pImgYUV->height;
	if(pImgRAW==0)
		pImgRAW=new ImageRAW(width, height, 3);
	else pImgRAW->Resize(width, height, 3);
	if(isTypeYUV(pImgYUV->format))
		cvtYUV2Raw(pImgRAW->pData, pImgRAW->widthBytes, width, height, pImgYUV->format, (int*)pImgYUV->pWidthBytes, (Byte**)pImgYUV->ppData);
	else if(isTypeRGB(pImgYUV->format) && pImgYUV->format==ASVL_PAF_RGB24_B8G8R8 || pImgYUV->format==ASVL_PAF_RGB24_R8G8B8)
		CopyData(pImgYUV->ppData[0], pImgYUV->pWidthBytes[0], pImgRAW->pData, pImgRAW->widthBytes, height);
	else
	{
		Alert("ConvertImageYUV2RGB do not support this format");
		return 0;
	}
	return pImgRAW;
}
ImageYUV* ConvertImageRGB2YUV(const ImageRGB* pImgRGB, OUT ImageYUV* pImgYUV/* =0 */)
{
	int width=pImgRGB->width, height=pImgRGB->height;
	if(pImgYUV==0)
		pImgYUV=new ImageYUV(width, height, ASVL_PAF_YUV, 4);
	else pImgYUV->Resize(width, height, 0, 4);
	if(isTypeYUV(pImgYUV->format))
		cvtRGB2YUV(pImgRGB->pData, pImgRGB->widthBytes, width, height, pImgYUV->format, pImgYUV->pWidthBytes, pImgYUV->ppData);
	else if(isTypeRGB(pImgYUV->format) && pImgYUV->format==ASVL_PAF_RGB24_B8G8R8 || pImgYUV->format==ASVL_PAF_RGB24_R8G8B8)
		CopyData(pImgRGB->pData, pImgRGB->widthBytes, pImgYUV->ppData[0], pImgYUV->pWidthBytes[0], height);
	else
	{
		Alert("ConvertImageYUV2RGB do not support this format");
		return 0;
	}
	return pImgYUV;
}
ImageRAW* ConvertImageRGB2RAW(const ImageRGB* pImgRGB, OUT ImageRAW* pImgRAW/* =0 */)
{
	int width=pImgRGB->width, height=pImgRGB->height;
	if(pImgRAW==0)
		pImgRAW=new ImageRAW(width, height, 3);
	else pImgRAW->Resize(width, height, 3);
	cvtRGB2YUV(pImgRGB->pData, pImgRGB->widthBytes, width, height, ASVL_PAF_YUV, (int*)&pImgRAW->widthBytes, (Byte**)&pImgRAW->pData);
	return pImgRAW;
}
ImageRGB* ConvertImageRAW2RGB(const ImageRAW* pImgRAW, OUT ImageRGB* pImgRGB/* =0 */)
{
	int width=pImgRAW->width, height=pImgRAW->height;
	if(pImgRGB==0)
		pImgRGB=new ImageRGB(width, height, 3);
	else pImgRGB->Resize(width, height, 3);
	cvtYUV2RGB24(pImgRGB->pData, pImgRGB->widthBytes, width, height, ASVL_PAF_YUV, (int*)&pImgRAW->widthBytes, (Byte**)&pImgRAW->pData);
	return pImgRGB;
}
ImageYUV* ConvertImageRAW2YUV(const ImageRAW* pImgRAW, OUT ImageYUV* pImgYUV/* =0 */)
{
	int width=pImgRAW->width, height=pImgRAW->height;
	if(pImgYUV==0)
		pImgYUV=new ImageYUV(width, height, ASVL_PAF_YUV, 4);
	else pImgYUV->Resize(width, height, 0, 4);
	if(isTypeYUV(pImgYUV->format))
		cvtRaw2YUV(pImgRAW->pData, pImgRAW->widthBytes, width, height, pImgYUV->format, (int*)pImgYUV->pWidthBytes, (Byte**)pImgYUV->ppData);
	else if(isTypeRGB(pImgYUV->format) && pImgYUV->format==ASVL_PAF_RGB24_B8G8R8 || pImgYUV->format==ASVL_PAF_RGB24_R8G8B8)
		CopyData(pImgRAW->pData, pImgRAW->widthBytes, pImgYUV->ppData[0], pImgYUV->pWidthBytes[0], height);
	else
	{
		Alert("ConvertImageYUV2RGB do not support this format");
		return 0;
	}
	return pImgYUV;
}
static int getImageFormatYUV(const char* fileName)
{
	int extPos=strlen(fileName)-1, namePos=extPos, /*numPos,*/ i;
	char ppExtName[][8]={
		"I444", "YV24", "NV12", "NV21", "I420", "YV12", "UYVY", "VYUY", 
		"YUYV", "YVYU", "UYVY2","VYUY2","YUYV2","YVYU2","I422V","YV16V",  
		"I422H", "YV16H", "YUV", "YVU", "UVY", "VUY", "LPI422H", "Grey", 
		"Gray", "Raw", "Rgn","RGB24","RGB", "BMP", "JPG", "PNG", "AVI",
		"NV16", "NV61", "NV24", "NV42"
	};
	int pFormat[]={
		ASVL_PAF_I444, ASVL_PAF_YV24, ASVL_PAF_NV12, ASVL_PAF_NV21, ASVL_PAF_I420, ASVL_PAF_YV12, ASVL_PAF_UYVY, ASVL_PAF_VYUY,
		ASVL_PAF_YUYV, ASVL_PAF_YVYU, ASVL_PAF_UYVY2,ASVL_PAF_VYUY2,ASVL_PAF_YUYV2,ASVL_PAF_YVYU2,ASVL_PAF_I422V,ASVL_PAF_YV16V,
		ASVL_PAF_I422H,ASVL_PAF_YV16H,ASVL_PAF_YUV, ASVL_PAF_YVU, ASVL_PAF_UVY, ASVL_PAF_VUY, ASVL_PAF_LPI422H, ASVL_PAF_GRAY, 
		ASVL_PAF_GRAY, ASVL_PAF_GRAY, ASVL_PAF_RGN, ASVL_PAF_RGB, ASVL_PAF_RGB, ASVL_PAF_BMP,ASVL_PAF_JPG, ASVL_PAF_PNG, ASVL_PAF_AVI,
		ASVL_PAF_NV16, ASVL_PAF_NV61, ASVL_PAF_NV24, ASVL_PAF_NV42
	};
	int formatNum=sizeof(pFormat)/sizeof(*pFormat);

	while(namePos>=0 && fileName[namePos]!='\\' && fileName[namePos]!='/')
		namePos--;
	namePos++;
	while(extPos>namePos && fileName[extPos]!='.')
		extPos--;
	if(extPos==namePos)
		return 0;
	extPos++;
	//     numPos=extPos-2;
	//     while(numPos>0 && !IS_NUMBER(fileName[numPos]))
	//         numPos--;

	for(i=0; i<formatNum; i++)
	{
		if(stricmp(fileName+extPos, ppExtName[i])==0)
			return pFormat[i];
	}
	//     Printf(TEXT_COLOR_RED, "Format '%s' unknown\n", fileName+extPos);
	return 0;
}
int GetImageSizeAndFormatYUV(const char* fileName, int *pFormat)
{
	int flen;
	FILE* fp=fopen(fileName, "rb");
	if(fp==0)
		return 0;
	fseek(fp, 0, SEEK_END);
	flen=ftell(fp);
	fclose(fp);
	if(pFormat)
		*pFormat=getImageFormatYUV(fileName);
	return flen;
}
#define IS_NUMBER(x) ('0'<=(x) && (x)<='9')
int splitRawFileName(const char* fileName, OUT int* pWidth, OUT int* pHeight)
{
	int extPos=strlen(fileName)-1, namePos=extPos, numPos, i, radix=1, height=0, width=0;
	char ppExtName[][8]={
		"I444", "YV24", "NV12", "NV21", "I420", "YV12", "UYVY", "VYUY", 
		"YUYV", "YVYU", "UYVY2","VYUY2","YUYV2","YVYU2","I422V","YV16V",  
		"I422H", "YV16H", "YUV", "YVU", "UVY", "VUY", "LPI422H", "Grey", 
		"Gray", "Raw", "Rgn","RGB24","RGB", "BMP", "JPG", "PNG", "AVI",
		"NV16", "NV61", "NV24", "NV42"
	};
	int pFormat[]={
		ASVL_PAF_I444, ASVL_PAF_YV24, ASVL_PAF_NV12, ASVL_PAF_NV21, ASVL_PAF_I420, ASVL_PAF_YV12, ASVL_PAF_UYVY, ASVL_PAF_VYUY,
		ASVL_PAF_YUYV, ASVL_PAF_YVYU, ASVL_PAF_UYVY2,ASVL_PAF_VYUY2,ASVL_PAF_YUYV2,ASVL_PAF_YVYU2,ASVL_PAF_I422V,ASVL_PAF_YV16V,
		ASVL_PAF_I422H,ASVL_PAF_YV16H,ASVL_PAF_YUV, ASVL_PAF_YVU, ASVL_PAF_UVY, ASVL_PAF_VUY, ASVL_PAF_LPI422H, ASVL_PAF_GRAY, 
		ASVL_PAF_GRAY, ASVL_PAF_GRAY, ASVL_PAF_RGN, ASVL_PAF_RGB, ASVL_PAF_RGB, ASVL_PAF_BMP,ASVL_PAF_JPG, ASVL_PAF_PNG, ASVL_PAF_AVI,
		ASVL_PAF_NV16, ASVL_PAF_NV61, ASVL_PAF_NV24, ASVL_PAF_NV42
	};
	int formatNum=sizeof(pFormat)/sizeof(*pFormat);

	while(namePos>=0 && fileName[namePos]!='\\' && fileName[namePos]!='/')
		namePos--;
	namePos++;
	while(extPos>namePos && fileName[extPos]!='.')
		extPos--;
	if(extPos==namePos)
		return 0;
	extPos++;
	numPos=extPos-2;
	while(numPos>namePos)
	{
		while(numPos>namePos && !IS_NUMBER(fileName[numPos]))
			numPos--;
		for(i=numPos, height=0, radix=1; i>=namePos; i--)
		{
			if(!IS_NUMBER(fileName[i]))
				break;
			height=(fileName[i]-'0')*radix+height;
			radix=radix*10;
		}
		if(i>namePos && (fileName[i]=='x' || fileName[i]=='X') && IS_NUMBER(fileName[i-1]))    
		{
			for(i=i-1, width=0, radix=1; i>=namePos; i--)
			{
				if(!IS_NUMBER(fileName[i]))
					break;
				width=(fileName[i]-'0')*radix+width;
				radix=radix*10;
			}
			if(height>0 && width>0)
				break;
		}
		numPos=i-1;
	}
	if(pWidth)
		*pWidth=width;
	if(pHeight)
		*pHeight=height;
	//Printf(TEXT_COLOR_GREEN, "    width=%d height=%d extName='%s'\n", width, height, fileName+extPos);
	// return ASVL_PAF_NV12;    
	for(i=0; i<formatNum; i++)
	{
		if(stricmp(fileName+extPos, ppExtName[i])==0)
			return pFormat[i];
	}
	//     Printf(TEXT_COLOR_RED, "Format '%s' unknown\n", fileName+extPos);
	return 0;
}
char* GetYuvExtenName(int yuvFormat)
{
    static char ppExtName[][8]={
        "I444", "YV24", "NV12", "NV21", "I420", "YV12", "UYVY", "VYUY", 
            "YUYV", "YVYU", "UYVY2","VYUY2","YUYV2","YVYU2","I422V","YV16V",  
            "I422H", "YV16H", "YUV", "YVU", "UVY", "VUY", "LPI422H", "Gray", "Raw", "Rgn", 
            "bmp", "jpg", "png", "avi", "RGB24", "RGB24", "NV16", "NV61", "NV24", "NV42"
    };
    int pFormat[]={
        ASVL_PAF_I444, ASVL_PAF_YV24, ASVL_PAF_NV12, ASVL_PAF_NV21, ASVL_PAF_I420, ASVL_PAF_YV12, ASVL_PAF_UYVY, ASVL_PAF_VYUY,
            ASVL_PAF_YUYV, ASVL_PAF_YVYU, ASVL_PAF_UYVY2,ASVL_PAF_VYUY2,ASVL_PAF_YUYV2,ASVL_PAF_YVYU2,ASVL_PAF_I422V,ASVL_PAF_YV16V,
            ASVL_PAF_I422H,ASVL_PAF_YV16H,ASVL_PAF_YUV, ASVL_PAF_YVU, ASVL_PAF_UVY, ASVL_PAF_VUY, ASVL_PAF_LPI422H, ASVL_PAF_GRAY, 
            ASVL_PAF_GRAY, ASVL_PAF_RGN, ASVL_PAF_BMP, ASVL_PAF_JPG, ASVL_PAF_PNG, ASVL_PAF_AVI, 
			ASVL_PAF_RGB24_R8G8B8, ASVL_PAF_RGB24_B8G8R8, ASVL_PAF_NV16, ASVL_PAF_NV61, ASVL_PAF_NV24, ASVL_PAF_NV42
    };
    int formatNum=sizeof(pFormat)/sizeof(*pFormat);
    int i;
    for(i=0; i<formatNum; i++)
    {
        if(pFormat[i]==yuvFormat)
            return ppExtName[i];
    }
    return 0;
}
// static int getFileSize(const char* fileName)
// {
// 	FILE* fp;
// 	int flen;
// 	if(!IsFileExist(fileName))
// 		return 0;
// 	fp=fopen(fileName, "rb");
// 	if(fp==0)
// 		return 0;
// 	fseek(fp, 0, SEEK_END);
// 	flen=ftell(fp);
// 	fclose(fp);
// 	return flen;
// }
static __int64 getFileSize(const char* fileName)
{
	__int64 fileSize=0;
	HANDLE hFile;
	if(!IsFileExist(fileName))
	{
		WriteLog("'%s' is not exist", fileName);
		return 0;
	}
	WriteLog("getFileSize(%s)", fileName);
	hFile=CreateFileA(fileName, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	WriteLog("CreateFileA(fileName, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0)=%d", hFile);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		// Printf(TEXT_COLOR_RED, "Could not open file (error %d)\n", GetLastError());
		WriteLog("Create File failed");
		return 0;
	}
	WriteLog("GetFileSizeEx(hFile, (LARGE_INTEGER*)&fileSize);");
	GetFileSizeEx(hFile, (LARGE_INTEGER*)&fileSize);
	CloseHandle(hFile);
	return fileSize;
}
// int GuessAlignBytes(int flen, int width, int height, int format)
// {
// 	int alignBytes=0, alignBytesMin=0;
// 	int dataSize=GetImageDataSize(width, height, format, 4);
// 	if(dataSize==flen)
// 		return 4;
// 
// 	for(alignBytes=1; alignBytes<=MAX_ALIGNBYTES; alignBytes*=2)
// 	{
// 		dataSize=GetImageDataSize(width, height, format, alignBytes);
// 		if(dataSize==flen)
// 		{
// 			alignBytesMin=alignBytes;
// 			break;
// 		}
// 	}
// 	return alignBytesMin;
// }

static int GuessAlignBytes(int flen, int width, int height, int format)
{
	int alignBytes=0, alignBytesMin=0;
	int dataSize=GetImageDataSize(width, height, format, 4);
	if(flen%dataSize==0)
		return 4;
	for(alignBytes=1; alignBytes<=MAX_ALIGNBYTES; alignBytes*=2)
	{
		dataSize=GetImageDataSize(width, height, format, alignBytes);
		if(flen%dataSize==0)
		{
			alignBytesMin=alignBytes;
			break;
		}
	}
	return alignBytesMin;
}

static int CheckVideoFile(__int64 flen, int width, int height, int yuvFormat, OUT int* pAlignBytes)
{
	int alignBytes=*pAlignBytes, dataSize;
	int frameNum=1;
	alignBytes=*pAlignBytes=4;
	dataSize=GetImageDataSize(width, height, yuvFormat, alignBytes);
	if(flen%dataSize==0)
		return (int)(flen/dataSize);
	for(alignBytes=1; alignBytes<=MAX_ALIGNBYTES; alignBytes*=2)
	{
		dataSize=GetImageDataSize(width, height, yuvFormat, alignBytes);
		if(flen%dataSize==0)
		{
			*pAlignBytes=alignBytes;
			frameNum=(int)(flen/dataSize);
			break;
		}
	}
	return frameNum;
}
static int GuessAlignBytesAndFormat(int flen, int width, int height, OUT int* pFormat, OUT int* pAlignBytes)
{
	int len=0, format=*pFormat;
	int alignBytes, dataSize;
	int pFormatList[]={ASVL_PAF_I444, ASVL_PAF_NV21, ASVL_PAF_I420, ASVL_PAF_UYVY, ASVL_PAF_I422V,
		ASVL_PAF_I422H,ASVL_PAF_YUV, ASVL_PAF_LPI422H, ASVL_PAF_GRAY};
	int pValidForamtIdx[16]={0}, pValidAlignBytes[16]={0}, validForamtNum=0;
	int familyNum=sizeof(pFormatList)/sizeof(*pFormatList), i;

	alignBytes=GuessAlignBytes(flen, width, height, format);
	dataSize=GetImageDataSize(width, height, format, 4);
	if(alignBytes<1)
	{
		for(i=0; i<familyNum; i++)
		{
			format=pFormatList[i];
			alignBytes=GuessAlignBytes(flen, width, height, format);
			if(alignBytes>=1)
			{
				pValidForamtIdx[validForamtNum]=i;
				pValidAlignBytes[validForamtNum]=alignBytes;
				validForamtNum++;
			}
		}
		if(validForamtNum==0)// Do not find the correct format
		{
			if(dataSize<flen)
			{
				alignBytes=4;
				format=*pFormat;
				if(pAlignBytes)
					*pAlignBytes=alignBytes;
			}
			return alignBytes;
		}
		alignBytes=pValidAlignBytes[0];
		format=pFormatList[pValidForamtIdx[0]];
	}
	*pFormat=format;
	if(pAlignBytes)
		*pAlignBytes=alignBytes;
	return alignBytes;
}
#define ALERT_FILE_SIZE (128*1024*1024)
// static int gIsVideoFile=0, gFrameNum=1;
// static int IsCurrentVidioFile(const char* fileName, int width, int height, int yuvFormat)
// {
// 	int yuvFormat0;
// 	__int64 flen=getFileSize(fileName);
// 	// gIsVideoFile=0;
// 	if(flen==0)
// 		return 0;
// 	if(width==0 || height==0)
// 		yuvFormat0=splitRawFileName(fileName, &width, &height);
// 	else yuvFormat0=getImageFormatYUV(fileName);
// 
// 	if(yuvFormat==0)
// 		yuvFormat=yuvFormat0;
// 	if(yuvFormat==0)
// 	{
// 		return 0;
// 	}
// 	if(width>0 && height>0)
// 	{
// 		int alignBytes;
// 		int frameNum=CheckVideoFile(flen, width, height, yuvFormat, &alignBytes);
// 		if(frameNum>1)
// 		{
// 			//gIsVideoFile=1;
// 			//gFrameNum=frameNum;
// 			return 1;
// 		}
// 	}
// 	return 0;
// }
#define SEEK_TYPE_BEGIN   0 // FILE_BEGIN
#define SEEK_TYPE_CURRENT 1 // FILE_CURRENT
#define SEEK_TYPE_END     2 // FILE_END
ImageYUV* ImageYUV::LoadImageYUV(const char* fileName, int width/*=0*/, int height/*=0*/, int yuvFormat/*=0*/, OUT int* pAlignBytes/*=0*/, int idxFrame/*=0*/)
{
	if(this==0)
		return 0;
	FILE* fp;
	__int64 flen64;
	int flen, alignBytes=4, dataSize;
	int yuvFormat0, retVal;

	if(!IsFileExist(fileName))
		return 0;

	flen64=getFileSize(fileName);
	WriteLog("getFileSize(fileName)=%I64d", flen64);
	if(flen64==0)
		return 0;
	
	if(width==0 || height==0)
		yuvFormat0=splitRawFileName(fileName, &width, &height);
	else yuvFormat0=getImageFormatYUV(fileName);
	WriteLog("splitRawFileName(fileName, &width, &height):width=%d height=%d yuvFormat=%d", width, height, yuvFormat0);

	if(yuvFormat==0)
		yuvFormat=yuvFormat0;

	if(yuvFormat==0 || width==0 || height==0)
	{
		// ShowMessage("This format do not support\r\n");
		return 0;
	}
	int frameNum=CheckVideoFile(flen64, width, height, yuvFormat, &alignBytes);
	WriteLog("%d=CheckVideoFile(flen64, width, height, yuvFormat, &alignBytes);", frameNum);
	if(frameNum>1) // if it is a video file
	{
		WriteLog("it is a video file");
		int readSize=0;
		HANDLE hFile=CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			// Printf(TEXT_COLOR_RED, "Could not open file (error %d)\n", GetLastError());
			return 0;
		}
		if(pAlignBytes)
			*pAlignBytes=alignBytes;
		idxFrame=(idxFrame+frameNum)%frameNum;
		dataSize=GetImageDataSize(width, height, yuvFormat, alignBytes);
		__int64 offset=(__int64)idxFrame*dataSize;
		SetFilePointerEx(hFile, *(LARGE_INTEGER*)&offset, 0, SEEK_TYPE_BEGIN);
		this->Resize(width, height, yuvFormat, alignBytes);			
		ReadFile(hFile, this->ppData[0], dataSize, (DWORD*)&readSize, 0);
		CloseHandle(hFile);
		YuvInfo *pYuvInfo=GetYuvInfo(this);
		pYuvInfo->frameNum=frameNum;
		pYuvInfo->idxFrame=idxFrame;
		pYuvInfo->fileName=fileName;
		pYuvInfo->alignBytes=alignBytes;
		return this;
	}
	// if it is a common file
	flen=(int)flen64;
	if(flen>ALERT_FILE_SIZE)
		return 0;
	WriteLog("it is a common file fileSize=%d", flen);
// 	this->Resize(width, height, yuvFormat, 4);
// 	yuvFormat0=yuvFormat;
	retVal=GuessAlignBytesAndFormat(flen, width, height, &yuvFormat, &alignBytes);
	WriteLog("GuessAlignBytesAndFormat(%d,%d,%d,%p,%p)=%d", flen, width, height, &yuvFormat, &alignBytes, retVal);
	if(retVal<=0)
		return 0;

	dataSize=GetImageDataSize(width, height, yuvFormat, alignBytes);
	WriteLog("GetImageDataSize=%d\n", dataSize);
	// Printf(TEXT_COLOR_BLUE, "width=%d height=%d yuvFormat=%s alignBytes=%d\n", width, height, GetYuvExtenName(yuvFormat), alignBytes);
	// ResizeImageYUV(pFrame, width, height, yuvFormat, alignBytes);
	if(pAlignBytes)
		*pAlignBytes=alignBytes;
	
	fp=fopen(fileName, "rb");
	if(fp==0)
		return 0;
	this->Resize(width, height, yuvFormat, alignBytes);
	fread(this->ppData[0], dataSize, 1, fp);
	fclose(fp);

	YuvInfo *pYuvInfo=GetYuvInfo(this);
	pYuvInfo->frameNum=frameNum;
	pYuvInfo->idxFrame=idxFrame;
	pYuvInfo->fileName=fileName;
	pYuvInfo->alignBytes=alignBytes;

	return this;
}
ImageYUV* ImageYUV::GetFrameYUV(int idxFrame/* =0 */)
{
	if(this==0 || idxFrame<0)
		return 0;
	YuvInfo* pYuvInfo=GetYuvInfo(this);
	Assert(pYuvInfo->fileName);
	if(idxFrame==pYuvInfo->idxFrame)
		return this;
	return LoadImageYUV(pYuvInfo->fileName, 0, 0, 0, 0, idxFrame);
}
int ImageYUV::GetFrameNum()
{
	if(this==0)
		return 0;
	YuvInfo* pYuvInfo=GetYuvInfo(this);
	return pYuvInfo->frameNum;
}

char* CreateYuvFileName(int width, int height, int format, const char* pBasePath, const char* pPrefix, int idx, const char* pSuffix, OUT char* fileName)
{
	if(width==0 || height==0)
		return 0;
	if(pPrefix==0)
        pPrefix="";
    if(pSuffix==0)
        pSuffix="";
	if(idx<0)
	{
		if(pSuffix[0]==0)
			sprintf(fileName, "%s\\%s%dx%d.%s", pBasePath, pPrefix, width, height, GetYuvExtenName(format));
		else
			sprintf(fileName, "%s\\%s%s_%dx%d.%s", pBasePath, pPrefix, pSuffix, width, height, GetYuvExtenName(format));
	}
	else
    {
		sprintf(fileName, "%s\\%s%03d%s_%dx%d.%s", pBasePath, pPrefix, idx, pSuffix, width, height, GetYuvExtenName(format));
    }
	return fileName;
}
int ImageYUV::SaveImage(const char* pBasePath, const char* pPrefix, int idx, const char* pSuffix, int isSaveAsVideo/* =0 */)
{
	if(this==0)
		return 0;
	char fileName[256];	
	HANDLE hFile;
	if(isSaveAsVideo)
	{
		CreateYuvFileName(width, height, format, pBasePath, pPrefix, -1, pSuffix, fileName);
		if(idx==0)
		{
			hFile=CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
			if(hFile==INVALID_HANDLE_VALUE)
			{
				// Printf(TEXT_COLOR_RED, "Could not open file (error %d)\n", GetLastError());
				return 0;
			}
		}
		else
		{
			hFile=CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
			if(hFile==INVALID_HANDLE_VALUE)
			{
				// Printf(TEXT_COLOR_RED, "Could not open file (error %d)\n", GetLastError());
				return 0;
			}
			LARGE_INTEGER offset={0, 0};
			SetFilePointerEx((HANDLE)hFile, offset, 0, FILE_END);	
		}
	}
	else
	{
		CreateYuvFileName(width, height, format, pBasePath, pPrefix, idx, pSuffix, fileName);
		hFile=CreateFileA(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			// Printf(TEXT_COLOR_RED, "Could not open file (error %d)\n", GetLastError());
			return 0;
		}
	}
	int i, writeSize=0;
	for(i=0; i<3; i++)
	{
		if(!isHadPlane(format, i))
			break;
		if(isHalfHeight(format, i))
			WriteFile((HANDLE)hFile, ppData[i], pWidthBytes[i]*(height/2), (DWORD*)&writeSize, 0);
		else WriteFile((HANDLE)hFile, ppData[i], pWidthBytes[i]*height, (DWORD*)&writeSize, 0);
	}
	CloseHandle(hFile);
	return 1;
}

int ImageYUV::ConvertToRGB(OUT Byte* pRGB, int rgbWidthBytes, int channel/* =3 */)
{
	if(isTypeYUV(format))
	{
		if(channel==3)
			cvtYUV2RGB24(pRGB, rgbWidthBytes, width, height, format, pWidthBytes, ppData);
		else if(channel==4)
			cvtYUV2RGB32(pRGB, rgbWidthBytes, width, height, format, pWidthBytes, ppData);
	}
	else if(isTypeRGB(format) && channel==3)
	{
		CopyData(ppData[0], pWidthBytes[0], pRGB, rgbWidthBytes, height);
	}
	else
	{
		return 0;
	}
	return 1;
}
ImageRGB* LoadAsImageRGB(const char* fileName, int channel/* =4 */)
{
	ImageYUV* pImgYUV=new ImageYUV();
	if(pImgYUV->LoadImageYUV(fileName))
	{
		ImageRGB* pImgRGB=new ImageRGB(pImgYUV->width, pImgYUV->height, channel);
		if(pImgYUV->ConvertToRGB(pImgRGB->pData, pImgRGB->widthBytes, channel))
		{
			delete pImgYUV;
			return pImgRGB;
		}
	}
	delete pImgYUV;
	return 0;
}
YUV4 GetYUV(ImageYUV* pImgYUV, int x, int y, OUT RGB4* pRGB)
{
    YUV4 yuv(0, 0, 0);
	RGB4 rgb(0, 0, 0);
	if(pImgYUV==0 || pImgYUV->ppData[0]==0)
		return yuv;
    Byte *pData0=pImgYUV->ppData[0], *pData1=pImgYUV->ppData[1], *pData2=pImgYUV->ppData[2];
    int widthBytes0=pImgYUV->pWidthBytes[0], widthBytes1=pImgYUV->pWidthBytes[1], widthBytes2=pImgYUV->pWidthBytes[2];
    int format=pImgYUV->format;
    if(0<=x && x<pImgYUV->width && 0<=y && y<pImgYUV->height)
    {
        if(format==ASVL_PAF_I444)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y*widthBytes1+x], yuv.v=pData2[y*widthBytes2+x]);
        else if(format==ASVL_PAF_YV24)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData2[y*widthBytes2+x], yuv.v=pData1[y*widthBytes1+x]);
        else if(format==ASVL_PAF_NV12)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y/2*widthBytes1+x/2*2], yuv.v=pData1[y/2*widthBytes1+x/2*2+1]);
        else if(format==ASVL_PAF_NV21)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y/2*widthBytes1+x/2*2+1], yuv.v=pData1[y/2*widthBytes1+x/2*2]);
        else if(format==ASVL_PAF_I420)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y/2*widthBytes1+x/2], yuv.v=pData2[y/2*widthBytes2+x/2]);
        else if(format==ASVL_PAF_YV12)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData2[y/2*widthBytes2+x/2], yuv.v=pData1[y/2*widthBytes1+x/2]);
        else if(format==ASVL_PAF_UYVY)
            (yuv.y=pData0[y*widthBytes0+x*2+1], yuv.u=pData0[y*widthBytes0+x/2*4], yuv.v=pData0[y*widthBytes0+x/2*4+2]);
        else if(format==ASVL_PAF_VYUY)
            (yuv.y=pData0[y*widthBytes0+x*2+1], yuv.u=pData0[y*widthBytes0+x/2*4+2], yuv.v=pData0[y*widthBytes0+x/2*4]);
        else if(format==ASVL_PAF_YUYV)
            (yuv.y=pData0[y*widthBytes0+x*2], yuv.u=pData0[y*widthBytes0+x/2*4+1], yuv.v=pData0[y*widthBytes0+x/2*4+3]);
        else if(format==ASVL_PAF_YVYU)
            (yuv.y=pData0[y*widthBytes0+x*2], yuv.u=pData0[y*widthBytes0+x/2*4+3], yuv.v=pData0[y*widthBytes0+x/2*4+1]);
        else if(format==ASVL_PAF_UYVY2)
            (yuv.y=pData0[y*widthBytes0+x*2+3-4*(x&1)], yuv.u=pData0[y*widthBytes0+x/2*4], yuv.v=pData0[y*widthBytes0+x/2*4+2]);
        else if(format==ASVL_PAF_VYUY2)
            (yuv.y=pData0[y*widthBytes0+x*2+3-4*(x&1)], yuv.u=pData0[y*widthBytes0+x/2*4+2], yuv.v=pData0[y*widthBytes0+x/2*4]);
        else if(format==ASVL_PAF_YUYV2)
            (yuv.y=pData0[y*widthBytes0+x*2+2-4*(x&1)], yuv.u=pData0[y*widthBytes0+x/2*4+1], yuv.v=pData0[y*widthBytes0+x/2*4+3]);
        else if(format==ASVL_PAF_YVYU2)
            (yuv.y=pData0[y*widthBytes0+x*2+2-4*(x&1)], yuv.u=pData0[y*widthBytes0+x/2*4+3], yuv.v=pData0[y*widthBytes0+x/2*4+1]);
        else if(format==ASVL_PAF_I422V)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y/2*widthBytes1+x], yuv.v=pData2[y/2*widthBytes2+x]);
        else if(format==ASVL_PAF_YV16V)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData2[y/2*widthBytes2+x], yuv.v=pData1[y/2*widthBytes1+x]);
        else if(format==ASVL_PAF_I422H)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y*widthBytes1+x/2], yuv.v=pData2[y*widthBytes2+x/2]);
        else if(format==ASVL_PAF_YV16H)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData2[y*widthBytes2+x/2], yuv.v=pData1[y*widthBytes1+x/2]);
        else if(format==ASVL_PAF_YUV)
            (yuv.y=pData0[y*widthBytes0+3*x+0], yuv.u=pData0[y*widthBytes0+3*x+1], yuv.v=pData0[y*widthBytes0+3*x+2]);
        else if(format==ASVL_PAF_YVU)
            (yuv.y=pData0[y*widthBytes0+3*x+0], yuv.u=pData0[y*widthBytes0+3*x+2], yuv.v=pData0[y*widthBytes0+3*x+1]);
        else if(format==ASVL_PAF_UVY)
            (yuv.y=pData0[y*widthBytes0+3*x+2], yuv.u=pData0[y*widthBytes0+3*x+0], yuv.v=pData0[y*widthBytes0+3*x+1]);
        else if(format==ASVL_PAF_VUY)
            (yuv.y=pData0[y*widthBytes0+3*x+2], yuv.u=pData0[y*widthBytes0+3*x+1], yuv.v=pData0[y*widthBytes0+3*x+0]);
        else if(format==ASVL_PAF_LPI422H)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y*widthBytes1+x/2*2], yuv.v=pData1[y*widthBytes1+x/2*2+1]);
		else if(format==ASVL_PAF_NV61)
            (yuv.y=pData0[y*widthBytes0+x], yuv.v=pData1[y*widthBytes1+x/2*2], yuv.u=pData1[y*widthBytes1+x/2*2+1]);
		else if(format==ASVL_PAF_NV24)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=pData1[y*widthBytes1+x*2], yuv.v=pData1[y*widthBytes1+x*2+1]);
		else if(format==ASVL_PAF_NV42)
            (yuv.y=pData0[y*widthBytes0+x], yuv.v=pData1[y*widthBytes1+x*2], yuv.u=pData1[y*widthBytes1+x*2+1]);
        else if(format==ASVL_PAF_GRAY)
            (yuv.y=pData0[y*widthBytes0+x], yuv.u=yuv.v=128);
        else if(format==ASVL_PAF_RGN)
            (yuv.y=pData0[y*widthBytes0+2*x], yuv.u=yuv.v=pData0[y*widthBytes0+2*x+1]);
        
		if(isTypeYUV(format))
		{
			YUV2RGB(yuv.y, yuv.u, yuv.v, rgb.r, rgb.g, rgb.b);
		}
		else if(isTypeRGB(format))
		{
			int channels=((format>>8)&0x00ff)+1;
			int idx=y*widthBytes0+channels*x;
			if(format==ASVL_PAF_RGB24_B8G8R8 || format==ASVL_PAF_RGB32_B8G8R8 || format==ASVL_PAF_RGB32_B8G8R8A8)
				(rgb.r=pData0[idx+2], rgb.g=pData0[idx+1], rgb.b=pData0[idx+0]);
			else if(format==ASVL_PAF_RGB24_R8G8B8 || format==ASVL_PAF_RGB32_R8G8B8 || format==ASVL_PAF_RGB32_A8R8G8B8)
				(rgb.r=pData0[idx+0], rgb.g=pData0[idx+1], rgb.b=pData0[idx+2]);
			RGB2YUV(rgb.r, rgb.g, rgb.b, yuv.y, yuv.u, yuv.v);
		}
		else
        {
            MessageBox(0, "GetYUV: Can not support this YUV format\r\n", "Error", MB_OK);
            // exit(-1);
        }
    }
	if(pRGB)
		*pRGB=rgb;
    return yuv;
}
YUV4 ImageYUV::GetYUV(int x, int y, OUT RGB4* pRGB/* =0 */)
{
	return ::GetYUV(this, x, y, pRGB);
}