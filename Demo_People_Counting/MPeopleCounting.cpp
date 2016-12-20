#include "MPeopleCounting.h"

UINT RunFrameThreadProc(LPVOID pdata){
	MPeopleCounting* model = (MPeopleCounting*)pdata;
	model->processFrame();

	return 0;
}

UINT RunFrameThreadInput(LPVOID pdata){
	MPeopleCounting* model = (MPeopleCounting*)pdata;
	model->addFrame();

	return 0;
}


MPeopleCounting::MPeopleCounting(){
	b_running = false;

	m_process_thread = NULL;
	m_input_thread = NULL;
}

MPeopleCounting::~MPeopleCounting(){

	b_running = false;
	m_has_new_frame = true;

	if(m_process_thread){
		Sleep(100);
		m_process_thread->Delete();
		m_process_thread = NULL;
	}

	if(m_input_thread){
		m_input_thread->Delete();
		m_input_thread = NULL;
	}
}

void MPeopleCounting::release(){

	b_running = false;

	if(m_process_thread){
		Sleep(100);
		m_process_thread->Delete();
		m_process_thread = NULL;
	}

	if(m_input_thread){
		m_input_thread->Delete();
		m_input_thread = NULL;
	}
}

void MPeopleCounting::begin(string file_name){
	b_running = true;
	m_file_name = file_name;

	m_process_thread = AfxBeginThread(RunFrameThreadProc, this);
	m_input_thread = AfxBeginThread(RunFrameThreadInput, this);
}

void MPeopleCounting::stop(){
	b_running = false;
}

void MPeopleCounting::addFrame(){
	//cout << m_file_name << endl;
	VideoCapture capture(m_file_name);
	if(!capture.isOpened() )
			cout <<"Error when reading steam_avi";
	Mat frame;
	while (b_running)
	{
		capture >> frame;
        if(frame.empty()){
            b_running = false;
			break;
		}
		else
			addFrame(frame);

		cout <<"addFrame ..."<< endl;
		Sleep(30);
	}

	b_running = false;
}

void MPeopleCounting::processFrame(){
	Mat last_frame(0, 0, CV_8UC3);
	Mat cur_frame(0, 0, CV_8UC3);

	int processing_time = 0;
	while (b_running)
	{
		processing_time = 0;
		getFrame(cur_frame);
		if(cur_frame.rows > 0){
			if(last_frame.rows == 0)
				last_frame = cur_frame.clone();
			else{
				int begin = clock();
				MIP::compareImg(cur_frame, last_frame);
				processing_time = clock() - begin;
				last_frame = cur_frame.clone();
			}
		}

		cout <<"processFrame: "<< processing_time << endl;
		Sleep(100);
	}
}

void MPeopleCounting::addFrame(const Mat& img){
	CSingleLock lock(&m_sync_frame);
	lock.Lock();

	if(lock.IsLocked()){
		m_cur_frame = img.clone();
		m_has_new_frame = true;
	}

	lock.Unlock();
}

void MPeopleCounting::getFrame(Mat& img){
	CSingleLock lock(&m_sync_frame);
	lock.Lock();

	if(lock.IsLocked()){
		if(m_has_new_frame)
			img = m_cur_frame.clone();
		else
			img = Mat(0, 0, CV_8UC3);

		m_has_new_frame = false;
	}

	lock.Unlock();
}