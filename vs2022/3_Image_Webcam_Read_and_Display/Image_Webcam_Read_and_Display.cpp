#include "pch.h"

// Include Libraries
#include<opencv2/opencv.hpp>

int main() {

    //Declaring a matrix to load the frames
    cv::Mat image;

    //Declaring an object to capture stream of frames from default camera using the direct show backend.    
    cv::VideoCapture camera(0, cv::CAP_DSHOW);

    //This section prompt an error message if no video stream is found
    if (!camera.isOpened()) {
        std::cerr << "No video stream detected" << std::endl;

        // Return error code
        return 1;
    }

    // Declaring the window to show the video
    cv::namedWindow("Webcam Player");

    //Taking an everlasting loop to show the video
    while (true) {
        camera >> image;

        //Breaking the loop if no video frame is detected
        if (image.empty()) {
            break;
        }
        //Showing the video//
        cv::imshow("Webcam Player", image);

        //Allowing 25 milliseconds frame processing time and initiating break condition
        char c = (char)cv::waitKey(25);
        //If 'Esc' is entered break the loop//
        if (c == 27) {
            break;
        }
    }
    //Releasing the buffer memory
    camera.release();

    cv::destroyAllWindows();

    return 0;
}