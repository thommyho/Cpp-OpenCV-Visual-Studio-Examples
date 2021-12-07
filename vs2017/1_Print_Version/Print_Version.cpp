#include "pch.h"

// Include Libraries
#include<opencv2/opencv.hpp>
#include<iostream>

int main()
{    
    std::cout << "OpenCV version : " << CV_VERSION << std::endl;
    std::cout << "Major version : " << CV_MAJOR_VERSION << std::endl;
    std::cout << "Minor version : " << CV_MINOR_VERSION << std::endl;
    std::cout << "Subminor version : " << CV_SUBMINOR_VERSION << std::endl;

    // Exit Program
    return 0;    
}