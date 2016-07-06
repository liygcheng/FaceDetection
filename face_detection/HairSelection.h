
/************************************************************************/
/* 
Author:  Lechao Cheng
Date: 6/30/2016
Email: liygcheng@zju.edu.cn
*/
/************************************************************************/
#pragma once

#include <iostream>
#include <opencv.hpp>
#include <omp.h>  
#include<io.h>
#include"CxFaceDA.h"
#include"utility.h"
#include<exception>


static void CheckForPath(const char* path)
{
	TK::tk_check_for_dir(path, 1); //1 for mkdir if empty
}


class HairParser
{
public:
	
	HairParser(const char* infolder, const char* outfolder) :
		m_infolder(infolder), m_outfolder(outfolder)
	{

		if (!InitParser())
			std::cout << "some thing error in Initial step" << std::endl;

	}

	~HairParser();

public:

	inline void setInfolder(const char* infolder){ m_infolder.assign(infolder);}
	inline void setOutfolder(const char* outfolder){ m_outfolder.assign(outfolder); }

public:

	bool InitParser(void);

	bool ImageSizeCheck(int & height,int & width); // check size and get h,w 

	bool ImageDefinitionCheck(cv::Mat& grayimage);//check  for  definition

	size_t DetectMaxFaceNum(const std::string& filename);//return max found faces num

	bool Run(void); // main  framework



private: // some utilities in class

	HairParser(){};//forbid

	void ElicitFilenamesFromInfolder(void); // get filenames(full path) from  infolder

	void CopyImage(const std::string infile,const std::string outfile);// single file  copy


private:

	std::string m_infolder;//input image folder to handle
	std::string m_outfolder;// dump results from infolder

	std::vector<std::string> m_filenames;//full path,better to reserve enough capacity

	std::vector<std::string> m_basename;//  basename  , not   full path
};





