#pragma once

#include "MInputFrame.h"

#define TIME_OUT_OBJECT 1
#define DIRECT_ENDENTIFIED 0
#define DIRECT_ENTER 1
#define DIRECT_LEAVE 2


enum MOVING_DIRECT{
	ENTER = 0,
	LEAVE,
	UNDENTIFINED
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
	time_t m_last_active;
	int m_ID;
	//bool m_being_count;

	MObject m_last_change_direct_state;

	int getNewID();
	MOVING_DIRECT getDirect(MObject state_last, MObject state_cur);
	
	
public:
	MTrackObject();
	~MTrackObject();

	static int STA_cur_ID;
	//static int getNewID();

	MOVING_DIRECT updateNewState(MObject new_state, time_t time_update);
	MOVING_DIRECT getDirect();
	MOVING_DIRECT updateCounting(MObject new_state);

	friend class MPeopleCounting;
};


class MPeopleCounting{

	CWinThread* m_process_thread;
	
	MInputFrame* m_input_object;

	Mat m_last_frame;
	Mat m_cur_frame;
	time_t m_cur_time;
	time_t m_cur_update_time;

	bool m_has_new_frame;
	
	int m_num_leave;
	int m_num_enter;

	CCriticalSection m_sync_frame;

	vector<MObject> m_tmp_objs;
	list<MTrackObject*> m_track_objs;

	void addFrame(const Mat& img);
	void getFrame(Mat& cur_frame, Mat& last_frame, time_t &time_update);
	float getObject(Mat& cur_frame, Mat& last_frame);
	void removeNoiseCircle();
	void updateTrackList();
	void updateActiveObj();
	void updateNewObj();
	void removeOldObj();
	float distance(Point2f a, Point2f b);

	void updateTracking(Mat& cur_frame, Mat& last_frame);
	void illu(Mat& img);

public:
	bool b_running;
	static int STA_bar;
	static float STA_resize_scale;
	static int STA_average_size;

public:
	MPeopleCounting();
	~MPeopleCounting();
	void release();

	void begin(MInputFrame* input_obj, int average_size, int bar_y);
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