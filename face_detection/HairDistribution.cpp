#include "HairDistribution.h"

#define  SHOW_KEY_POINTS

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

