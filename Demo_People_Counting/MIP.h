#pragma once

#include "include.h"

class MIP{

public:
	//static float compareImg(Mat &img1, Mat &img2, int average_size);
	static void removeNoiseCircle(vector<Point2f> &center, vector<float> &radius, int average_size);
};
