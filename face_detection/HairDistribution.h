
/************************************************************************/
/* 
Author: Lechao Cheng
Date: 7/1/2016
update:7/4/2016
Email: liygcheng@zju.edu.cn
*/
/************************************************************************/


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <io.h>
#include <opencv.hpp>
#include "CxFaceDA.h"


extern "C"
{
#include <vl/generic.h>
#include <vl/kmeans.h>
}

#define myrandom(x) ((rand()%(x+1))*1.0/(x+1)) 


class Distributor  // hair  Distributor
{
public:
	Distributor(const char* infolder, const char* outfolder, const int K) :m_infolder(infolder), m_outfolder(outfolder), m_K(K)
	{
	  if (!initialConfig())
	  {
		  std::cerr << "initial error , aborting\n" << std::endl;
		  exit(-1);
	  }
	
	};
	~Distributor();

public:
	void calcFeature(void);
	void perfromKmeans(void);

	inline int getDataNum(void){ return m_dataNum; }
	inline vl_uint32 * & getAssignment(void){ return m_dataAssignment; }
	void detectKeyPoints(const std::string filename);
private:
	Distributor(); // forbid
	bool initialConfig(void);



private:

	std::string m_infolder;
	std::string m_outfolder;

	std::vector<std::string> m_filenames;
	std::vector<std::string> m_basename;

	double* m_featureData; //m_featureData[][];

	const int m_featureDim = 16;

	int m_dataNum;
	
	int m_K;
	double m_energy;

	VlKMeans* m_kmeans;
	vl_uint32 * m_dataAssignment;// range from 1 ~ m_dataNum
	double* m_distance;


};

class Detector
{
public:
	
	Detector(const char* infolder, const char* filename, const char* outfolder = NULL) :m_infolder(infolder), m_filename(filename), m_outfolder(outfolder)
	{
		if (!Initialize())
		{
			exit(-1);
		}
	
	};
	virtual ~Detector();

private:
	virtual bool Initialize(void)
	{


		return true;
	}

private:
	Detector();

private:
	std::string  m_infolder;
	std::string  m_outfolder;
	std::string  m_filename;

};




