#include "MIP.h"


RNG rng(12345);

float MIP::compareImg(Mat &img1, Mat &img2, int average_size)
{
	if(img1.rows != img2.rows)
		return 1.0f;
	if(img1.cols != img2.cols)
		return 1.0f;

	Mat img_1 = img1.clone();
	Mat img_2 = img2.clone();

	if(img_1.channels() != 1)
		cvtColor(img_1, img_1, CV_BGR2GRAY);
	if(img_2.channels() != 1)
		cvtColor(img_2, img_2, CV_BGR2GRAY);

	cv::Size kernel_size(3, 3);
	if(img_1.cols > 80)
	{
		if(img_1.cols < 160)
			kernel_size = cv::Size(5, 5);
		else
		{
			resize(img_1, img_1, cv::Size(200, img_1.rows * (200.0f / img_1.cols)));
			resize(img_2, img_2, cv::Size(200, img_2.rows * (200.0f / img_2.cols)));
		}
	}
	GaussianBlur(img_1, img_1, kernel_size, 0);
	GaussianBlur(img_2, img_2, kernel_size, 0);

	Mat grad_x_1, grad_y_1, abs_grad_x_1, abs_grad_y_1;
	Sobel(img_1, grad_x_1, CV_16S, 1, 0);
	convertScaleAbs(grad_x_1, abs_grad_x_1);
	Sobel(img_1, grad_y_1, CV_16S, 0, 1);
	convertScaleAbs(grad_y_1, abs_grad_y_1);

	Mat grad_x_2, grad_y_2, abs_grad_x_2, abs_grad_y_2;
	Sobel(img_2, grad_x_2, CV_16S, 1, 0);
	convertScaleAbs(grad_x_2, abs_grad_x_2);
	Sobel(img_2, grad_y_2, CV_16S, 0, 1);
	convertScaleAbs(grad_y_2, abs_grad_y_2);

	Mat sub_x, sub_x_inv, sub_x_merg;
	cv::subtract(abs_grad_x_1, abs_grad_x_2, sub_x);
	cv::subtract(abs_grad_x_2, abs_grad_x_1, sub_x_inv);
	bitwise_or(sub_x, sub_x_inv, sub_x_merg);

	Mat sub_y, sub_y_inv, sub_y_merg;
	cv::subtract(abs_grad_y_1, abs_grad_y_2, sub_y);
	cv::subtract(abs_grad_y_2, abs_grad_y_1, sub_y_inv);
	bitwise_or(sub_y, sub_y_inv, sub_y_merg);

	Mat sub_xy;
	bitwise_or(sub_x_merg, sub_y_merg, sub_xy);

	Mat sub_thresh;
	cv::threshold(sub_xy, sub_thresh, 90, 255, CV_THRESH_BINARY);

	Mat disp;
	Mat img_merg;
	cv::hconcat(img_1, img_2, img_merg);
	Mat img_sobel_x;
	//cv::hconcat(abs_grad_x_1, abs_grad_x_2, img_sobel_x);
	Mat img_sobel_y;
	//cv::hconcat(abs_grad_y_1, abs_grad_y_2, img_sobel_y);
	Mat img_sub;
	//cv::hconcat(sub_x_merg, sub_y_merg, img_sub);
	

	int erosion_elem = 0;
	int erosion_size = 0;
	int dilation_elem = 0;
	int dilation_size = 3;
	int const max_elem = 2;
	int const max_kernel_size = 21;

	int dilation_type;
	if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
	else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
	else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }

	Mat element = getStructuringElement( dilation_type,
		Size( 2*dilation_size + 1, 2*dilation_size+1 ),
		Point( dilation_size, dilation_size ) );
	/// Apply the dilation operation
	dilate( sub_thresh, sub_thresh, element );

	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	//Canny( src_gray, canny_output, thresh, thresh*2, 3 );
	/// Find contours
	findContours( sub_thresh.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );


	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<float>radius( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	{
		approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
		boundRect[i] = boundingRect( Mat(contours_poly[i]) );
		minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
	}

	removeNoiseCircle(center, radius, average_size);

	for( int i = 0; i < center.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		//drawContours( sub_xy, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
		//rectangle( sub_xy, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
		circle( img_merg, center[i], (int)radius[i] * 0.75, color, 2, 8, 0 );
	}



	//cv::vconcat(img_merg, img_sobel_x, disp);
	//cv::vconcat(disp, img_sobel_y, disp);
	//cv::vconcat(disp, img_sub, disp);
	//cv::vconcat(disp, img_subxy, disp);
	Mat img_subxy;
	cv::hconcat(sub_xy, sub_thresh, img_subxy);
	cv::vconcat(img_merg, img_subxy, disp);

	if(disp.rows > 800)
		resize(disp, disp, cv::Size(disp.cols * (800.0f / disp.rows), 800));

	cv::imshow("compare...", disp);
	cv::waitKey(1);

	float diff = countNonZero(sub_thresh);
	return diff /= (sub_thresh.rows * sub_thresh.cols);

	
	//AfxThread
}

void MIP::removeNoiseCircle(vector<Point2f> &center, vector<float> &radius, int average_size)
{
	for(int i = 0; i < center.size(); i++){
		if(radius[i] < average_size / 2.5)
		{
			center.erase(center.begin() + i);
			radius.erase(radius.begin() + i);
			i--;
		}
		else if(radius[i] > average_size / 1.5)
		{
			center.erase(center.begin() + i);
			radius.erase(radius.begin() + i);
			i--;
		}
	}
}