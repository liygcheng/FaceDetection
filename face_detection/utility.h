//some utilities 
//Author:  Lechao Cheng
//Date: 6/29/2016
//Email: liygcheng@zju.edu.cn
#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>
#include<string>
#include<fstream>
#include<vector>
#include<direct.h>//for  mkdir
//windows api to parse path
#include<Shlwapi.h>
//  char buffer_1[ ] = "c:\\path\\file"; 
//char *lpStr1;
//char *filename;
//lpStr1 = buffer_1;
//filename = PathFindFileName(lpStr1)
//windows api to parse path
//#include "landmark.pb.h"
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

#define TK_USE_OPENMP

//#define SHOW_FACE_INFO

namespace TK{

	class PBErrorCollector:public google::protobuf::compiler::MultiFileErrorCollector
	{
	public:
		PBErrorCollector(){};
		virtual ~PBErrorCollector(){};
		virtual void AddError(const string& filename, int line, int column, const string& message)
		{
			std::cerr << " An  Error Occurred!:" << std::endl;
			std::cerr << "filename = " << filename << std::endl;
			std::cerr << " line = " << line << std::endl;
			std::cerr << " column = " << column << std::endl;
			std::cerr << " message = " << message << std::endl;
		}
	private:

	};

	static size_t tk_get_filenames(const char* infolder, std::vector<std::string>& filenames, const char* suffix)
	{

		filenames.clear();
		_finddata64i32_t m_file;
		long m_lf;

		std::string m_path(infolder);
		m_path.append("/*.").append(suffix);

		if ((m_lf = _findfirst(m_path.c_str(), &m_file)) == -1l)
			std::cout << "No files  to be found! Please  check it again!" << std::endl;
		else
		{
			std::cout << "File  List :" << std::endl;

			while (_findnext(m_lf, &m_file) == 0)
			{

		
				std::string tmp(infolder);
				filenames.push_back(tmp.append("/").append(m_file.name));
				//std::cout <<m_file.name<< std::endl;
			}

			_findclose(m_lf);

		
			filenames.erase(filenames.begin()); // get rid of dir 

		}

		if (!filenames.size())
		{
			std::cout << "found  no  file " << std::endl;
		}
		else
		{
			std::cout << "found  files : " << filenames.size() << std::endl;
		}

		return filenames.size();
	}

	static bool tk_dump_filenames(const std::vector<std::string> &filenames,const char* outname,size_t baseCount = 0){


		std::cout << "filename = " << outname << std::endl;

		std::ofstream out(outname);

		if (!out) { std::cerr << "open file error,aborting.." << std::endl; return false; }

		for (std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
		{
			out << *it << " " <<(it - filenames.begin())+ baseCount<< std::endl;
		}
		out.close();

		std::cout << "filenames dump done!" << std::endl;


		return true;
	}
	
	static bool tk_dump_piecewise_filenames(const std::vector<std::string> &filenames,const char* outfolder,size_t numPerSegment){


		size_t file_len = filenames.size();

		assert(file_len);

		/// Generating configure.txt
		std::string constr(outfolder);

		std::ofstream  txt(constr.append("configure.txt"));

		std::cout << constr << std::endl;
		if (!txt)  { std::cerr << "open file error,aborting.." << std::endl; return false; }


		

		///

		size_t max_batch_num = file_len / (numPerSegment);

		txt << outfolder << " " << max_batch_num+1 << std::endl;

		for (size_t batch_num = 0; batch_num <= max_batch_num; ++batch_num)
		{

			size_t startIdx = batch_num * numPerSegment;
			size_t endIdx = min(max((size_t)0, (batch_num + 1)*numPerSegment - 1), file_len - 1);

			std::vector<std::string>  subfilename(&filenames[startIdx], &filenames[endIdx]);
			char* subname = new char[256]; //batch dump name
	
			memset(subname, 0, sizeof(subname));

			//std::cout << "outfolder = " << sizeof(outfolder) << std::endl;

			std::string tmp(outfolder);

			std::sprintf(subname, tmp.append("%d-%d.filenames").c_str(), startIdx, endIdx);

			if (_access(subname, 0) != -1)
			{
				std::cout << "File " << subname << " already exists.\n" << std::endl;
				txt << subname << " " << startIdx << " " << endIdx << std::endl;
				continue;
			}
			txt << subname <<" "<<startIdx<<" "<<endIdx<< std::endl;

			TK::tk_dump_filenames(subfilename, subname, startIdx);

		}

		txt.close();

		return true;
	}

	static void tk_copy_file(const char* in_file,const  char* out_file)
	{
		if (!CopyFile(in_file, out_file, FALSE))//注意：使用时将 “配置属性->常规->字符集” 改为多字节
		{
			printf("Copy file error : %x\n", GetLastError());
		}

	}

	static void tk_check_for_dir(const char* path,int flag)
	{
		
	

		WIN32_FIND_DATAA fd;
		memset(&fd, 0, sizeof(fd));
		HANDLE hFind = FindFirstFileA(path, &fd);
		if (hFind != INVALID_HANDLE_VALUE && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			return;
		}

		printf("mkdir state = %d\n", _mkdir(path));;

		//if (hFind == INVALID_HANDLE_VALUE) {
		//	//printf("Invalid File Handle. Get Last Error reports %d ", GetLastError());
		//	if (flag == 1)
		//	{
		//		printf("mkdir state = %d\n", _mkdir(path));;

		//	}
		//}
		//else {
		//	printf("The first file found is %s ", fd.cFileName);
		//	system("pause");
		//	FindClose(hFind);
		//}
		//


		

	}

	static void PB_Initialize(const char* filename, google::protobuf::compiler::Importer*& importer,const google::protobuf::FileDescriptor*& filedescripter){

		TK::PBErrorCollector errorCollector ;
		google::protobuf::compiler::DiskSourceTree diskSourceTree;


	   importer = new google::protobuf::compiler::Importer(&diskSourceTree,&errorCollector);

	   diskSourceTree.MapPath("", "./protobuf");

	   filedescripter = importer->Import(filename);

	
	}

	static bool PB_Writer(const char* filename,google::protobuf::Message*& message)
	 {

		 std::cout << "filename = " << filename << std::endl;
		 
		 std::fstream outfile(filename, std::ios::out | std::ios::binary);
		 
		 if (!outfile)
		 {
			 std::cout << "Open File Error" << std::endl;
		 }

		 if (!message->SerializeToOstream(&outfile)) {
			 std::cerr << "Failed to write msg." << std::endl;
			 return false;
		 }
		 outfile.close();
		 return true;
	 }

	static bool PB_Reader(const char* filename,google::protobuf::Message*& message)
	 {


		std::fstream input(filename, std::ios::in | std::ios::binary);
		if (!input)
		{
			std::cout << "Open File Error" << std::endl;
		}
		 if (!message->ParseFromIstream(&input)) {
			 std::cerr << "Failed to parse message." << std::endl;
			 input.close();
			 return false;
		 }
		 input.close();
		 return true;
	 }

	static void PB_Print(const google::protobuf::Message& message)
	{
	


	}

	static bool tk_centralization(const google::protobuf::Message* & msg,cv::Mat_<double>& center,cv::Mat_<double>& raw )
	{
		//center.release();
		//raw.release();

		

		const google::protobuf::Descriptor* des = msg->GetDescriptor();
		const google::protobuf::Reflection* ref = msg->GetReflection();

		const google::protobuf::FieldDescriptor* field = NULL;

		field = des->FindFieldByName("state");

		bool  m_state = ref->GetBool(*msg, field);

		if (m_state)
		{
			// get element
			 field = des->FindFieldByName("box");
			 const google::protobuf::Message* box =&( ref->GetMessage(*msg, field));
			
			 double startX = static_cast<double>(box->GetReflection()->GetInt32(*box, box->GetDescriptor()->FindFieldByName("startX")));
			 double endX   = static_cast<double>(box->GetReflection()->GetInt32(*box, box->GetDescriptor()->FindFieldByName("endX")));
			 double startY = static_cast<double>(box->GetReflection()->GetInt32(*box, box->GetDescriptor()->FindFieldByName("startY")));
			 double endY   = static_cast<double>(box->GetReflection()->GetInt32(*box, box->GetDescriptor()->FindFieldByName("endY")));

			 //
			 double  width     = (endX - startX);
			 double height     = (endY - startY);
			 double center_X   = startX + width / 2.0;
			 double center_Y   = startY + height / 2.0;
			 //landmark
			
			 field = des->FindFieldByName("landmark");
			 
			 size_t len = ref->FieldSize(*msg, field);

			 raw    = cv::Mat::zeros(len, 2, CV_64FC1);
			 center = cv::Mat::zeros(len, 2, CV_64FC1);

			

			 for (size_t t = 0; t < len;++t)
			 {
				 const google::protobuf::Message* landmark = &(ref->GetRepeatedMessage(*msg, field, t));
				 raw[t][0] =static_cast<double>( landmark->GetReflection()->GetInt32(*landmark, landmark->GetDescriptor()->FindFieldByName("X")));
				 raw[t][1] =static_cast<double>( landmark->GetReflection()->GetInt32(*landmark, landmark->GetDescriptor()->FindFieldByName("Y")));
			
				 center[t][0] = (raw[t][0] - center_X) / (width / 2.0);
				 center[t][1] = (raw[t][1] - center_Y) / (height / 2.0);

				// std::cout << "x = " << center[t][0] << std::endl;
				// std::cout << "y = " << center[t][0] << std::endl;

			 }

#ifdef SHOW_FACE_INFO

			 std::string filename = ref->GetString(*msg, des->FindFieldByName("filename"));
			 cv::Mat m_image = cv::imread(filename);
			 if (!m_image.data)
				 return false;
			 if (m_image.channels() != 3)  //  get rid of non-color  images
				 return false;


			 for (int i = 0; i < raw.rows; ++i)
			 {			
				 cv::circle(m_image,cv::Point2d(raw[i][0],raw[i][1]), 3, CV_RGB(255, 0, 0));
			 }
	
			 cv::namedWindow("Face Detect");		
			 cv::imshow("Face Detect", m_image);
			 cv::waitKey(0);



#endif



			 
		}


		return m_state;

	}
	 





}