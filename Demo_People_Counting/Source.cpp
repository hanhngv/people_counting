#include "include.h"
#include "MIP.h"
#include "MPeopleCounting.h"

int main()
{
	MPeopleCounting counting_model;

	counting_model.begin("..//inputs//Vid_01.mp4", 50);
	while(counting_model.b_running)
		Sleep(1000);

	counting_model.release();
	
	return 0;
}
