
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
#include <time.h>

extern "C"
{
#include <vl/generic.h>
#include <vl/kmeans.h>
}

#include "utility.h"


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
	
	Detector(const char* infolder, const char* filename, const char* outfolder = "./data/") :m_infolder(infolder), m_dumpname(filename), m_outfolder(outfolder)
	{
		if (!Initialize())
		{
			exit(-1);
		}
	
	};
	virtual ~Detector();

public:
	inline void setInfolder(const char* infolder){ m_infolder.assign(infolder); }
	inline void setOutfolder(const char* outfolder){ m_outfolder.assign(outfolder); }

public:
	virtual void ElicitFilenamesFromInfolder(void); // get filenames(full path) from  in folder
	virtual void CopyImage(const std::string infile, const std::string outfile);// single file  copy

	virtual void DetectMessage();

	virtual bool ElicitMessage(const char* in_proto_name);


	inline void Clear(void){

		m_filename.clear();
	    m_basename.clear();


		if(!m_faceinfos) m_faceinfos->Clear();

	}


	inline google::protobuf::Message* & GetMessagePointer()
	{
		return m_faceinfos;
	}
		
	
	static  void*  PartialDetectMessage(void* reg);

private:

	virtual bool Initialize(void);

	


private:
	Detector();

private:

	enum 
	{
		PATCH_WISE_NUM = 10000,
		MAX_THREADS_NUM = 10
	};

	//for  file 
	std::string  m_infolder;
	std::string  m_outfolder;
	//std::string  m_protoname;
	std::string  m_dumpname;
    std::vector<std::string>  m_filename;
	std::vector<std::string>  m_basename;

	//  for protocol  buffer archive use
	google::protobuf::compiler::Importer* m_importer = NULL; //  for every .proto file
	const google::protobuf::FileDescriptor* m_filedec = NULL;
	const google::protobuf::FieldDescriptor* m_field = NULL;

	const google::protobuf::Descriptor* m_faceinfos_des = NULL;
	const google::protobuf::Descriptor* m_faceinfo_des = NULL;
	const google::protobuf::Descriptor* m_landmark_des = NULL;
	const google::protobuf::Descriptor* m_boundingbox_des = NULL;

	google::protobuf::MessageFactory * m_messageFactory = NULL;

	 google::protobuf::Message* m_faceinfos = NULL;
	 google::protobuf::Message* m_faceinfo = NULL;
	 google::protobuf::Message* m_landmark = NULL;
	 google::protobuf::Message* m_boundingbox = NULL;


	const google::protobuf::Reflection* m_faceinfos_ref = NULL;
	const google::protobuf::Reflection* m_faceinfo_ref = NULL;
	const google::protobuf::Reflection* m_landmark_ref = NULL;
	const google::protobuf::Reflection* m_boundingbox_ref = NULL;









	






};




