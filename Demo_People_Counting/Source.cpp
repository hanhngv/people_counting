#include "include.h"
#include "MIP.h"
#include "MPeopleCounting.h"
#include "MInputFrame.h"

int main()
{
	MPeopleCounting counting_model;

	MInputFrame* input_obj = new MInputFromVideo();
	//input_obj->init("..//inputs//Vid_02.mp4");
	//counting_model.begin(input_obj, 70);

	input_obj->init("..//inputs//Vid_01.mp4");
	counting_model.begin(input_obj, 50);

	while(counting_model.b_running)
		Sleep(1000);

	counting_model.release();
	
	delete input_obj;
	input_obj = NULL;

	return 0;
}
