#include "pch.h"

// Include Libraries
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// ToDO: Add more explanation

void CaptureAndProcessFrame(cv::VideoCapture &camera, bool first)
{
	if (first)
	{
		// Declaring the window to show the video
		cv::namedWindow("Original Image");

		cv::namedWindow("Gray Image");
		cv::namedWindow("Canny Image");
		cv::Mat image;
		while (true)
		{
			camera >> image;
			//Breaking the loop if no video frame is detected
			if (!image.empty())
			{
				break;
			}
		}
		//Taking an everlasting loop to show the video
		cv::Mat contours;
		cv::Mat gray_image;
		//Showing the video//
		cv::imshow("Original Image", image);
		cvtColor(image, gray_image, cv::COLOR_RGB2GRAY);

		cv::Canny(image, contours, 10, 350);

		cv::imshow("Gray Image", gray_image);

		cv::imshow("Canny Image", contours);

		// ToDo: Remove absolute pixel shifting uglyness
		auto image_rect_orig = cv::getWindowImageRect("Original Image");
		cv::moveWindow("Gray Image", image_rect_orig.x + image_rect_orig.width + 10, image_rect_orig.y - 30);
		auto image_rect_gray = cv::getWindowImageRect("Gray Image");
		cv::moveWindow("Canny Image", image_rect_gray.x + image_rect_gray.width + 10, image_rect_gray.y - 30);
	}
	else
	{
		//Taking an everlasting loop to show the video
		cv::Mat contours;
		cv::Mat gray_image;
		cv::Mat image;
		while (true)
		{
			camera >> image;
			//Breaking the loop if no video frame is detected
			if (image.empty())
			{
				break;
			}

			//Showing the video//
			cv::imshow("Original Image", image);
			cvtColor(image, gray_image, cv::COLOR_RGB2GRAY);

			cv::Canny(image, contours, 10, 350);

			cv::imshow("Gray Image", gray_image);

			cv::imshow("Canny Image", contours);
			//Allowing 25 milliseconds frame processing time and initiating break condition
			char c = (char)cv::waitKey(25);
			//If 'Esc' is entered break the loop//
			if (c == 27)
			{
				break;
			}
		}
	}
}
int main()
{

	//Declaring a matrix to load the frames
	cv::Mat image;

	//Declaring an object to capture stream of frames from default camera using the direct show backend.
	cv::VideoCapture camera(0, cv::CAP_DSHOW);

	//This section prompt an error message if no video stream is found
	if (!camera.isOpened())
	{
		std::cerr << "No video stream detected" << std::endl;

		// Return error code
		return 1;
	}

	// Calling it once, to set webcam default size and arrange the windows accordingly.
	CaptureAndProcessFrame(camera, true);

	// Calling to continuesly grabbing and processing the images
	CaptureAndProcessFrame(camera, false);

	//Releasing the buffer memory
	camera.release();

	cv::destroyAllWindows();
	return 0;
}