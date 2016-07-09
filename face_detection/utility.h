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

	static void PB_Initialize(const char* root, const char* filename, google::protobuf::compiler::Importer*& importer){

		TK::PBErrorCollector errorCollector ;
		google::protobuf::compiler::DiskSourceTree diskSourceTree;


	   importer = new google::protobuf::compiler::Importer(&diskSourceTree,&errorCollector);

	   diskSourceTree.MapPath("", root);
	   importer->Import(filename);

	
	}

	static bool PB_Writer(const char* filename,google::protobuf::Message*& message)
	 {
		 std::string rootpath("./data/");
		 rootpath = rootpath.append(filename);
		 std::cout << "rootpath = " << rootpath << std::endl;
		 
		 std::fstream outfile(rootpath, std::ios::out | std::ios::binary);
		 
		 if (!message->SerializeToOstream(&outfile)) {
			 std::cerr << "Failed to write msg." << std::endl;
			 return false;
		 }
		 outfile.close();
		 return true;
	 }

	static bool PB_Reader(const char* filename,google::protobuf::Message& message)
	 {
		std::string rootpath("./data/");
		rootpath = rootpath.append(filename);

		std::fstream input(rootpath, std::ios::in | std::ios::binary );
		 if (!message.ParseFromIstream(&input)) {
			 std::cerr << "Failed to parse message." << std::endl;
			 return false;
		 }
		 return true;
	 }

	static void PB_Print(const google::protobuf::Message& message)
	{
		//std::cout << "********************************************" << std::endl;
		//std::cout << " state    = "<<message.state()<< std::endl;

		//std::cout << " filename = "<<message.filename() << std::endl;

		//std::cout << " basename = "<<message.basename() << std::endl;

		//std::cout << " startX = "<<message.box().startx() <<" endX = "<<message.box().endx()<< std::endl;
		//std::cout << " startY = " << message.box().starty() <<" endY = "<<message.box().endy()<< std::endl;
		//std::cout << " centroidX = "<<message.box().centroidx() <<" centroidY = "<<message.box().centroidy() << std::endl;
		//std::cout << " width = "<<message.box().width() <<" height = "<<message.box().height() << std::endl;


		//std::cout << " Face index =  "<<message.GetCachedSize()<< std::endl;

		//std::cout << "********************************************" << std::endl;

	}


	 

}