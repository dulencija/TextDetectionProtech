#include "textDetector.h"


/**
* \brief CameraElement::CameraElement - destructor.
*/
protech::TextDetector::~TextDetector()
{
	clear();
}

/**
* \brief ModulePathA - This method finds executing module directory.
* \return std::string directory path.
*/
std::string protech::TextDetector::ModulePathA()
{
	TCHAR buffer[256];
	GetModuleFileName(NULL, buffer, MAX_PATH);

	size_t origsize = wcslen(buffer) + 1;
	const size_t newsize = 256;
	size_t convertedChars = 0;
	char tmp_char[newsize];
	wcstombs_s(&convertedChars, tmp_char, origsize, buffer, _TRUNCATE);

	std::string::size_type pos = std::string(tmp_char).find_last_of("\\/");
	return std::string(tmp_char).substr(0, pos);
}


/**
* \brief initialize - Method for initializing Tesseract and setting up parameters.
* \param [in] std::string _OUTPUT_FOLDER_PATH - output folder.
*/
void  protech::TextDetector::initialize(std::string _OUTPUT_FOLDER_PATH)
{
	OUTPUT_FOLDER_PATH = _OUTPUT_FOLDER_PATH;
	std::string tessdataPath = ModulePathA() + "//";
	tessEngine.initialize(tessdataPath);
}


/**
* \brief clear - Method for clearing all remaining data (mainly Tesseract data).
*/
void  protech::TextDetector::clear()
{
	tessEngine.clear();
	tessEngine.~TesseractEngine();
}


/**
* \brief floodFillNewRects - method for getting connected components and their rects in binary image.
* \param [in] cv::Mat & foreground - Input image.
* \param [out] cv::Mat & out_mask - Output mask of "good" objects.
* \param [in] int AREA_DOWN - Threshold for deleting small artefacts.
* \param [in] int AREA_UP - Threshold for deleting big artefacts.
* \param [in] double elongationDown Lower - Lower limit threshold for ratio of width and height of object bounding box.
* \param [in] double elongationUp Upper - Upper limit threshold  for ratio of width and height of object bounding box.
* \param [in] double rectangularityDown - Ratio of object area and bounding box area.
* \param [out] std::vector<std::vector<cv::Point>> allObjects - Vector of separated objects.
* \param [out] std::vector<std::vector<cv::Point>> allRects - Vector of separated objects bounding box.
*/
void protech::TextDetector::floodFillNewRects(cv::Mat & foreground, cv::Mat & out_mask, int AREA_DOWN, int AREA_UP, double elongationDown, double elongationUp, double rectangularityDown, std::vector<std::vector<cv::Point>> & allObjects, std::vector<cv::Rect> & allRects)
{
	//floodfill
	cv::Mat tmp_mask;
	tmp_mask.create(cv::Size(foreground.cols + 2, foreground.rows + 2), foreground.type());
	tmp_mask.setTo(0);
	tmp_mask.total();
	int    connectivity = 8;
	int    new_mask_val = 2;

	bool continue_loop = false;

	std::vector<cv::Point> tmp_cpVector;

	uchar * ptr_input = foreground.data;
	uchar * ptr_tmp_mask = tmp_mask.data;
	uchar * ptr_outMask = out_mask.data;


	for (int i = 0; i < (int)foreground.rows; i++)
	{
		for (int j = 0; j < (int)foreground.cols; j++)
		{
			if (ptr_input[foreground.step*i + j] > 0 && ptr_tmp_mask[tmp_mask.step*i + j] == 0)
			{
				cv::Point seed = cv::Point(j, i);
				cv::Rect tmp_rect;

				new_mask_val += 1;
				if (new_mask_val > 255)
				{
					new_mask_val = 2;
				}

				int flags = connectivity + CV_FLOODFILL_FIXED_RANGE;
				int area = cv::floodFill(foreground, tmp_mask, seed, cv::Scalar((double)new_mask_val), &tmp_rect, cv::Scalar(0), cv::Scalar(0), flags);

				if (area > 0)
				{
					if ((area < AREA_DOWN) || (area > AREA_UP))//Deleting small and big artefacts
					{
						for (int p = tmp_rect.y; p < tmp_rect.y + tmp_rect.height; p++)
						{
							for (int q = tmp_rect.x; q < tmp_rect.x + tmp_rect.width; q++)
							{
								if (ptr_input[foreground.step*p + q] == new_mask_val)
								{
									ptr_input[foreground.step*p + q] = 0;
									ptr_tmp_mask[tmp_mask.step*p + q] = 1;
								}
							}
						}
					}
					else
					{
						for (int p = tmp_rect.y; p < tmp_rect.y + tmp_rect.height; p++)
						{
							for (int q = tmp_rect.x; q < tmp_rect.x + tmp_rect.width; q++)
							{
								if (ptr_input[foreground.step*p + q] == new_mask_val)
								{
									ptr_input[foreground.step*p + q] = 1;
									ptr_tmp_mask[tmp_mask.step*p + q] = 1;
									tmp_cpVector.push_back(cv::Point(q, p));
								}
							}
						}
					}

					if (!tmp_cpVector.empty())
					{
						double elongation = (double)(tmp_rect.width) / (double)(tmp_rect.height);//b.box w and h ratio
						double rectangularity = (double)(area) / (double)(tmp_rect.width * tmp_rect.height);

						if ((rectangularity > rectangularityDown) && (elongation > elongationDown) && (elongation < elongationUp))
						{
							allObjects.push_back(tmp_cpVector);
							allRects.push_back(tmp_rect);
						}
						tmp_cpVector.clear();
					}
				}
			}
		}
	}

	out_mask.setTo(0);
	for (unsigned int i = 0; i < allObjects.size(); i++)
	{
		for (unsigned int j = 0; j < allObjects[i].size(); j++)
		{
			ptr_outMask[out_mask.step*allObjects[i][j].y + allObjects[i][j].x] = 255;
		}
	}

	if (tmp_mask.data != NULL)
		tmp_mask.release();
}


/**
* \brief getTextInfoTesseract - method for getting OCR text from given image.
* \param [in] const cv::Mat & inputImg -  image for text detection.
* \param [out] std::string & result - detected text.
*/
void protech::TextDetector::getTextInfoTesseract(const cv::Mat & inputImg, std::string & result)
{
	try{
		cv::Mat openCVImage2;
		inputImg.copyTo(openCVImage2);

		tessEngine.setImage(openCVImage2);

		result = tessEngine.processPage().string();

		if (!openCVImage2.empty())
			openCVImage2.release();
	}
	catch (std::exception & e)
	{
		;
	}
	catch (char * str)
	{
		;
	}
	catch (...)
	{
		;
	}
}


/**
* \brief textDetectionFunction - function for text detection on a given image.
*Detection is based on contours of edges and Tesseract.
*Result is saved as two images with same name as original image + sufix in output folder.
* \param [in] cv::Mat& currentframe - image for text detection.
* \param [in] std::string img_name - image name.
*/
void protech::TextDetector::textDetectionFunction(cv::Mat& currentframe, std::string img_name)
{
	cv::Mat large;
	cv::resize(currentframe, large, cv::Size(1400, (int)(currentframe.rows / (currentframe.cols / 1400.0))), 0, 0, CV_INTER_NN); // CV_INTER_LANCZOS4

	cv::Mat smallImg, currentframeBkp, currentframeBkp1;
	cv::cvtColor(large, smallImg, CV_BGR2GRAY);
	large.copyTo(currentframeBkp);
	large.copyTo(currentframeBkp1);

	// morphological gradient
	cv::Mat grad;
	cv::Mat morphKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	cv::morphologyEx(smallImg, grad, cv::MORPH_GRADIENT, morphKernel);

	// binarize
	cv::Mat bw;
	cv::threshold(grad, bw, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

	cv::Mat connected;
	morphKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 2));
	cv::morphologyEx(bw, connected, cv::MORPH_CLOSE, morphKernel);

	// find contours
	cv::Mat mask = cv::Mat::zeros(bw.size(), CV_8UC1);
	cv::Mat rectMask = cv::Mat::zeros(bw.size(), CV_8UC1);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(connected, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	// filter contours
	cv::Mat finalMask = cv::Mat::zeros(bw.size(), CV_8UC1);

	for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	{
		cv::Rect rect = cv::boundingRect(contours[idx]);
		cv::Mat maskROI(mask, rect);
		maskROI = cv::Scalar(0, 0, 0);
		// fill the contour
		cv::drawContours(mask, contours, idx, cv::Scalar(255, 255, 255), CV_FILLED);

		// ratio of non-zero pixels in the filled region
		double r = (double)cv::countNonZero(maskROI) / (rect.width*rect.height);

		double rara = (double)(min(rect.width, rect.height)) / max(rect.width, rect.height) * rect.width * rect.height;//relative aspect ratio * area
		double rarav = (double)(min(rect.width, rect.height)) / max(rect.width, rect.height) * (rect.width + rect.height) / 2;//relative aspect ratio * average side value (h+w / 2)


		//filtering
		if ((r > 0.3) &&
			(rect.height > 10 && rect.width > 10) && //width and hight are higher than 10 px
			(rect.width * rect.height < 100000) && //area must be less than 200.000 px <==
			(rect.width * rect.height > 200) && //area must be larger than 100 px <==
			(rara < 6000) &&
			(rarav < 50) &&
			(rect.height < 75))

		{
			cv::rectangle(large, rect, cv::Scalar(0, 255, 0), 2);
			cv::rectangle(rectMask, rect, cv::Scalar(255), -1);
			cv::drawContours(finalMask, contours, idx, cv::Scalar(255, 255, 255), CV_FILLED, 8, hierarchy);
		}
		else
		{
			;
		}

		if (maskROI.data != NULL)
			maskROI.release();
	}
	//finalMask = finalMask & bw;

	//rect processing
	cv::Mat connectedRects;
	cv::Mat morphKernel_1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(21, 15));
	cv::morphologyEx(rectMask, connectedRects, cv::MORPH_CLOSE, morphKernel_1);

	cv::Mat connectedRectsFF = cv::Mat::zeros(connectedRects.size(), CV_8UC1);
	cv::Mat connectedRectsRectMask = cv::Mat::zeros(connectedRects.size(), CV_8UC1);

	std::vector<std::vector<cv::Point>> v_new_component02;
	std::vector<cv::Rect> v_new_component_rects02;

	floodFillNewRects(connectedRects, connectedRectsFF, 2000, 1000000, 0.025, 40, 0.3, v_new_component02, v_new_component_rects02);//ff for big rects

	for (int rc = 0; rc < v_new_component_rects02.size(); rc++)
	{
		cv::Mat tmpImage(smallImg, v_new_component_rects02[rc]);
		cv::Mat tmpMask(connectedRectsFF, v_new_component_rects02[rc]);
		bitwise_and(tmpImage, tmpMask, tmpImage);
		std::string tmoStringRes = "";
		getTextInfoTesseract(tmpImage, tmoStringRes);
		std::string newRes = boost::erase_all_regex_copy(tmoStringRes, boost::regex("[^a-zA-Z0-9]+"));

		if (newRes.length() < 3)//2 3 4
		{
			cv::rectangle(connectedRectsFF, v_new_component_rects02[rc], cv::Scalar(0), -1);
		}
		else
		{
			cv::rectangle(currentframeBkp, v_new_component_rects02[rc], cv::Scalar(0, 255, 0), 2);
			cv::rectangle(connectedRectsRectMask, v_new_component_rects02[rc], cv::Scalar(255), -1);
		}

		if (tmpImage.data != NULL)
			tmpImage.release();
		if (tmpMask.data != NULL)
			tmpMask.release();
	}

	//mask results
	cv::Mat morphKernel_2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(23, 23));
	cv::dilate(connectedRectsFF, connectedRectsFF, morphKernel_2);

	connectedRectsFF = connectedRectsFF & connectedRectsRectMask;

	cv::Mat connectedRectsFF_3C;
	connectedRectsFF_3C.create(connectedRectsFF.size(), CV_8UC3);
	cv::cvtColor(connectedRectsFF, connectedRectsFF_3C, CV_GRAY2BGR);

	currentframeBkp1 = currentframeBkp1 + connectedRectsFF_3C;

	//write imgs
	cv::imwrite(OUTPUT_FOLDER_PATH + "//" + std::string(img_name + "_masked.jpg"), currentframeBkp1);
	cv::imwrite(OUTPUT_FOLDER_PATH + "//" + std::string(img_name + "_rects.jpg"), currentframeBkp);

	//clear data
	if (large.data != NULL)
		large.release();
	if (smallImg.data != NULL)
		smallImg.release();
	if (currentframeBkp.data != NULL)
		currentframeBkp.release();
	if (currentframeBkp1.data != NULL)
		currentframeBkp1.release();
	if (grad.data != NULL)
		grad.release();
	if (morphKernel.data != NULL)
		morphKernel.release();
	if (bw.data != NULL)
		bw.release();
	if (connected.data != NULL)
		connected.release();
	if (mask.data != NULL)
		mask.release();
	if (rectMask.data != NULL)
		rectMask.release();
	if (finalMask.data != NULL)
		finalMask.release();

	if (connectedRects.data != NULL)
		connectedRects.release();
	if (morphKernel_1.data != NULL)
		morphKernel_1.release();
	if (connectedRectsFF.data != NULL)
		connectedRectsFF.release();
	if (connectedRectsRectMask.data != NULL)
		connectedRectsRectMask.release();

	if (morphKernel_2.data != NULL)
		morphKernel_2.release();
	if (connectedRectsFF_3C.data != NULL)
		connectedRectsFF_3C.release();

	if (!contours.empty())
		contours.clear();
	if (!hierarchy.empty())
		hierarchy.clear();
	if (!v_new_component02.empty())
		v_new_component02.clear();
	if (!v_new_component_rects02.empty())
		v_new_component_rects02.clear();
}