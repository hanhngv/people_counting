#include "include.h"
#include "MIP.h"
#include "MPeopleCounting.h"

int main()
{
	/*Mat img = imread("..//inputs//IMGs//abc.jpg");
	if(img.rows != 0)
	{
		imshow("violet", img);
		cout <<" He' nho" << endl;
		cvWaitKey(0);
	}
	else
		cout <<"Dau co hinh dau :/" << endl;*/
	//Mat img1 = imread("..//inputs//IMGs//in000034.jpg"); 
	//Mat img2 = imread("..//inputs//IMGs//in000042.jpg");

	//time_t begin = clock();
	//MIP::compareImg(img1, img2);
	//cout << clock() - begin << endl;
	//cvWaitKey(0);

	MPeopleCounting counting_model;

	time_t begin = clock();
	counting_model.begin("..//inputs//Vid_03.mp4");
	while(counting_model.b_running)
		Sleep(1000);

	counting_model.release();
	
	return 0;
}
