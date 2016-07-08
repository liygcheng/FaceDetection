
#include"HairSelection.h"

#include "HairDistribution.h"

#include "utility.h"


//#define  TEST_PROTOBUF
//#define  TEST_DISTRIBUTION
#ifndef  TEST_PROTOBUF
#ifndef TEST_DISTRIBUTION
	#define TEST_SELECTION
#endif // !1
#endif // ! TEST_PROTOBUF





int main(void)
{

	const char* m_infolder = "C:/Project/data/images/";
	const char* m_outfoler = "C:/Project/data/";
	//CheckForPath(m_outfoler);

#ifdef TEST_SELECTION
	//HairParser* m_parser = new HairParser(m_infolder, m_outfoler);

	//m_parser->DetectMaxFaceNum("C:/Project/data/images/song4.jpg");

#endif // TEST_SELECTION

#ifdef TEST_DISTRIBUTION
	Distributor* m_Distributor = new Distributor(m_infolder, m_outfoler, 5);
	m_Distributor->detectKeyPoints("C:/Project/data/images/236401_8bd744c780_b.jpg");

	//	m_Distributor->calcFeature();
	//
	//	m_Distributor->perfromKmeans();
	//
	//	vl_uint32* tmp = m_Distributor->getAssignment();
	//
	//#pragma omp parallel for
	//	for (int i = 0; i < m_Distributor->getDataNum(); ++i)
	//	{
	//		printf("image %d is assigned  to ----------->  %d \n",i,(int)(tmp[i]));
	//	}
	//


#endif // TEST_DISTRIBUTION

#ifdef TEST_PROTOBUF

	google::protobuf::compiler::Importer* importer = NULL;
	TK::PB_Initialize("./protobuf/", "landmark.proto",importer);



	 const google::protobuf::Descriptor* faceinfos_dsc = importer->pool()->FindMessageTypeByName("FaceInfos");

	 const google::protobuf::Descriptor* faceinfo_dsc = importer->pool()->FindMessageTypeByName("FaceInfo");

	 const google::protobuf::Descriptor* landmark_dsc = importer->pool()->FindMessageTypeByName("LandMark");

	 const google::protobuf::Descriptor* box_dsc = importer->pool()->FindMessageTypeByName("BoundingBox");

	// const google::protobuf::FieldDescriptor* m_state_dsc = faceinfo->FindFieldByName("state");

	 google::protobuf::MessageFactory* fact = new google::protobuf::DynamicMessageFactory(importer->pool());

	 google::protobuf::Message* faceinfos = fact->GetPrototype(faceinfos_dsc)->New();
	 google::protobuf::Message* faceinfo  = fact->GetPrototype(faceinfo_dsc)->New();

	 const google::protobuf::Reflection* faceinfo_ref = faceinfo->GetReflection();

	 faceinfo_ref->SetBool(faceinfo, faceinfo_dsc->FindFieldByName("state"), true);

	 
	 //faceinfos->GetReflection()->AddMessage(faceinfo, faceinfos_dsc->FindFieldByName("info"));/*(faceinfos, faceinfo, faceinfos_dsc->FindFieldByName("info"));*/


	 TK::PB_Writer("liygcheng_7_7_2016",*faceinfos);

	 //std::cout << "faceinfo name = " << faceinfo_dsc->name() << std::endl;

	// const google::protobuf::FieldDescriptor* faceinfo = importer->pool()->FindFieldByName("FaceInfo");
	// const google::protobuf::Descriptor* landmark = importer->pool()->FindMessageTypeByName("LandMark");
	// const google::protobuf::Descriptor* box = importer->pool()->FindMessageTypeByName("BoundingBox");

	//FaceInfos faceinfos;
	//FaceInfos faceinfotmp;

	//FaceInfo* faceinfo;

	//BoundingBox boundingbox;

	//LandMark* landmark;

	//faceinfo->set_state(true);
	//faceinfo->set_filename("C:/image.jpg");
	//faceinfo->set_basename("image.jpg");

	////faceinfo->box().set_startx(100);

	////boundingbox = faceinfo->box();
	//boundingbox.set_startx(0);
	//boundingbox.set_starty(100);
	//boundingbox.set_endx(200);
	//boundingbox.set_endy(300);
	//boundingbox.set_width(200);
	//boundingbox.set_height(200);
	//boundingbox.set_centroidx(100);
	//boundingbox.set_centroidy(200);

	//faceinfo->set_allocated_box(&boundingbox);

	//landmark = faceinfo->add_landmark();
	//landmark->set_id(1);
	//landmark->set_x(10);
	//landmark->set_y(20);
	//landmark = faceinfo->add_landmark();
	//landmark->set_id(2);
	//landmark->set_x(20);
	//landmark->set_y(30);

	//TK::PB_Writer("liygcheng_7_5_2016", faceinfos);
	//TK::PB_Reader("liygcheng_7_5_2016", faceinfotmp);
	//TK::PB_Print(faceinfotmp.info(0));



#endif // TEST_PROTOBUF

	 Detector* m_detector = new Detector(m_infolder, "faceinfos.dat", m_outfoler);

	 m_detector->ElicitFilenamesFromInfolder();

	 m_detector->DetectMessage();


	system("pause");

	return 0;
}
