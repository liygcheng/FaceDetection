#include"HairSelection.h"

#define MAX_PRE_ALLOCATE_SIZE 100100
#define  SHOWFACE
//#define  SHOWLOGINFO
#ifndef SHOWLOGINFO
#define SHOWERRORINFO
#endif
//Main entry
bool HairParser::Run(void)
{
	//step 1: get filenames
	ElicitFilenamesFromInfolder(); //get filenames

	int fileLen = m_filenames.size();

	//check out  for null
	if (!fileLen) 
	{
		std::cerr << "No files  in folder" << std::endl; 
		return false;
	}

	//step 2: for each filename ,detect face num

#pragma omp parallel for   //  Here use multi-thread to accelerate  the  process
	for (int i = 0; i < fileLen; ++i)
	{

		std::string filename = m_filenames[i];
		std::string basename = m_basename[i];
		

		int faceNum = DetectMaxFaceNum(filename);

		if (faceNum != 1)
		{
		#ifdef SHOWERRORINFO
			std::cout << "count = "<<i<<"  filename = "<<basename << std::endl;
			CopyImage(filename, std::string("C:/Project/data/portrait/residual/").append(basename));
		#endif
			continue;
		}

		// transfer  image
		std::string outfile(m_outfolder);

#ifdef SHOWLOGINFO
		std::cout << basename << " ------> ";
#endif
		

		CopyImage(filename, outfile.append(basename)); //  copy file  to  corresponding  folder
		

	}



	return true;
}


//Public Functions

bool HairParser::InitParser(void)
{
	m_filenames.reserve(MAX_PRE_ALLOCATE_SIZE);
	m_basename.reserve(MAX_PRE_ALLOCATE_SIZE);


	if (m_infolder.empty())
	{
		std::cerr << "m_infolder  is  empty " << std::endl;
		return false;
	}
	if (m_outfolder.empty())
	{
		std::cerr << "m_outfolder  is empty " << std::endl;
		return false;
	}

	//DISPLAY::NOT_SHOW_FACES; // show faces  or not 
	//TRANSFER::TRANSFER_IAMGE; // copy images or not

	return true;
}

bool HairParser::ImageSizeCheck(int & height, int & width)
{
	return (width >= 300 && height >= 225);
}
	

bool HairParser::ImageDefinitionCheck(cv::Mat& grayimage)// detect definition ratio  清晰度检测
{
	double sum = 0;
	int h = grayimage.rows;
	int w = grayimage.cols;

	int step = grayimage.step / sizeof(uchar);

	uchar* data = grayimage.data;

	for (int i = 0; i < h - 1; i++)
	{
		for (int j = 0; j < w - 1; j++)
		{
			sum += sqrt((pow((double)(data[(i + 1)*step + j] - data[i*step + j]), 2) + pow((double)(data[i*step + j + 1] - data[i*step + j]), 2)));
			sum += abs(data[(i + 1)*step + j] - data[i*step + j]) + abs(data[i*step + j + 1] - data[i*step + j]);
		}
	}
	double cast = sum / (h * w);// >= 14 is ok

	printf("cast : %f\n", cast);

	system("pause");
	
	return cast>=14.0;
}


size_t HairParser::DetectMaxFaceNum(const std::string& filename)
{
	cv::Mat m_image = cv::imread(filename);
	if (!m_image.data)
		return 0;
	

	cv::Mat m_gray;

	cv::cvtColor(m_image, m_gray, cv::COLOR_BGR2GRAY);

	

	// for  some check
	if (!ImageSizeCheck(m_image.rows, m_image.cols) ) //get rid of small images
		return 0;

	if (m_image.channels() != 3)  //  get rid of non-color  images
		return 0;

	if (m_gray.data)
		m_gray.release();

	//for some check

	vecR m_rect;



	int w = m_image.cols - (m_image.cols & 1);
	int h = m_image.rows - (m_image.rows & 1);

	cv::Mat tmp = m_image(Rect(0, 0, w, h));



	CxFaceDA::ArcSoftFaceDetection(tmp, 2, 5, m_rect);

	if (m_rect.empty()) // no face
	{
		m_image.release();
		tmp.release();
		return 0;
	}
	
#ifdef SHOWFACE

	printf("area = %lf\n",1.0*m_rect[0].area()/(m_image.rows*m_image.cols));
	rectangle(tmp, m_rect[0], CV_RGB(0, 255, 0));
	cv::namedWindow("Face Detect");
	//cvNamedWindow("MyImg");
	cv::imshow("Face Detect", tmp);
	//cvShowImage("Face Detect", tmp);

	cv::waitKey(0);

#endif // SHOWFACE





	
	//system("pause");

	m_image.release();
	tmp.release();

	return m_rect.size();
}


//Private Functions
void HairParser::ElicitFilenamesFromInfolder(void)
{
	_finddata64i32_t m_file;
	long m_lf;

	std::string m_path(m_infolder);
	m_path.append("/*.*");

	if ((m_lf = _findfirst(m_path.c_str(), &m_file)) == -1l)
		std::cout << "No files  to be found! Please  check it again!" << std::endl;
	else
	{
		std::cout << "File  List :" << std::endl;

		while (_findnext(m_lf,&m_file) ==0)
		{
			m_basename.push_back(m_file.name);
			std::string tmp(m_infolder);
			m_filenames.push_back(tmp.append("/").append(m_file.name));
			//std::cout <<m_file.name<< std::endl;
		}

		_findclose(m_lf);

		m_basename.erase(m_basename.begin()); //  get rid of dir
		m_filenames.erase(m_filenames.begin()); // get rid of dir 

	}

	if (!m_basename.size())
	{
		std::cout << "found  no  file " << std::endl;
	}
	else
	{
		std::cout << "found  files : "<<m_basename.size() << std::endl;
	}

}



void HairParser::CopyImage(const std::string infile, const std::string outfile)
{
	
	if (!CopyFile(infile.c_str(), outfile.c_str(), FALSE))//注意：使用时将 “配置属性->常规->字符集” 改为多字节
	{
		printf("Copy file error : %x\n", GetLastError());
	}
#ifdef SHOWLOGINFO
	std::cout << "		done!" << std::endl;
#endif
	

}

//void HairParser::CopyFiles(const std::string infolder, const std::string outfolder)
//{
//
//}
//
//void HairParser::CopyFiles(const std::vector<std::string> fileVec, const std::string outfolder)
//{
//
//
//}


//deconstructor
HairParser::~HairParser()
{
	m_basename.clear();
	m_filenames.clear();
}