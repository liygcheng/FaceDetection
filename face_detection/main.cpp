
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

void Test(void){


    google::protobuf::compiler::Importer* myimporter = NULL;
    const google::protobuf::FileDescriptor* fd = NULL;

	//TK::PB_Initialize("./protobuf/", "landmark.proto", myimporter);

	TK::PBErrorCollector errorCollector;
	google::protobuf::compiler::DiskSourceTree diskSourceTree;


	myimporter = new google::protobuf::compiler::Importer(&diskSourceTree, &errorCollector);

	diskSourceTree.MapPath("", "./protobuf");

	fd = myimporter->Import("landmark.proto");

   
    google::protobuf::MessageFactory* msgfact = new google::protobuf::DynamicMessageFactory(fd->pool());


	const  google::protobuf::Descriptor* dec = fd->pool()->FindMessageTypeByName("LandMark");
	const  google::protobuf::Descriptor* boxdec = fd->pool()->FindMessageTypeByName("BoundingBox");
	const  google::protobuf::Descriptor* infodec = fd->pool()->FindMessageTypeByName("FaceInfo");
	const  google::protobuf::Descriptor* infosdec = fd->pool()->FindMessageTypeByName("FaceInfos");



	google::protobuf::Message* boxmsg = msgfact->GetPrototype(boxdec)->New();
    google::protobuf::Message* msg = msgfact->GetPrototype(dec)->New();
	google::protobuf::Message* infomsg = msgfact->GetPrototype(infodec)->New();
	google::protobuf::Message* infosmsg = msgfact->GetPrototype(infosdec)->New();


	const google::protobuf::Reflection* ref = msg->GetReflection();
	const google::protobuf::Reflection* boxref = boxmsg->GetReflection();
	const google::protobuf::Reflection* inforef = infomsg->GetReflection();
	const google::protobuf::Reflection* infosref = infosmsg->GetReflection();


	const google::protobuf::FieldDescriptor* fieldDec = NULL;

	fieldDec = dec->FindFieldByName("id");

	ref->SetUInt32(msg, fieldDec, 10);
	
	fieldDec = dec->FindFieldByName("X");

	ref->SetInt32(msg, fieldDec, 20);

	fieldDec = dec->FindFieldByName("Y");

	ref->SetInt32(msg, fieldDec, 100);




	fieldDec = boxdec->FindFieldByName("startX");
	boxref->SetInt32(boxmsg,fieldDec,324);
	fieldDec = boxdec->FindFieldByName("startY");
	boxref->SetInt32(boxmsg, fieldDec, 32);
	fieldDec = boxdec->FindFieldByName("endX");
	boxref->SetInt32(boxmsg, fieldDec, 5);
	fieldDec = boxdec->FindFieldByName("endY");
	boxref->SetInt32(boxmsg, fieldDec, 1);
	fieldDec = boxdec->FindFieldByName("centroidX");
	boxref->SetInt32(boxmsg, fieldDec, 76);
	fieldDec = boxdec->FindFieldByName("centroidY");
	boxref->SetInt32(boxmsg, fieldDec, 72);

	fieldDec = boxdec->FindFieldByName("width");
	boxref->SetUInt32(boxmsg, fieldDec, 32);
	fieldDec = boxdec->FindFieldByName("height");
	boxref->SetUInt32(boxmsg, fieldDec, 45);




	//fieldDec = infosdec->FindFieldByName("info");
	

	fieldDec = infosdec->FindFieldByNumber(1);


	//infosref->MutableRepeatedMessage(infosmsg,fieldDec,0);

	infomsg = infosref->AddMessage(infosmsg, fieldDec);
	inforef = infomsg->GetReflection();
	infodec = infomsg->GetDescriptor();


	fieldDec = infodec->FindFieldByName("state");
	inforef->SetBool(infomsg, fieldDec, true);
	fieldDec = infodec->FindFieldByName("filename");
	inforef->SetString(infomsg, fieldDec, "filename.jpg");
	fieldDec = infodec->FindFieldByName("basename");
	inforef->SetString(infomsg, fieldDec, "basename.jpg");

	fieldDec = infodec->FindFieldByName("box");
	inforef->SetAllocatedMessage(infomsg, boxmsg, fieldDec);



	

	TK::PB_Writer("Lechao_msg", msg);

	TK::PB_Writer("Lechao_boxmsg", boxmsg);

	TK::PB_Writer("Lechao_info",infomsg);

	TK::PB_Writer("Lechao_infos", infosmsg);
	//
	google::protobuf::Message* msg1 = msgfact->GetPrototype(dec)->New();
	google::protobuf::Message* boxmsg1 = msgfact->GetPrototype(boxdec)->New();

	TK::PB_Reader("Lechao_msg", msg1);
	TK::PB_Reader("Lechao_boxmsg", boxmsg1);

	const google::protobuf::Reflection* ref1 = msg1->GetReflection();
	const google::protobuf::Reflection* boxref1 = boxmsg1->GetReflection();

	std::cout << "id = "<<ref1->GetUInt32(*msg1,msg1->GetDescriptor()->FindFieldByName("id")) << std::endl;

	std::cout << "X = " << ref1->GetInt32(*msg1, msg1->GetDescriptor()->FindFieldByName("X")) << std::endl;

	std::cout << "Y = " << ref1->GetInt32(*msg1, msg1->GetDescriptor()->FindFieldByName("Y")) << std::endl;


	std::cout << "startX: = " << boxref1->GetInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("startX")) << std::endl;

	std::cout << "startY: = " << boxref1->GetInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("startY")) << std::endl;

	std::cout << "endX: = " << boxref1->GetInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("endX")) << std::endl;

	std::cout << "endY: = " << boxref1->GetInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("endY")) << std::endl;

	std::cout << "width: = " << boxref1->GetUInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("width")) << std::endl;

	std::cout << "height: = " << boxref1->GetUInt32(*boxmsg1, boxmsg1->GetDescriptor()->FindFieldByName("height")) << std::endl;


}




int main(void)
{

	const char* m_infolder = "C:/Project/data/portrait/selected/";
	const char* m_outfoler = "./data/";
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

	 Detector* m_detector = new Detector(m_infolder, "faceinfos", m_outfoler);

	 m_detector->ElicitFilenamesFromInfolder();

	 //m_detector->Clear();

	 //m_detector->ElicitMessage("faceinfos_2016_7_11");

	 //google::protobuf::Message*  msg = m_detector->GetMessagePointer();

	 //const google::protobuf::Descriptor* msgdec = msg->GetDescriptor();

	 //const google::protobuf::Reflection* msgref = msg->GetReflection();

	 //std::cout << ""<<msgref->GetString(*msg,msgdec->FindFieldByName("rootFolder")) << std::endl;

	 //std::cout << "number of FaceInfo:"<<msgref->FieldSize(*msg,msgdec->FindFieldByName("info")) << std::endl;
	 
	 m_detector->DetectMessage();

	// Test();
	system("pause");

	return 0;
}
