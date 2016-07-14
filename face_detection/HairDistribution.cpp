#include "HairDistribution.h"

//#define  SHOW_KEY_POINTS
#define LOG_INFO
//Distributor Class
Distributor::~Distributor()
{
	if (!m_filenames.empty())
		m_filenames.clear();
	if (!m_basename.empty())
		m_basename.clear();
	if (!m_featureData)
		delete m_featureData;
	
	if (!m_dataAssignment)	
		delete m_dataAssignment;
	if (!m_distance)
		delete m_distance;

	vl_kmeans_delete(m_kmeans);
	
}

bool Distributor::initialConfig(void)
{
	//step 1:   set up filenames 
	_finddata64i32_t m_file;
	long m_lf;

	std::string m_path(m_infolder);
	m_path.append("/*.*");

	if ((m_lf = _findfirst(m_path.c_str(), &m_file)) == -1l)
		std::cout << "No files  to be found! Please  check it again!" << std::endl;
	else
	{
		std::cout << "File  List :" << std::endl;

		while (_findnext(m_lf, &m_file) == 0)
		{
			m_basename.push_back(m_file.name);
			std::string tmp(m_infolder);
			m_filenames.push_back(tmp.append("/").append(m_file.name));
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
		std::cout << "found  files : " << m_basename.size() << std::endl;
	}

	m_dataNum = m_basename.size();


	assert(m_dataNum && m_featureDim && m_K);


	//step 2:   allocate space
	m_featureData = new double[m_dataNum*m_featureDim];

//#pragma omp parallel for
	//for (int i = 0; i < m_dataNum; ++i)
	//	for (int j = 0; j < m_featureDim;++j)
	//	{
	//	m_featureData[i*m_featureDim + j] = 
	//	}
	//	/*m_featureData[i] = new double[m_featureDim];*/



	//step 3:   initial  kmeans

	m_kmeans = vl_kmeans_new(VL_TYPE_DOUBLE, VlDistanceL2);

	vl_kmeans_set_algorithm(m_kmeans, VlKMeansLloyd);
	vl_kmeans_init_centers_plus_plus(m_kmeans, m_featureData, m_featureDim, m_dataNum, m_K);
	vl_kmeans_set_max_num_iterations(m_kmeans, 500);
	vl_kmeans_set_num_repetitions(m_kmeans, 50);


	m_dataAssignment = static_cast<vl_uint32*>(vl_malloc(sizeof(vl_uint32) * m_dataNum));
	m_distance = static_cast<double*>(vl_malloc(sizeof(double) * m_dataNum));

	return true;
}

void Distributor::perfromKmeans(void)
{

	vl_kmeans_cluster(m_kmeans, m_featureData, m_featureDim, m_dataNum, m_K);
	//vl_kmeans_refine_centers(m_kmeans, m_featureData, m_K);
	m_energy = vl_kmeans_get_energy(m_kmeans);

	printf("energy  = %lf \n", m_energy);

	vl_kmeans_quantize(m_kmeans, m_dataAssignment, m_distance, m_featureData, m_dataNum);


}

void Distributor::calcFeature(void)
{


#pragma omp parallel for
	for (int i = 0; i < m_dataNum; ++i)
		for (int j = 0; j < m_featureDim; ++j)
			m_featureData[i*m_featureDim + j] = myrandom(i + j);
		
}

void Distributor::detectKeyPoints(const std::string filename)
{
	cv::Mat m_image = cv::imread(filename);
	if (!m_image.data)
		return ;

	if (m_image.channels() != 3)  //  get rid of non-color  images
		return ;

	//for some check
	vecR m_rect;

	int w = m_image.cols - (m_image.cols & 1);
	int h = m_image.rows - (m_image.rows & 1);

	cv::Mat tmp = m_image(Rect(0, 0, w, h));

	vecP2d  m_keyPoints;

	CxFaceDA::ArcSoftFaceAlignment(tmp, 5, m_keyPoints);

#ifdef SHOW_KEY_POINTS

	for (unsigned int i = 0; i < m_keyPoints.size(); ++i)
	{
		cv::Vec2f tmp = m_keyPoints[i];
		cv::circle(m_image, cv::Point(static_cast<int>(tmp[0]),static_cast<int>(tmp[1])), 3, CV_RGB(255, 0, 0));
	}

	//rectangle(tmp, m_rect[0], CV_RGB(0, 255, 0));
	cv::namedWindow("Face Detect");
	//cvNamedWindow("MyImg");
	cv::imshow("Face Detect", tmp);
	//cvShowImage("Face Detect", tmp);

	cv::waitKey(0);


#endif // SHOW_KEY_POINTS




}

// Detector Class

bool Detector::Initialize(void)
{


	// initialize
	m_filename.reserve(20000);
	m_basename.reserve(20000);

	//protobuf initial

	TK::PBErrorCollector errorCollector;
	google::protobuf::compiler::DiskSourceTree diskSourceTree;
	m_importer = new google::protobuf::compiler::Importer(&diskSourceTree, &errorCollector);

	diskSourceTree.MapPath("", "./protobuf");

	m_filedec = m_importer->Import("landmark.proto");



	m_faceinfos_des = m_filedec->pool()->FindMessageTypeByName("FaceInfos");
	m_boundingbox_des = m_filedec->pool()->FindMessageTypeByName("BoundingBox");

	m_messageFactory = new google::protobuf::DynamicMessageFactory(m_filedec->pool());



	return true;
}

void Detector::ElicitFilenamesFromInfolder(void)
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

		while (_findnext(m_lf, &m_file) == 0)
		{
			
			m_basename.push_back(m_file.name);
			std::string tmp(m_infolder);
			m_filename.push_back(tmp.append("/").append(m_file.name));
			//std::cout <<m_file.name<< std::endl;
		}

		_findclose(m_lf);

		m_basename.erase(m_basename.begin()); //  get rid of dir
		m_filename.erase(m_filename.begin()); // get rid of dir 

	}

	if (!m_basename.size())
	{
		std::cout << "found  no  file " << std::endl;
	}
	else
	{
		std::cout << "found  files : " << m_basename.size() << std::endl;
	}



}

void Detector::CopyImage(const std::string infile, const std::string outfile)
{
	if (!CopyFile(infile.c_str(), outfile.c_str(), FALSE))//注意：使用时将 “配置属性->常规->字符集” 改为多字节
	{
		printf("Copy file error : %x\n", GetLastError());
	}
#ifdef SHOWLOGINFO
	std::cout << "		done!" << std::endl;
#endif


}

void Detector::DetectMessage(void)
{
	// step 1:  for every image

	size_t file_len = m_filename.size();

	assert(file_len);

	size_t max_batch_num = file_len / (PATCH_WISE_NUM);

	time_t td = time(0);
	tm* ltm = localtime(&td);

	
	for (size_t batch_num = 0; batch_num <= max_batch_num; ++batch_num)
	{

		
		//////////////////////////////////////////////////////////////////////////
		size_t  startIdx = batch_num * PATCH_WISE_NUM;

		size_t  endIdx = std::min(std::max((size_t)0,(batch_num + 1)*PATCH_WISE_NUM - 1), file_len-1);


		std::vector<std::string>  subfilename(&m_filename[startIdx], &m_filename[endIdx]);
		std::vector<std::string>  subbasename(&m_basename[startIdx], &m_basename[endIdx]);

		char* subname = new char[256]; //batch dump name
		memset(subname,0,sizeof(subname));
		std::string tmp;
		std::sprintf(subname, tmp.append("./data/FaceInfos/").append(m_dumpname).append("_%d-%d_%d_%d_%d").c_str(),startIdx, endIdx, ltm->tm_mon + 1, ltm->tm_mday,ltm->tm_year+1900);

		if (_access(subname, 0) != -1)
		{
			std::cout << "File " << subname << " already exists.\n" << std::endl;
			continue;
		}
	
		
		//////////////////////////////////////////////////////////////////////////

		//std::cout << "subname = " << subname << std::endl;
		//////////////////////////////////Log info////////////////////////////////////////

		std::cout << "Handling Batch " << batch_num+1<<" ----->"<<subname << std::endl;

		/////////////////////////////////Log info//////////////////////////////////////

		size_t len = subbasename.size();

		m_faceinfos = m_messageFactory->GetPrototype(m_faceinfos_des)->New();
		m_faceinfos_ref = m_faceinfos->GetReflection();// is this so called Reflection ?

		m_faceinfos_ref->SetString(m_faceinfos, m_faceinfos_des->FindFieldByCamelcaseName("rootFolder"), m_infolder);


		for (size_t i = 0; i < len; ++i)
		{
			//if (i% ==0 )
			std::cout << "count = " << i << std::endl;

			m_field = m_faceinfos_des->FindFieldByName("info");

			m_faceinfo = m_faceinfos_ref->AddMessage(m_faceinfos, m_field);
			m_faceinfo_des = m_faceinfo->GetDescriptor();
			m_faceinfo_ref = m_faceinfo->GetReflection();


			m_boundingbox = m_messageFactory->GetPrototype(m_boundingbox_des)->New();
			m_boundingbox_ref = m_boundingbox->GetReflection();


			std::string filename = subfilename[i];
			std::string basename = subbasename[i];

			m_field = m_faceinfo_des->FindFieldByName("basename");
			m_faceinfo_ref->SetString(m_faceinfo, m_field, basename);

			cv::Mat m_image = cv::imread(filename);
			if ((!m_image.data) || (m_image.channels() != 3)) // state = false;
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);

				continue;
			}


			int w = m_image.cols - (m_image.cols & 1);
			int h = m_image.rows - (m_image.rows & 1);
			cv::Mat tmp = m_image(Rect(0, 0, w, h));


			vecR m_rect;
			CxFaceDA::ArcSoftFaceDetection(tmp, 2, 5, m_rect);

			if (m_rect.size() != 1)
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);
				continue;
			}

			//std::cout <<" sx = "<< m_rect[0].tl().x << " sy = " << m_rect[0].tl().y<< std::endl;
			//std::cout << " ex = " << m_rect[0].br().x << " ey = " << m_rect[0].br().y << std::endl;


			// Set up bounding box
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startX"), m_rect[0].tl().x);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startY"), m_rect[0].tl().y);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endX"), m_rect[0].br().x);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endY"), m_rect[0].br().y);
			//m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidX"), (m_rect[0].tl().x + m_rect[0].br().x) / 2);
			//m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidY"), (m_rect[0].tl().y + m_rect[0].br().y) / 2);
			//m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("width"), m_rect[0].br().x - m_rect[0].tl().x);
			//m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("height"),m_rect[0].br().y - m_rect[0].tl().x);

			m_field = m_faceinfo_des->FindFieldByName("box");
			m_faceinfo_ref->SetAllocatedMessage(m_faceinfo, m_boundingbox, m_field);

			vecP2d  m_keyPoints;
			CxFaceDA::ArcSoftFaceAlignment(tmp, 5, m_keyPoints);

			size_t key_len = m_keyPoints.size();
			if (key_len == 0)
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);
				continue;
			}

			for (size_t k = 0; k < key_len; ++k)
			{
				m_field = m_faceinfo_des->FindFieldByName("landmark");

				m_landmark = m_faceinfo_ref->AddMessage(m_faceinfo, m_field);
				m_landmark_des = m_landmark->GetDescriptor();
				m_landmark_ref = m_landmark->GetReflection();

				m_field = m_landmark_des->FindFieldByName("id");
				m_landmark_ref->SetUInt32(m_landmark, m_field, k + 1);

				m_field = m_landmark_des->FindFieldByName("X");
				m_landmark_ref->SetInt32(m_landmark, m_field, static_cast<google::protobuf::int32>(m_keyPoints[k][0]));

				m_field = m_landmark_des->FindFieldByName("Y");
				m_landmark_ref->SetInt32(m_landmark, m_field, static_cast<google::protobuf::int32>(m_keyPoints[k][1]));

			}


			m_field = m_faceinfo_des->FindFieldByName("state");
			m_faceinfo_ref->SetBool(m_faceinfo, m_field, true);



			//Log info

#ifdef  LOG_INFO





			//Log info
#endif
			//filename.clear();
			//basename.clear();
			m_image.release();
			m_rect.clear();
			m_keyPoints.clear();
			tmp.release();

		}




		TK::PB_Writer(subname, m_faceinfos);
		if (!m_faceinfos) m_faceinfos->Clear();
		//Clear();



	}
	Clear();
	system("pause");
//#pragma  omp parallel for


	

#ifdef SHOW_KEY_POINTS

	for (unsigned int i = 0; i < m_keyPoints.size(); ++i)
	{
		cv::Vec2f tmp = m_keyPoints[i];
		cv::circle(m_image, cv::Point(static_cast<int>(tmp[0]), static_cast<int>(tmp[1])), 3, CV_RGB(255, 0, 0));
	}

	//rectangle(tmp, m_rect[0], CV_RGB(0, 255, 0));
	cv::namedWindow("Face Detect");
	//cvNamedWindow("MyImg");
	cv::imshow("Face Detect", tmp);
	//cvShowImage("Face Detect", tmp);

	cv::waitKey(0);


#endif // SHOW_KEY_POINTS

}

void Detector::DetectMessageByPiecewise(const char* configurefile)
{
	std::ifstream config(configurefile);

	if (!config){ std::cerr << "Open configure file error." << std::endl; return; }

	std::string imageFolder,imageFiles;

	size_t batchNum, startIdx, endIdx;

	//std::getline(config, imageFolder);

	config >> imageFolder >> batchNum;

	while (config.peek() != EOF)
	{
		//  get image names  from configure files
		config >> imageFiles >> startIdx >> endIdx;

		std::cout << "imageFiles = " << imageFiles << std::endl;
		std::ifstream patchFile(imageFiles);

		if (!patchFile) { std::cerr << "Open filenames file error." << std::endl; continue; }

		std::vector<std::string > imagenames;
		imagenames.reserve(PATCH_WISE_NUM + 40);
		std::string tmp;
		size_t idx;


		while (patchFile.peek()!= EOF)
		{
			patchFile >> tmp >> idx;
			imagenames.push_back(tmp);
		}

		//  get image names  from configure files


		std::cout << "imagenames : " << imagenames.size() << std::endl;
		// generating dump string name
		imageFolder.pop_back(); //get rid of /

		std::string subname;
		size_t dumpiter = imageFolder.find_last_of("/");

		if (dumpiter == imageFolder.size())
		{
			std::cout << "Find   no  /" << std::endl;
			continue;
		}
		subname.assign(imageFolder.substr(0, dumpiter));
		//std::cout << "root path : " << subname.append("/imageinfos/") << std::endl;

		imageFolder.push_back('/');// add /


		char* subname_ = new char[256]; //batch dump name
		memset(subname_, 0, sizeof(subname_));
		
		std::sprintf(subname_, subname.append("/imageinfos/%d-%d.faceinfos").c_str(), startIdx, endIdx);

		// generating dump string name

		std::cout << "subname_  = " << subname_ << std::endl;

		if (_access(subname_, 0) != -1)
		{
			std::cout << "File " << subname_ << " already exists.\n" << std::endl;
			continue;
		}



		m_faceinfos = m_messageFactory->GetPrototype(m_faceinfos_des)->New();
		m_faceinfos_ref = m_faceinfos->GetReflection();// is this so called Reflection ?

		m_faceinfos_ref->SetString(m_faceinfos, m_faceinfos_des->FindFieldByName("rootFolder"), imageFolder);

		size_t len = imagenames.size();

		for (size_t i = 0; i < len; ++i)
		{
			//if (i% ==0 )
			std::cout << "count = " << i << " batch = "<<startIdx <<" -- "<<endIdx<< std::endl;

			m_field = m_faceinfos_des->FindFieldByName("info");

			m_faceinfo = m_faceinfos_ref->AddMessage(m_faceinfos, m_field);
			m_faceinfo_des = m_faceinfo->GetDescriptor();
			m_faceinfo_ref = m_faceinfo->GetReflection();


			m_boundingbox = m_messageFactory->GetPrototype(m_boundingbox_des)->New();
			m_boundingbox_ref = m_boundingbox->GetReflection();


			std::string filename = imagenames[i];
			//std::string basename = subbasename[i];

			m_field = m_faceinfo_des->FindFieldByName("filename");
			m_faceinfo_ref->SetString(m_faceinfo, m_field, filename);

			cv::Mat m_image = cv::imread(filename);
			if ((!m_image.data) || (m_image.channels() != 3)) // state = false;
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);

				continue;
			}


			int w = m_image.cols - (m_image.cols & 1);
			int h = m_image.rows - (m_image.rows & 1);
			cv::Mat tmp = m_image(Rect(0, 0, w, h));


			vecR m_rect;
			CxFaceDA::ArcSoftFaceDetection(tmp, 2, 5, m_rect);

			if (m_rect.size() != 1)
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);
				continue;
			}

			// Set up bounding box
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startX"), m_rect[0].tl().x);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startY"), m_rect[0].tl().y);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endX"), m_rect[0].br().x);
			m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endY"), m_rect[0].br().y);
			//m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidX"), (m_rect[0].tl().x + m_rect[0].br().x) / 2);
			//m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidY"), (m_rect[0].tl().y + m_rect[0].br().y) / 2);
			//m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("width"), m_rect[0].br().x - m_rect[0].tl().x);
			//m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("height"),m_rect[0].br().y - m_rect[0].tl().x);

			m_field = m_faceinfo_des->FindFieldByName("box");
			m_faceinfo_ref->SetAllocatedMessage(m_faceinfo, m_boundingbox, m_field);

			vecP2d  m_keyPoints;
			CxFaceDA::ArcSoftFaceAlignment(tmp, 5, m_keyPoints);

			size_t key_len = m_keyPoints.size();
			if (key_len == 0)
			{
				m_field = m_faceinfo_des->FindFieldByName("state");
				m_faceinfo_ref->SetBool(m_faceinfo, m_field, false);
				continue;
			}

			for (size_t k = 0; k < key_len; ++k)
			{
				m_field = m_faceinfo_des->FindFieldByName("landmark");

				m_landmark = m_faceinfo_ref->AddMessage(m_faceinfo, m_field);
				m_landmark_des = m_landmark->GetDescriptor();
				m_landmark_ref = m_landmark->GetReflection();

				m_field = m_landmark_des->FindFieldByName("id");
				m_landmark_ref->SetUInt32(m_landmark, m_field, k + 1);

				m_field = m_landmark_des->FindFieldByName("X");
				m_landmark_ref->SetInt32(m_landmark, m_field, static_cast<google::protobuf::int32>(m_keyPoints[k][0]));

				m_field = m_landmark_des->FindFieldByName("Y");
				m_landmark_ref->SetInt32(m_landmark, m_field, static_cast<google::protobuf::int32>(m_keyPoints[k][1]));

			}


			m_field = m_faceinfo_des->FindFieldByName("state");
			m_faceinfo_ref->SetBool(m_faceinfo, m_field, true);



			//Log info

#ifdef  LOG_INFO





			//Log info
#endif
			//filename.clear();
			//basename.clear();
			m_image.release();
			m_rect.clear();
			m_keyPoints.clear();
			tmp.release();

		}




		TK::PB_Writer(subname_, m_faceinfos);
		if (!m_faceinfos) m_faceinfos->Clear();
		//Clear();

		patchFile.close();
	}


	config.close();


}

bool Detector::ElicitMessage(const char* in_proto_name)
{

	//TK::PBErrorCollector errorCollector;
	//google::protobuf::compiler::DiskSourceTree diskSourceTree;
	//m_importer = new google::protobuf::compiler::Importer(&diskSourceTree, &errorCollector);

	//diskSourceTree.MapPath("", "./protobuf");

	//m_filedec = m_importer->Import("landmark.proto");



	//m_faceinfos_des = m_filedec->pool()->FindMessageTypeByName("FaceInfos");
	//m_boundingbox_des = m_filedec->pool()->FindMessageTypeByName("BoundingBox");

	//m_messageFactory = new google::protobuf::DynamicMessageFactory(m_filedec->pool());

	TK::PB_Reader(in_proto_name, m_faceinfos);


	return true;
}


/************************************************************************/
/*  Multi Thread Detect                                                                     */
/************************************************************************/
void* Detector::PartialDetectMessage(void* reg){
	
	return NULL;
}

Detector::~Detector()
{

	m_filename.clear();
	m_basename.clear();

}


//////////////////////////////////////////////////////////////////////////

//
//
//bool FileParser::Initial(void){
//
//
//	m_fileNum = TK::tk_get_filenames(m_infolder.c_str(), m_totalImageName, "jpg");
//
//	TK::PBErrorCollector errorCollector;
//	google::protobuf::compiler::DiskSourceTree diskSourceTree;
//	m_importer = new google::protobuf::compiler::Importer(&diskSourceTree, &errorCollector);
//
//	diskSourceTree.MapPath("", "./protobuf");
//	m_filedes = m_importer->Import("filenames.proto");
//
//	m_messageFactory = new google::protobuf::DynamicMessageFactory(m_filedes->pool());
//
//	m_ImagePatchDesc = m_filedes->pool()->FindMessageTypeByName("ImagePatch");
//
//	return true;
//
//}
//
//size_t FileParser::DumpImageFileNames(FileParser::DUMP_METHOD method)
//{
//
//	assert(m_fileNum);
//
//
//	size_t max_batch_num = m_fileNum / (PATCH_WISE_NUM);
//	
//
//	switch (method)
//	{
//	case FileParser::USE_SPLIT_FILES:
//
//		for (size_t batch_num = 0; batch_num <= max_batch_num; ++batch_num)
//		{
//
//			size_t startIdx = batch_num * PATCH_WISE_NUM;
//			size_t endIdx = std::min(std::max((size_t)0, (batch_num + 1)*PATCH_WISE_NUM - 1), m_fileNum - 1);
//
//			std::vector<std::string>  subfilename(&m_totalImageName[startIdx], &m_totalImageName[endIdx]);
//			char* subname = new char[256]; //batch dump name
//			memset(subname, 0, sizeof(subname));
//			std::string tmp(m_outfolder);
//			std::sprintf(subname, tmp.append("%d-%d.filenames").c_str(), startIdx, endIdx);
//			TK::tk_dump_filenames(subfilename, subname,startIdx);
//
//		}
//
//
//
//
//		break;
//
//
//	case FileParser::USE_PROTOBUF_DUMP:
//
//
//		m_ImagePatch = m_messageFactory->GetPrototype(m_ImagePatchDesc)->New();
//		m_ImagePatchRef = m_ImagePatch->GetReflection();
//
//		break;
//
//	default:
//		break;
//	}
//
//
//
//
//
//
//	return (size_t)0;
//}

/************************************************************************/
/* FaceShape                                                                     */
/************************************************************************/
cv::Mat_<double>& FaceShape::GetMeanShape(void)
{
		
		size_t len = m_imageinfos.size();

		m_shapeNum = 0;

		std::string midshape("D:/Arcsoft/data/portrait/big_results/midResults/");

		m_meanShape = cv::Mat::zeros(95, 2, CV_64FC1);

		for (size_t t = 0; t < len; ++t)
		{
			//dump

			std::string  midname(midshape);

			size_t s = m_imageinfos[t].find_last_of("/")+1;
			size_t e = m_imageinfos[t].find_first_of(".");

			midname.append(m_imageinfos[t].substr(s, e)).append(".txt");
			size_t num = 0;
			cv::Mat_<double> temp = cv::Mat::zeros(95, 2, CV_64FC1);
			

			//mid dump
			
			std::cout << "m_imageinfos[t] = " << m_imageinfos[t].substr(s,e) << std::endl;

			TK::PB_Reader(m_imageinfos[t].c_str(), m_faceinfos);

			m_faceinfos_desc = m_faceinfos->GetDescriptor();
			m_faceinfos_ref = m_faceinfos->GetReflection();

			m_field = m_faceinfos_desc->FindFieldByName("info");

			
			//


			size_t  imageNum = m_faceinfos_ref->FieldSize(*m_faceinfos, m_field);

			for (size_t i = 0; i < imageNum;++i)
			{

				m_faceinfo = &(m_faceinfos_ref->GetRepeatedMessage(*m_faceinfos, m_field, i));

				cv::Mat_<double> raw;
				cv::Mat_<double> center;

				if (TK::tk_centralization(m_faceinfo,center,raw))
				{
					//std::cout << " OK " << std::endl;
					++m_shapeNum;
					++num;
					m_meanShape = m_meanShape + center;
					temp = temp + center;
				}
				else
				{
					std::cout << "No keypoints" << std::endl;
				}

			}

			temp = temp / num;
			//dump  meanshape

			std::ofstream tmp(midname);
			tmp << num << std::endl;

			for (size_t i = 0; i < temp.rows; ++i)
			{
				tmp << temp[i][0] << " " << temp[i][1] << std::endl;
			}
			tmp.close();


			if (!m_faceinfos)  m_faceinfos->Clear();

		}

		m_meanShape = m_meanShape / m_shapeNum;
		std::ofstream ms("./data/meanshape.txt");
		for (size_t i = 0; i < m_meanShape.rows; ++i)
		{
			ms << m_meanShape[i][0] << " " << m_meanShape[i][1] << std::endl;
		}
		ms.close();



	
		return m_meanShape;

}

