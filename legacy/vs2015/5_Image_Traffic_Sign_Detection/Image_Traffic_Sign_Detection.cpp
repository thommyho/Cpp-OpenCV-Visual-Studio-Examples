#include "pch.h"

// Include Libraries
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <string>

#include <fstream>
#include <sstream>
#include <iostream>

#include<experimental/filesystem>

// Calling the application from Visual Studio (e.g. Debugging) will have the Project-Directory as Starting/Working Directory.
// The test image is deployed next to application, so that a direct double-click works as well.
// This code lines extract the absolute path to the application (module) regardless if called from the IDE or directly from the Explorer.
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
TCHAR   DllPath[MAX_PATH] = { 0 };

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Initialize the parameters
float m_fConfidenceThreshold = 0.5f; // Confidence threshold
float m_fNonMaxSuppThreshold = static_cast<float>(0.4);  // Non-maximum suppression threshold
int m_iInputImageWidth = 416;  // Width of network's input image
int m_iInputImageHeight = 416; // Height of network's input images
std::vector<cv::String> m_vecssClasses;

// Draw the predicted bounding box
void DrawBoundingBox(int iClassId, float fConf, int iLeft, int iTop, int iRight, int iBottom, cv::Mat& ImFrame)
{
	//Draw a rectangle displaying the bounding box
	cv::rectangle(ImFrame, cv::Point(iLeft, iTop), cv::Point(iRight, iBottom), cv::Scalar(255, 178, 50), 3);

	//Get the label for the class name and its confidence
	std::string ssLabel = cv::format("%.2f", fConf);
	if (!m_vecssClasses.empty())
	{
		CV_Assert(iClassId < (int)m_vecssClasses.size());
		ssLabel = m_vecssClasses[iClassId] + ":" + ssLabel;
	}

	//Display the label at the top of the bounding box
	int iBaseLine;
	cv::Size labelSize = cv::getTextSize(ssLabel, cv::FONT_HERSHEY_TRIPLEX, 0.5, 1, &iBaseLine);
	iTop = cv::max(iTop, labelSize.height);
	cv::rectangle(ImFrame, cv::Point(iLeft, iTop - static_cast<int>(round(1.5 * labelSize.height))),
		cv::Point(iLeft + static_cast<int>(round(1.5 * labelSize.width)),
			iTop + iBaseLine), cv::Scalar(255, 255, 255), cv::FILLED);
	cv::putText(ImFrame, ssLabel, cv::Point(iLeft, iTop), cv::FONT_HERSHEY_TRIPLEX, 0.75, cv::Scalar(0, 0, 0), 1);
}


// Remove the bounding boxes with low confidence using non-maxima suppression
void PostProcess(cv::Mat& Imframe, const std::vector<cv::Mat>& BoundingBoxOutputs)
{
	std::vector<int> veciClassIds;
	std::vector<float> vecfConfidences;
	std::vector<cv::Rect> vecsBoxes;

	for (size_t i = 0; i < BoundingBoxOutputs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* pfData = (float*)BoundingBoxOutputs[i].data;
		for (int j = 0; j < BoundingBoxOutputs[i].rows; ++j, pfData += BoundingBoxOutputs[i].cols)
		{
			cv::Mat sScores = BoundingBoxOutputs[i].row(j).colRange(5, BoundingBoxOutputs[i].cols);
			cv::Point sClassIdPoint;
			double dConfidence;
			// Get the value and location of the maximum score
			cv::minMaxLoc(sScores, 0, &dConfidence, 0, &sClassIdPoint);
			if (dConfidence > m_fConfidenceThreshold)
			{
				int iCenterX = (int)(pfData[0] * Imframe.cols);
				int iCenterY = (int)(pfData[1] * Imframe.rows);
				int iWidth = (int)(pfData[2] * Imframe.cols);
				int iHeight = (int)(pfData[3] * Imframe.rows);
				int iLeft = iCenterX - iWidth / 2;
				int iTop = iCenterY - iHeight / 2;

				veciClassIds.push_back(sClassIdPoint.x);
				vecfConfidences.push_back((float)dConfidence);
				vecsBoxes.push_back(cv::Rect(iLeft, iTop, iWidth, iHeight));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	std::vector<int> vecIndices;
	cv::dnn::NMSBoxes(vecsBoxes, vecfConfidences, m_fConfidenceThreshold, m_fNonMaxSuppThreshold, vecIndices);
	for (size_t i = 0; i < vecIndices.size(); ++i)
	{
		int idx = vecIndices[i];
		cv::Rect sbox = vecsBoxes[idx];
		DrawBoundingBox(veciClassIds[idx], vecfConfidences[idx], sbox.x, sbox.y,
			sbox.x + sbox.width, sbox.y + sbox.height, Imframe);
	}
}


// Get the names of the output layers
std::vector<cv::String> GetOutputsNames(const cv::dnn::Net& sNet)
{
	static std::vector<cv::String> vecssNames;
	if (vecssNames.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		std::vector<int> veciOutLayers = sNet.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		std::vector<cv::String> vecssLayersNames = sNet.getLayerNames();

		// Get the names of the output layers in names
		vecssNames.resize(veciOutLayers.size());
		for (size_t i = 0; i < veciOutLayers.size(); ++i)
			vecssNames[i] = vecssLayersNames[veciOutLayers[i] - 1];
	}
	return vecssNames;
}



int main()
{

	// Get the absolute module path
	GetModuleFileName((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));

	// Use convient and object oriented path library.
	std::experimental::filesystem::path pathModule(DllPath);
	// Strip the application name from the path by getting the parent path and extending the path with the name of the test imgage.

	// Load names of classes 
	auto pathClassesFile =  pathModule.parent_path() / "TrafficSignDetection" / "gstdb.names";
	std::ifstream ifs(pathClassesFile.c_str());
	std::string ssLine;
	while (getline(ifs, ssLine)) m_vecssClasses.push_back(ssLine);

	// Give the configuration and weight files for the model
	auto pathModelConfiguration = std::experimental::filesystem::path(pathModule.parent_path() / "TrafficSignDetection" / "yolov3.cfg");
	auto pathModelWeights = std::experimental::filesystem::path(pathModule.parent_path() / "TrafficSignDetection" / "yolov3_best.weights");
	cv::String ssModelConfiguration = pathModelConfiguration.generic_string();
	cv::String ssModelWeights = pathModelWeights.generic_string();
	
	// Load the network
	cv::dnn::Net SDeepnet = cv::dnn::readNetFromDarknet(ssModelConfiguration, ssModelWeights);
	SDeepnet.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
	SDeepnet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
	//  net.setPreferableBackend(DNN_BACKEND_CUDA);//if GPU
	//	net.setPreferableTarget(DNN_TARGET_CUDA);//if GPU
	//SDeepnet.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);	// if GPU, but not 

	// Open a video file or an image file or a camera stream.
	cv::VideoCapture sVidCapture;
	//VideoWriter video;
	cv::Mat sFrame, sBlob;

	// Create a window
	static const std::string ssWinName = "Traffic Sign Recognition";
	cv::namedWindow(ssWinName, cv::WINDOW_NORMAL);

	auto pathVideoFile = std::experimental::filesystem::path(pathModule.parent_path() / "TrafficSignDetection"/ "opencvMunichdrive.mp4");
	sVidCapture.open(pathVideoFile.generic_string());
	// Process frames.
	while (cv::waitKey(1) < 0)
	{
		// get frame from the video
		sVidCapture >> sFrame;
		if (sFrame.empty()) 
		{
			cv::waitKey(3000);
			break;
		}
		// Create a 4D blob from a frame.
		cv::dnn::blobFromImage(sFrame, sBlob, 1 / 255.0, cv::Size(m_iInputImageWidth, m_iInputImageHeight), cv::Scalar(0, 0, 0), true, false);

		//Sets the input to the network
		SDeepnet.setInput(sBlob);

		// Runs the forward pass to get output of the output layers
		std::vector<cv::Mat> outs;
		SDeepnet.forward(outs, GetOutputsNames(SDeepnet));

		// Remove the bounding boxes with low confidence
		PostProcess(sFrame, outs);
		imshow(ssWinName, sFrame);
	}
	sVidCapture.release();
	return 0;
}
