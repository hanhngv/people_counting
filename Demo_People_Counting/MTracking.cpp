#include "MPeopleCounting.h"


int MTrackObject::STA_cur_ID = 0;

/// -----------------------------------------------------
/// --------------------- MTrackObject ------------------
/// -----------------------------------------------------

MTrackObject::MTrackObject(){
	m_direct = MOVING_DIRECT::UNDENTIFINED;
	m_is_active = false;
	//m_being_count = false;
}

MTrackObject::~MTrackObject(){
}

int MTrackObject::getNewID(){
	STA_cur_ID++;

	return STA_cur_ID;
}

MOVING_DIRECT MTrackObject::updateNewState(MObject new_state, time_t time_update){
	m_object_hist.push_back(new_state);
	this->m_is_active = true;
	this->m_last_active = time_update;

	MOVING_DIRECT new_count = MOVING_DIRECT::UNDENTIFINED;

	if(m_object_hist.size() == 1){
		this->m_ID = MTrackObject::getNewID();
		m_direct = MOVING_DIRECT::UNDENTIFINED;

		m_last_change_direct_state = new_state;
	}

	else if(m_object_hist.size() == 2){
		m_direct = getDirect(*(m_object_hist.begin()), new_state);
	}

	else{
		list<MObject>::iterator cur_state = m_object_hist.end();
		cur_state--;
		cur_state--;

		MOVING_DIRECT new_direct = getDirect(*(cur_state), new_state);

		if((new_direct == MOVING_DIRECT::ENTER && m_direct == MOVING_DIRECT::LEAVE)
			||(new_direct == MOVING_DIRECT::LEAVE && m_direct == MOVING_DIRECT::ENTER)){
				new_count = updateCounting(new_state);

				m_last_change_direct_state = new_state;
		}

		m_direct = new_direct;
	}
	
	return new_count;
}

MOVING_DIRECT MTrackObject::getDirect(MObject state_last, MObject state_cur){
	if(state_cur.m_center.y > state_last.m_center.y)
		return MOVING_DIRECT::ENTER;
	else if(state_cur.m_center.y < state_last.m_center.y)
		return MOVING_DIRECT::LEAVE;

	return MOVING_DIRECT::UNDENTIFINED;
}

MOVING_DIRECT MTrackObject::getDirect(){
	return m_direct;
}

MOVING_DIRECT MTrackObject::updateCounting(MObject new_state){
	int cur_bar = MPeopleCounting::STA_bar * MPeopleCounting::STA_resize_scale;

	MOVING_DIRECT new_count = MOVING_DIRECT::UNDENTIFINED;

	if(((new_state.m_center.y - cur_bar) * (m_last_change_direct_state.m_center.y - cur_bar) < 0)
		&& abs(new_state.m_center.y - m_last_change_direct_state.m_center.y) > MPeopleCounting::STA_average_size){
			return m_direct;
	}

	return new_count;
}

/// -----------------------------------------------------
/// ------------------- MPeopleCounting -----------------
/// -----------------------------------------------------

void MPeopleCounting::updateTracking(Mat& cur_frame, Mat& last_frame){
	STA_resize_scale = getObject(cur_frame, last_frame);

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
		int min_dist = STA_average_size;
		
		list<MTrackObject*>::iterator it = m_track_objs.end();
		while (it != m_track_objs.begin())
		{
			it--;

			if(true/*(*it)->m_is_active*/){
				list<MObject>::iterator last_hist = ((*it)->m_object_hist).end();
				last_hist--;

				int cur_dist = distance(m_tmp_objs[i].m_center, (*last_hist).m_center);

				if(cur_dist < STA_average_size / 1.75){
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
			MOVING_DIRECT new_count = nearest_obj->updateNewState(m_tmp_objs[i], m_cur_update_time);
			if(new_count == MOVING_DIRECT::ENTER)
				m_num_enter++;
			else if(new_count == MOVING_DIRECT::LEAVE)
				m_num_leave++;

			/*nearest_obj->m_object_hist.push_back(m_tmp_objs[i]);
			nearest_obj->m_last_active = m_cur_update_time;*/
			m_tmp_objs.erase(m_tmp_objs.begin() + i);
			i--;
		}
	}
}

void MPeopleCounting::updateNewObj(){
	for(int i = 0; i < m_tmp_objs.size(); i++){
		MTrackObject* new_obj = new MTrackObject;
		new_obj->updateNewState(m_tmp_objs[i], m_cur_update_time);
		
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
			if(m_cur_update_time - (*it)->m_last_active > 0){
				(*it)->m_is_active = false;

				list<MObject>::iterator last_state_iter = (*it)->m_object_hist.end();
				last_state_iter--;

				MObject last_state = *last_state_iter;
				MOVING_DIRECT new_count = (*it)->updateCounting(last_state);
				if(new_count == MOVING_DIRECT::ENTER)
					m_num_enter++;
				else if(new_count == MOVING_DIRECT::LEAVE)
					m_num_leave++;

				if(new_count != MOVING_DIRECT::UNDENTIFINED){
					(*it)->m_last_change_direct_state = last_state;
				}
			}

		}
		else if(m_cur_update_time - (*it)->m_last_active > TIME_OUT_OBJECT * 1000){

			list<MObject>::iterator last_state_iter = (*it)->m_object_hist.end();
			last_state_iter--;

			MObject last_state = *last_state_iter;
			MOVING_DIRECT new_count = (*it)->updateCounting(last_state);
			if(new_count == MOVING_DIRECT::ENTER)
				m_num_enter++;
			else if(new_count == MOVING_DIRECT::LEAVE)
				m_num_leave++;

			it = m_track_objs.erase(it);
			//(*it)->m_is_active = false;
		}
	}
}