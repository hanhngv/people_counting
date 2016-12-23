#include "MPeopleCounting.h"


RNG rng(12345);

int MPeopleCounting::STA_bar = 100;
float MPeopleCounting::STA_resize_scale = 1.0f;
int MPeopleCounting::STA_average_size = 50;


UINT RunFrameThreadProc(LPVOID pdata){
	MPeopleCounting* model = (MPeopleCounting*)pdata;
	model->processFrame();

	return 0;
}


/// -----------------------------------------------------
/// -------------------------- MObject ------------------
/// -----------------------------------------------------

MObject::MObject(){
	m_center.x = 0;
	m_center.y = 0;
	m_radius = -1;
}

MObject::~MObject(){
}

/// -----------------------------------------------------
/// ----------------------- MPeopleCounting -------------
/// -----------------------------------------------------

MPeopleCounting::MPeopleCounting(){
	b_running = false;

	//STA_average_size = 50;
	//m_resize_scale = 1.0f;
	m_process_thread = NULL;
//	m_input_thread = NULL;
	m_num_enter = 0;
	m_num_leave = 0;
}

MPeopleCounting::~MPeopleCounting(){

	b_running = false;
	m_has_new_frame = true;

	if(m_process_thread){
		Sleep(100);
		m_process_thread->Delete();
		m_process_thread = NULL;
	}

	while (m_track_objs.size() > 0){
		delete *(m_track_objs.begin());
		m_track_objs.pop_front();
	}
}

void MPeopleCounting::release(){

	b_running = false;

	if(m_process_thread){
		Sleep(100);
		m_process_thread->Delete();
		m_process_thread = NULL;
	}

	/*if(m_input_thread){
		m_input_thread->Delete();
		m_input_thread = NULL;
	}*/

	while (m_track_objs.size() > 0){
		delete *(m_track_objs.begin());
		m_track_objs.pop_front();
	}
}

void MPeopleCounting::begin(MInputFrame* input_obj, int average_size, int bar_y){
	m_input_object = input_obj;
	b_running = true;

	STA_average_size = average_size;
	STA_bar = bar_y;
	m_num_enter = 0;
	m_num_leave = 0;

	m_process_thread = AfxBeginThread(RunFrameThreadProc, this);
}

void MPeopleCounting::stop(){
	b_running = false;
}

//void MPeopleCounting::addFrame(){
//	//cout << m_file_name << endl;
//	VideoCapture capture(m_file_name);
//	if(!capture.isOpened() )
//			cout <<"Error when reading steam_avi";
//	Mat frame;
//	while (b_running)
//	{
//		capture >> frame;
//        if(frame.empty()){
//            b_running = false;
//			break;
//		}
//		else
//			addFrame(frame);
//
//		cout <<"addFrame ..."<< endl;
//		Sleep(30);
//	}
//
//	b_running = false;
//}

void MPeopleCounting::processFrame(){
	Mat last_frame(0, 0, CV_8UC3);
	Mat cur_frame(0, 0, CV_8UC3);

	int processing_time = 0;
	while (b_running)
	{
		processing_time = 0;

		b_running = m_input_object->getFrame(cur_frame, last_frame, m_cur_update_time);
		if(cur_frame.rows > 0){
			if(last_frame.rows == 0)
				last_frame = cur_frame.clone();
			else{
				time_t begin = clock();
				updateTracking(cur_frame, last_frame);
				processing_time = clock() - begin;

				//updateTrackList();

				illu(cur_frame);

				last_frame = cur_frame.clone();
			}
		}

		cout <<"processFrame: "<< processing_time << endl;
		Sleep(100);
	}
}

void MPeopleCounting::addFrame(const Mat& img){
	time_t cur_time = clock();

	CSingleLock lock(&m_sync_frame);
	while(lock.IsLocked())
		Sleep(1);
	lock.Lock();

	if(lock.IsLocked()){
		m_last_frame = m_cur_frame.clone();
		m_cur_frame = img.clone();
		m_has_new_frame = true;
		m_cur_time = cur_time;
	}

	lock.Unlock();
}

void MPeopleCounting::getFrame(Mat& cur_frame, Mat& last_frame, time_t &time_update){
	CSingleLock lock(&m_sync_frame);
	while(lock.IsLocked())
		Sleep(1);
	lock.Lock();

	if(lock.IsLocked()){
		if(m_has_new_frame){
			cur_frame = m_cur_frame.clone();
			last_frame = m_last_frame.clone();
		}
		else{
			cur_frame = Mat(0, 0, CV_8UC3);
			last_frame = Mat(0, 0, CV_8UC3);
		}
		time_update = m_cur_time;

		m_has_new_frame = false;
	}

	lock.Unlock();
}

float MPeopleCounting::getObject(Mat& img1, Mat& img2)
{
	float resize_scale = 1;

	if(img1.rows != img2.rows)
		return resize_scale;// 1.0f;
	if(img1.cols != img2.cols)
		return resize_scale;// 1.0f;

	Mat img_1 = img1.clone();
	Mat img_2 = img2.clone();

	if(img_1.channels() != 1)
		cvtColor(img_1, img_1, CV_BGR2GRAY);
	if(img_2.channels() != 1)
		cvtColor(img_2, img_2, CV_BGR2GRAY);

	cv::Size kernel_size(3, 3);
	if(img_1.cols > 80)
	{
		if(img_1.cols < 160)
			kernel_size = cv::Size(5, 5);
		else
		{
			resize_scale = 200.0f / img_1.cols;
			resize(img_1, img_1, cv::Size(img_1.cols * resize_scale, img_1.rows * resize_scale));
			resize(img_2, img_2, cv::Size(img_2.cols * resize_scale, img_2.rows * resize_scale));
		}
	}
	GaussianBlur(img_1, img_1, kernel_size, 0);
	GaussianBlur(img_2, img_2, kernel_size, 0);

	Mat grad_x_1, grad_y_1, abs_grad_x_1, abs_grad_y_1;
	Sobel(img_1, grad_x_1, CV_16S, 1, 0);
	convertScaleAbs(grad_x_1, abs_grad_x_1);
	Sobel(img_1, grad_y_1, CV_16S, 0, 1);
	convertScaleAbs(grad_y_1, abs_grad_y_1);

	Mat grad_x_2, grad_y_2, abs_grad_x_2, abs_grad_y_2;
	Sobel(img_2, grad_x_2, CV_16S, 1, 0);
	convertScaleAbs(grad_x_2, abs_grad_x_2);
	Sobel(img_2, grad_y_2, CV_16S, 0, 1);
	convertScaleAbs(grad_y_2, abs_grad_y_2);

	Mat sub_x, sub_x_inv, sub_x_merg;
	cv::subtract(abs_grad_x_1, abs_grad_x_2, sub_x);
	cv::subtract(abs_grad_x_2, abs_grad_x_1, sub_x_inv);
	bitwise_or(sub_x, sub_x_inv, sub_x_merg);

	Mat sub_y, sub_y_inv, sub_y_merg;
	cv::subtract(abs_grad_y_1, abs_grad_y_2, sub_y);
	cv::subtract(abs_grad_y_2, abs_grad_y_1, sub_y_inv);
	bitwise_or(sub_y, sub_y_inv, sub_y_merg);

	Mat sub_xy;
	bitwise_or(sub_x_merg, sub_y_merg, sub_xy);

	Mat sub_thresh;
	cv::threshold(sub_xy, sub_thresh, 90, 255, CV_THRESH_BINARY);

	int erosion_elem = 0;
	int erosion_size = 0;
	int dilation_elem = 0;
	int dilation_size = 3;
	int const max_elem = 2;
	int const max_kernel_size = 21;

	int dilation_type;
	if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
	else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
	else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }

	Mat element = getStructuringElement( dilation_type,
		Size( 2*dilation_size + 1, 2*dilation_size+1 ),
		Point( dilation_size, dilation_size ) );
	/// Apply the dilation operation
	dilate( sub_thresh, sub_thresh, element );

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Find contours
	findContours( sub_thresh.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );


	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	{
		approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
		boundRect[i] = boundingRect( Mat(contours_poly[i]) );
		minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
	}

	m_tmp_objs.clear();
	for(int i = 0; i < center.size(); i++){
		MObject new_obj;
		new_obj.m_center = center[i];
		new_obj.m_radius = radius[i];

		m_tmp_objs.push_back(new_obj);
	}

	//removeNoiseCircle();


	//Mat img_merg;
	//cv::hconcat(img_1, img_2, img_merg);
	//for( int i = 0; i < m_tmp_objs.size(); i++ ){
	//	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	//	//drawContours( sub_xy, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	//	//rectangle( sub_xy, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
	//	circle( img_merg, m_tmp_objs[i].m_center, (int)m_tmp_objs[i].m_radius * 0.75, color, 2, 8, 0 );
	//}

	/*Mat img_subxy, disp;
	cv::hconcat(sub_xy, sub_thresh, img_subxy);
	cv::vconcat(img_merg, img_subxy, disp);

	if(disp.rows > 800)
		resize(disp, disp, cv::Size(disp.cols * (800.0f / disp.rows), 800));

	cv::imshow("compare...", disp);
	cv::waitKey(1);*/

	return resize_scale;
}

void MPeopleCounting::illu(Mat& img){
	//for( int i = 0; i < m_tmp_objs.size(); i++ ){
	//	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	//	//drawContours( sub_xy, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	//	//rectangle( sub_xy, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
	//	circle( img, Point2f(m_tmp_objs[i].m_center.x / m_resize_scale, m_tmp_objs[i].m_center.y / m_resize_scale), (int)m_tmp_objs[i].m_radius / m_resize_scale, color, 2, 8, 0 );
	//}

	list<MTrackObject*>::iterator it = m_track_objs.end();
	while(it != m_track_objs.begin()){
		it--;

		list<MObject>::iterator last_pos = (*it)->m_object_hist.end();
		last_pos--;

		MObject last = (*last_pos);

		ostringstream s_stream;
		s_stream << (*it)->m_ID;
		string num_str(s_stream.str());

		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		if(!(*it)->m_is_active)
			color = Scalar(0, 0, 0);
		Point2f draw_point(last.m_center.x / STA_resize_scale, last.m_center.y / STA_resize_scale);
		circle( img, draw_point, (int)last.m_radius / STA_resize_scale, color, 2, 8, 0 );
		putText(img, num_str, draw_point, FONT_HERSHEY_SIMPLEX, 1, color, 1.5);

		MOVING_DIRECT direct = (*it)->getDirect();
		if(direct == MOVING_DIRECT::UNDENTIFINED){
			circle( img, draw_point, (int)last.m_radius / STA_resize_scale / 4, color, 2, 8, 0 );
		}
		else if(direct == MOVING_DIRECT::ENTER){
			line(img, draw_point, cv::Point(draw_point.x, draw_point.y + last.m_radius / STA_resize_scale / 1.5), color, 2);
		}
		else if(direct == MOVING_DIRECT::LEAVE){
			line(img, draw_point, cv::Point(draw_point.x, draw_point.y - last.m_radius / STA_resize_scale / 1.5), color, 2);
		}

	}
	line(img, cv::Point(0, STA_bar), cv::Point(img.cols, STA_bar), Scalar(0, 0, 255), 2);

	ostringstream s_stream_enter;
	s_stream_enter << m_num_enter;
	string num_str = "Enter:" + string(s_stream_enter.str());
	putText(img, num_str, cv::Point(img.cols / 2, (STA_bar + STA_average_size / 2 / STA_resize_scale)), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 1.5);

	ostringstream s_stream_leave;
	s_stream_leave << m_num_leave;
	num_str =  "Leave: " + string(s_stream_leave.str());
	putText(img, num_str, cv::Point(img.cols / 2, (STA_bar - STA_average_size / 2 /STA_resize_scale)), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 1.5);


	if(img.rows > 400)
		resize(img, img, cv::Size(img.cols * 400.0f / img.rows, 400));
	
	cv::imshow("objects", img);
	cv::waitKey(1);
}

void MPeopleCounting::removeNoiseCircle(){
	for(int i = 0; i < m_tmp_objs.size(); i++){
		if(m_tmp_objs[i].m_radius < STA_average_size / 2.5)
		{
			m_tmp_objs.erase(m_tmp_objs.begin() + i);
			i--;
		}
		else if(m_tmp_objs[i].m_radius > STA_average_size )
		{
			/*m_tmp_objs.erase(m_tmp_objs.begin() + i);
			i--;*/
		}
		else
			m_tmp_objs[i].m_radius *= 0.75;
	}
}

float MPeopleCounting::distance(Point2f a, Point2f b){
	float tmp = (a.x - b.x) * (a.x - b.x);
	tmp += (a.y - b.y) * (a.y - b.y);

	return pow(tmp, 0.5);
}