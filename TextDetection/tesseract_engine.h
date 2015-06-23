
#ifndef OCR_ENGINE_TESSERACT_H
#define OCR_ENGINE_TESSERACT_H


#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

	class TesseractEngine
	{

	public:
		TesseractEngine();

		~TesseractEngine();

		/* interface functions */
		bool initialize(std::string tessdataPath);
		
		void clear();
		void end();

		void setPageSegMode(tesseract::PageSegMode segMode);

		void setImage(const cv::Mat &image);

		bool setWhiteList(const char *whitelist);

		bool setVariable(std::string name, std::string value);

		STRING processPage();

	private:
		std::string m_lang;
		Pix* m_pix;

		tesseract::TessBaseAPI m_tessBase;
		tesseract::OcrEngineMode m_mode;
		tesseract::PageSegMode m_pageSegMode;

		Pix* createPix(int width, int height, int depth, int widthStep, const int *imageData);

		std::string ModulePath();
	};


#endif