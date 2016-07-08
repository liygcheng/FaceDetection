#include "HairDistribution.h"

//#define  SHOW_KEY_POINTS

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

	TK::PB_Initialize("./protobuf", "landmark.proto", m_importer);

	m_faceinfos_des = m_importer->pool()->FindMessageTypeByName("FaceInfos");
	m_faceinfo_des = m_importer->pool()->FindMessageTypeByName("FaceInfo");
	m_landmark_des = m_importer->pool()->FindMessageTypeByName("LandMark");
	m_boundingbox_des = m_importer->pool()->FindMessageTypeByName("BoundingBox");


	m_messageFactory = new google::protobuf::DynamicMessageFactory(m_importer->pool());

	m_faceinfos = m_messageFactory->GetPrototype(m_faceinfo_des)->New();
	//m_faceinfo = m_messageFactory->GetPrototype(m_faceinfo_des)->New();
	//m_landmark = m_messageFactory->GetPrototype(m_landmark_des)->New();
	//m_boundingbox = m_messageFactory->GetPrototype(m_boundingbox_des)->New();


	m_faceinfos_ref = m_faceinfos->GetReflection();// is this so called Reflection ?
	//m_faceinfo_ref = m_faceinfo->GetReflection();
	//m_landmark_ref = m_landmark->GetReflection();
	//m_boundingbox_ref = m_boundingbox->GetReflection();

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

//#pragma  omp parallel for
	for (size_t i = 0; i < file_len; ++i)
	{
		std::cout << "count = " <<i<< std::endl;
		m_faceinfo = m_messageFactory->GetPrototype(m_faceinfo_des)->New();
		//m_landmark = m_messageFactory->GetPrototype(m_landmark_des)->New();
		m_boundingbox = m_messageFactory->GetPrototype(m_boundingbox_des)->New();
		m_faceinfo_ref = m_faceinfo->GetReflection();
		//m_landmark_ref = m_landmark->GetReflection();
		m_boundingbox_ref = m_boundingbox->GetReflection();
	

		std::string filename = m_filename[i];
		std::string basename = m_basename[i];

		m_faceinfo_ref->SetString(m_faceinfo, m_faceinfo_des->FindFieldByName("filename"), filename); //
		m_faceinfo_ref->SetString(m_faceinfo, m_faceinfo_des->FindFieldByName("basename"), basename); //
		
		cv::Mat m_image = cv::imread(filename);
		if ((!m_image.data) || (m_image.channels() != 3)) // state = false;
		{
			m_faceinfo_ref->SetBool(m_faceinfo, m_faceinfo_des->FindFieldByName("state"),false);
			m_faceinfos_ref->AddMessage(m_faceinfo, m_faceinfos_des->FindFieldByName("FaceInfo"), m_messageFactory);
			continue;
		}


		int w = m_image.cols - (m_image.cols & 1);
		int h = m_image.rows - (m_image.rows & 1);
		cv::Mat tmp = m_image(Rect(0, 0, w, h));


		vecR m_rect;
		CxFaceDA::ArcSoftFaceDetection(tmp,2, 5, m_rect);

		if (m_rect.size() != 1)
		{
			m_faceinfo_ref->SetBool(m_faceinfo, m_faceinfo_des->FindFieldByName("state"),false);

			m_faceinfos_ref->AddMessage(m_faceinfos, m_faceinfos_des->FindFieldByName("FaceInfo"), m_messageFactory);
			
			continue;
		}


		m_faceinfo_ref->SetBool(m_faceinfo, m_faceinfo_des->FindFieldByName("state"), true);


		// Set up bounding box
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startX"), m_rect[0].tl().x);
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("startY"), m_rect[0].tl().y);
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endX"), m_rect[0].br().x);
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("endY"), m_rect[0].tl().y);
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidX"), (m_rect[0].tl().x + m_rect[0].br().x)/2);
		m_boundingbox_ref->SetInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("centroidY"), (m_rect[0].tl().y + m_rect[0].br().y)/2);

		m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("width"), m_rect[0].br().x - m_rect[0].tl().x);
		m_boundingbox_ref->SetUInt32(m_boundingbox, m_boundingbox_des->FindFieldByName("height"),m_rect[0].br().y - m_rect[0].tl().x);
		
		m_faceinfo_ref->AddMessage(m_faceinfo, m_faceinfo_des->FindFieldByName("BoundingBox"));

		//m_faceinfo_ref->SetAllocatedMessage(m_faceinfo, m_boundingbox, m_faceinfo_des->FindFieldByName("BoundingBox"));

		

		vecP2d  m_keyPoints;
		CxFaceDA::ArcSoftFaceAlignment(tmp, 5, m_keyPoints);



		size_t key_len = m_keyPoints.size();
		if (key_len == 0)
		{
			m_faceinfo_ref->SetBool(m_faceinfo, m_faceinfo_des->FindFieldByName("state"), false);

			m_faceinfos_ref->AddMessage(m_faceinfos, m_faceinfos_des->FindFieldByName("FaceInfo"), m_messageFactory);

			continue;
		}

		for (size_t k = 0; k < key_len; ++k)
		{
			m_landmark = m_messageFactory->GetPrototype(m_landmark_des)->New();
			m_landmark_ref = m_landmark->GetReflection();
			m_landmark_ref->SetUInt32(m_landmark, m_landmark_des->FindFieldByName("id"), k+1);
			m_landmark_ref->SetInt32(m_landmark, m_landmark_des->FindFieldByName("X"), static_cast<google::protobuf::int32>( m_keyPoints[k][0]));
			m_landmark_ref->SetInt32(m_landmark, m_landmark_des->FindFieldByName("Y"), static_cast<google::protobuf::int32>( m_keyPoints[k][1]));
			m_faceinfo_ref->AddMessage(m_landmark, m_faceinfo_des->FindFieldByName("landmark"), m_messageFactory);

		}

		
		m_faceinfos_ref->AddMessage(m_faceinfos, m_faceinfos_des->FindFieldByName("FaceInfo"), m_messageFactory);


		TK::PB_Writer(m_dumpname.c_str(), *m_faceinfos);



		filename.clear();
		basename.clear();
		m_image.release();
		m_rect.clear();
		m_keyPoints.clear();
		tmp.release();

	}




	//  step 2:  assign message

	//cv::Mat m_image = cv::imread(filename);
	//if (!m_image.data)
	//	return;

	//if (m_image.channels() != 3)  //  get rid of non-color  images
	//	return;

	//for some check
	//vecR m_rect;

	//int w = m_image.cols - (m_image.cols & 1);
	//int h = m_image.rows - (m_image.rows & 1);

	//cv::Mat tmp = m_image(Rect(0, 0, w, h));

	//vecP2d  m_keyPoints;

	//CxFaceDA::ArcSoftFaceAlignment(tmp, 5, m_keyPoints);

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


Detector::~Detector()
{

	m_filename.clear();
	m_basename.clear();

}
