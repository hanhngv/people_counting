#pragma once

#include "include.h"


class MInputFrame{
	
protected:
	Mat m_cur_frame;
	Mat m_last_frame;
	bool m_is_has_new_frame;
	time_t m_cur_time;
	bool m_remain_frame;

	CCriticalSection m_sync_input_frame;
	CWinThread* m_input_thread;

	virtual bool addOneFrame(Mat& new_frame) = 0;
	
	void addFrame();

public:
	MInputFrame();
	~MInputFrame();

	virtual bool init(string init_param) = 0;
	bool getFrame(Mat& cur_frame, Mat& last_frame, time_t &cur_update_time);
	void inputFrameExcutor();
};

class MInputFromVideo: public MInputFrame{
protected:
	VideoCapture m_capture;

	CWinThread* m_input_thread;

	bool addOneFrame(Mat& new_frame);

public:
	bool init(string file_name);
};