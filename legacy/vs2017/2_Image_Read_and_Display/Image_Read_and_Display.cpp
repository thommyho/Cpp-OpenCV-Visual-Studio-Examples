#include "pch.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <Windows.h>
// Include Libraries
#include<opencv2/opencv.hpp>
#include<experimental/filesystem>

// Calling the application from Visual Studio (e.g. Debugging) will have the Project-Directory as Starting/Working Directory.
// The test image is deployed next to application, so that a direct double-click works as well.
// This code lines extract the absolute path to the application (module) regardless if called from the IDE or directly from the Explorer.
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
TCHAR   DllPath[MAX_PATH] = { 0 };

int main()
{
    // Get the absolute module path
    GetModuleFileName((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));

    // Use convient and object oriented path library.
    std::experimental::filesystem::path pathModule(DllPath);
    // Strip the application name from the path by getting the parent path and extending the path with the name of the test imgage.
    auto pathFileTestImage = pathModule.parent_path() / "starry_night.jpg";

    // Read an image 
    cv::Mat img = cv::imread(pathFileTestImage.generic_string());
    if (img.empty())
    {
        // Reading image failed. Bail out.
        // Exit program with failure status.
        return 1;
    }

    // Display the image.
    cv::imshow("Dear Vincent Willem van Gogh, what a beautiful image...", img);

    // Wait for a keystroke.   
    cv::waitKey(0);

    // Destroys all the windows created                         
    cv::destroyAllWindows();

    // Exit Program
    return 0;
}