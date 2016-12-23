#include "MPeopleCounting.h"


int MTrackObject::STA_cur_ID = 0;

/// -----------------------------------------------------
/// --------------------- MTrackObject ------------------
/// -----------------------------------------------------

MTrackObject::MTrackObject(){
	m_direct = MOVING_DIRECT::UNDEFINED;
	m_is_active = false;
}

MTrackObject::~MTrackObject(){
}

int MTrackObject::getNewID(){
	STA_cur_ID++;

	return STA_cur_ID;
}

/// -----------------------------------------------------
/// ------------------- MPeopleCounting -----------------
/// -----------------------------------------------------

void MPeopleCounting::updateTracking(Mat& cur_frame, Mat& last_frame){
	m_resize_scale = getObject(cur_frame, last_frame);
	removeNoiseCircle();
	updateTrackList();
}


void MPeopleCounting::updateTrackList(){
	updateActiveObj();

	updateNewObj();	

	removeOldObj();
}


void MPeopleCounting::updateActiveObj(){
	for(int i = 0; i < m_tmp_objs.size(); i++){
		MTrackObject* nearest_obj = NULL;
		int min_dist = m_average_size;
		
		list<MTrackObject*>::iterator it = m_track_objs.end();
		while (it != m_track_objs.begin())
		{
			it--;

			if((*it)->m_is_active){
				list<MObject>::iterator last_hist = ((*it)->m_object_hist).end();
				last_hist--;

				int cur_dist = distance(m_tmp_objs[i].m_center, (*last_hist).m_center);

				if(cur_dist < m_average_size / 1.75){
					if(!nearest_obj){
						nearest_obj = *it;
						min_dist = cur_dist;
					}
					else if(cur_dist < min_dist){
						nearest_obj = *it;
						min_dist = cur_dist;
					}
				}
			}
		}

		if(nearest_obj){
			nearest_obj->m_object_hist.push_back(m_tmp_objs[i]);
			nearest_obj->m_last_active = m_cur_update_time;
			m_tmp_objs.erase(m_tmp_objs.begin() + i);
			i--;
		}
	}
}

void MPeopleCounting::updateNewObj(){
	for(int i = 0; i < m_tmp_objs.size(); i++){
		MTrackObject* new_obj = new MTrackObject;
		new_obj->m_object_hist.push_back(m_tmp_objs[i]);
		new_obj->m_is_active = true;
		new_obj->m_ID = MTrackObject::getNewID();
		new_obj->m_last_active = m_cur_update_time;		
		
		m_track_objs.push_back(new_obj);
	}

	m_tmp_objs.clear();
}

void MPeopleCounting::removeOldObj(){
	list<MTrackObject*>::iterator it = m_track_objs.end();
	while (it != m_track_objs.begin())
	{
		it--;

		if((*it)->m_is_active){
			if(m_cur_update_time - (*it)->m_last_active > TIME_OUT_OBJECT * 1000){
				it = m_track_objs.erase(it);
				//(*it)->m_is_active = false;
			}
		}
	}
}