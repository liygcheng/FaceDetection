
/*-----------------------------------------------------------------------------
 *
 * This file is ArcSoft's property. It contains ArcSoft's trade secret, 
 * proprietary and confidential information. 
 * 
 * The information and code contained in this file is only for authorized 
 * ArcSoft employees to design, create, modify, or review.
 * 
 * DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT 
 * PROPER AUTHORIZATION.
 * 
 * If you are not an intended recipient of this file, you must not copy, 
 * distribute, modify, or take any action in reliance on it. 
 * 
 * If you have received this file in error, please immediately notify ArcSoft 
 * and permanently delete the original and any copy of any file and any 
 * printout thereof.
 *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * File Name - ajlSimpleExifAPI.h
 * Purpose   - AJL Common Header File
 *             This file contains:  definitions for data types, data
 *             structures, error codes, and function prototypes used
 *             in the Arcsoft (R) JPEG Library (AJL).
 *
 * History   - 
 *	Current Version:	1.0.0.0
 *	Last Updated Date:	 
 *	By:					Ji Chen
 *---------------------------------------------------------------------------*/

#ifndef __AJL_SIMEXIF_INC__
#define __AJL_SIMEXIF_INC__

/* configuration */
#ifdef __WIN32__
# undef __WIN32__
#endif

/* Spec Declare */
#if defined( __cplusplus )
#define AJL_API extern "C" 
#else
#define AJL_API
#endif

/** pdefine.h ***************************************************************/
typedef signed int      AInt;
typedef signed char     AInt8;
typedef signed short    AInt16;
typedef signed long     AInt32;
typedef unsigned int    AUInt;
typedef unsigned char   AUInt8;
typedef unsigned short  AUInt16;
typedef unsigned long   AUInt32;

typedef long            ALong;
typedef short           AShort;
typedef unsigned char   AByte;
typedef unsigned short  AWord;
typedef unsigned long   ADWord;
typedef float           AFloat;
typedef double          ADouble;
typedef void *          AHandle;
typedef char            AChar;
typedef unsigned short  AWChar;
typedef long            ABool;
typedef void            AVoid;
typedef void *          APVoid;
typedef char *          APChar;
typedef const char *    APCChar;
typedef	ALong           ARESULT;
typedef ADWord          ACOLORREF; 

typedef AByte           ASMP;
typedef AShort          ACOEF;

/** aconst.h ***********************************************************/

/* Status Tag */
#define ANULL                   0
#define AFALSE                  0
#define ATRUE                   1

/* Error Tag */  
#define AJL_OK                  0x00000000L
#define AJL_ERR_ARGUMENT        0x00000001L
#define AJL_ERR_NOT_AVAIL       0x00000002L
#define AJL_ERR_FILE_OP         0x00000003L
#define AJL_ERR_MEMORY_FAIL     0x00000004L
#define AJL_ERR_NOT_JPEG        0x00000005L
#define AJL_ERR_SCALEFACTOR     0x00000006L
#define AJL_ERR_UNKNOWN         0x00000007L
#define AJL_ERROR               0x00000008L
#define AJL_ERR_WRONGSCALE      0x00000009L
#define AJL_ERR_EXIF_NOT_AVAIL  0x0000000AL
#define AJL_USER_INTERRUPT      0x0000000BL
#define AJL_ERR_INVALID_PARAMS  0x0000000CL
#define AJL_ERR_UNSUPPORTTED    0x0000000DL
#define AJL_ERR_UNFINISHED      0x0000000EL
#define AJL_ERR_UNNAMED			(~0)


typedef struct TagEXIF

{
	/* for special exif tag */
       AInt				nWidth; 
       AInt				nHeight;
       AChar			szMake[64];
	   AInt             nMakeLength; //Include '/0'         
       AChar			szModel[70];
	   AInt             nModelLength;  
       ADouble			dExposureTime;
       ADouble			dFNumber;
       ALong			nExoposureProgram;
       ALong			nISOSpeedRatings;
       AChar			szDataTimeOriginal[20]; // The length must be 20
       AChar			szDataTimeDigitized[20];
	   AChar            szDataTime[20];
       ADouble			dShutterSpeedValue;
       ADouble			dAperturevalue;
       ADouble			dExposureBiasValue;
       ADouble			dMaxApertureValue;
       ALong			nMeteringMode;
       ALong			nFlash;
       ADouble			dlocalLength;

    /* for special thumbnail data */
	   AByte *			bThumbImage; //only support BRG Chunky format 
	   AInt				nThumbWidth;
	   AInt				nThumbHeight;
	   AInt				nThumbPitch;
	   
} TagEXIF, *PTagEXIF;

AJL_API
ARESULT  ajlSaveBGRToFile(AByte * image, //only support BRG Chunky format 
						  AInt width, 
						  AInt height, 
						  AInt pitch,
						  TagEXIF * pExif, 
						  AChar * dstFileName
						  );



AJL_API
ARESULT  ajlDecodeJPGFromMemoryToFile(AByte * pSrcData, AUInt imgsize, 
						  AChar * dstFileName, AUInt *nWidth, AUInt *nHeight);

AJL_API
ARESULT  ajlDecodeJPGFromMemoryToMemory(AByte * pSrcData, AUInt imgsize, 
						  AByte* * pdstdata, AUInt *nWidth, AUInt *nHeight);

#endif /* __AJL_INC__ */
