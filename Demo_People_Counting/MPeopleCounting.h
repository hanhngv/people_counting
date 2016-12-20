#pragma once

#include "MIP.h"


class MPeopleCounting{

	CWinThread* m_process_thread;
	CWinThread* m_input_thread;

	string m_file_name;

	Mat m_last_frame;
	Mat m_cur_frame;
	bool m_has_new_frame;
	int m_average_size;

	CCriticalSection m_sync_frame;

	void addFrame(const Mat& img);
	void getFrame(Mat& cur_frame, Mat& last_frame);

public:
	bool b_running;

public:
	MPeopleCounting();
	~MPeopleCounting();
	void release();

	void begin(string file_name, int average_size);
	void stop();

	void addFrame();
	void processFrame();
};


UINT RunFrameThreadProc(LPVOID pdata);/*{
	MPeopleCounting* model = (MPeopleCounting*)pdata;
	model->processFrame();

	return 0;
}*/

UINT RunFrameThreadInput(LPVOID pdata);/*{
	MPeopleCounting* model = (MPeopleCounting*)pdata;
	model->addFrame();

	return 0;
}*/