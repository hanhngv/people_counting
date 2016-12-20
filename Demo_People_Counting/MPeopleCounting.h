#pragma once

#include "include.h"

#define TIME_OUT_OBJECT 2

enum MOVING_DIRECT{
	UP = 0,
	DOWN,
	UNDEFINED
};

class MObject{
	Point2f m_center;
	float m_radius;

public:
	MObject();
	~MObject();

	friend class MPeopleCounting;
	friend class MTrackObject;
};

class MTrackObject{
	list<MObject> m_object_hist;
	MOVING_DIRECT m_direct;

	bool m_is_active;
	int m_ID;
	
	int getNewID();

public:
	MTrackObject();
	~MTrackObject();

	static int STA_cur_ID;

	friend class MPeopleCounting;
};

class MPeopleCounting{

	CWinThread* m_process_thread;
	CWinThread* m_input_thread;

	string m_file_name;

	Mat m_last_frame;
	Mat m_cur_frame;
	bool m_has_new_frame;
	int m_average_size;
	float m_resize_scale;

	CCriticalSection m_sync_frame;

	vector<MObject> m_tmp_objs;
	list<MTrackObject*> m_track_objs;

	void addFrame(const Mat& img);
	void getFrame(Mat& cur_frame, Mat& last_frame);
	float getObject(Mat& cur_frame, Mat& last_frame);
	void removeNoiseCircle();
	void updateTrackList();
	void updateActiveObj();

	void updateTracking(Mat& cur_frame, Mat& last_frame);
	void illu(Mat& img);

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