
#ifndef TEXT_DETECTOR_H
#define TEXT_DETECTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <set>
#include <fstream>
#include <stdio.h>

#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "tesseract_engine.h"


namespace protech
{
	class TextDetector
	{
	private:
		TesseractEngine tessEngine;//!< Tesseract engine object.
		std::string OUTPUT_FOLDER_PATH;//!< output folder path.

	public:
		TextDetector(){};
		~TextDetector();		
		std::string ModulePathA();
		void initialize(std::string _OUTPUT_FOLDER_PATH);
		void clear();
		void floodFillNewRects(cv::Mat & foreground, cv::Mat & out_mask, int AREA_DOWN, int AREA_UP, double elongationDown, double elongationUp, double rectangularityDown, std::vector<std::vector<cv::Point>> & allObjects, std::vector<cv::Rect> & allRects);
		void getTextInfoTesseract(const cv::Mat & inputImg, std::string & result);
		void textDetectionFunction(cv::Mat& currentframe, std::string img_name);
	};
}
#endif

