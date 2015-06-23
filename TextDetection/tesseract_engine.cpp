
#include "tesseract_engine.h"


/**
* \brief ModulePathA - This method finds executing module directory.
* \return std::string directory path.
*/
std::string TesseractEngine::ModulePath()
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
* \brief TesseractEngine::TesseractEngine - constructor.
*/
TesseractEngine::TesseractEngine()
{
	m_lang = "eng";
	setPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	m_mode = tesseract::OEM_TESSERACT_ONLY;
	//setSpeed(Engine::FAST);
}


/**
* \brief TesseractEngine::~TesseractEngine - destructor.
*/
TesseractEngine::~TesseractEngine()
{
	clear();
	end();
}


/**
* \brief initialize - This method initialize tesseract data.
* \return bool - true if data is initialized, otherwise false.
*/
bool TesseractEngine::initialize(std::string tessdataPath)
{
	//***//std::string tessdata(ModulePath() + "/Tesseract-OCR/");
	std::string tessdata(tessdataPath);
	
	int success = m_tessBase.Init(tessdata.c_str(), m_lang.c_str(), m_mode);

	//std::cout << "Tesseract Open Source OCR Engine with Leptonica: " << tesseract::TessBaseAPI::Version() << std::endl;

	m_tessBase.SetPageSegMode(m_pageSegMode);

	if (!m_tessBase.SetVariable("load_system_dawg", "F"))
		printf("Setting load_system_dawg variable failed!!!\n");

	if (!m_tessBase.SetVariable("load_freq_dawg", "F"))
		printf("Setting load_freq_dawg variable failed!!!\n");

	if (!m_tessBase.SetVariable("language_model_penalty_non_dict_word", "1"))
		printf("Setting language_model_penalty_non_dict_word variable failed!!!\n");
	
	if (!m_tessBase.SetVariable("language_model_penalty_non_freq_dict_word", "1"))
		printf("Setting language_model_penalty_non_freq_dict_word variable failed!!!\n");

	return true;
}


/**
* \brief clear - This method clears tesseract data.
*/
void TesseractEngine::clear()
{
	m_tessBase.Clear();
}

/**
* \brief end - This method ends tesseract data.
*/
void TesseractEngine::end()
{
	m_tessBase.End();
}


/**
* \brief setPageSegMode - This method sets tesseract pege segmentation mode.
* \param [in] tesseract::PageSegMode segMode - segmentation mode.
*/
void TesseractEngine::setPageSegMode(tesseract::PageSegMode segMode)
{
	m_pageSegMode = segMode;
	m_tessBase.SetPageSegMode(segMode);
}


/**
* \brief createPix - This method creates tesseract image.
* \param [in] int width - image width.
* \param [in] int height - image height.
* \param [in] int depth - image depth.
* \param [in] int widthStep - image width step.
* \param [in] int * imageData - image data.
* \return *Pix - tesseract image.
*/
Pix* TesseractEngine::createPix(int width, int height, int depth, int widthStep, const int *imageData)
{
	Pix* pix = pixCreateNoInit(width, height, depth);
	pixSetWpl(pix, width);
	pixSetColormap(pix, NULL);

	int length = height*widthStep;
	l_uint32* datas = new l_uint32[length];

	// get a copy of the image data
	memcpy(datas, reinterpret_cast<const l_uint32*>(imageData), length);
	pixSetData(pix, datas);

	// correct the endianess
	pixEndianByteSwap(pix);

	return pix;
}


/**
* \brief setImage - This method wraps openCV image to tesseract image.
* \param [in] cv::Mat &image - openCV image.
*/
void TesseractEngine::setImage(const cv::Mat &image)
{
	if (image.data == NULL)
	{
		return;
	}

	int width = image.cols;
	int height = image.rows;

	switch (image.type())
	{
	case CV_8UC1:
	{
					cv::Mat fourChannelBlowUp;
					cv::cvtColor(image, fourChannelBlowUp, CV_GRAY2RGBA);

					m_pix = createPix(width, height,
						fourChannelBlowUp.elemSize() * 8,
						fourChannelBlowUp.step,
						reinterpret_cast<const int*>(fourChannelBlowUp.data));

					fourChannelBlowUp.release();
	}
		break;
	case CV_8UC3:
	{
					cv::Mat fourChannelBlowUp;
					cv::cvtColor(image, fourChannelBlowUp, CV_BGR2RGBA);

					m_pix = createPix(width, height,
						fourChannelBlowUp.elemSize() * 8,
						fourChannelBlowUp.step,
						reinterpret_cast<const int*>(fourChannelBlowUp.data));

					fourChannelBlowUp.release();
	}
		break;
	case CV_8UC4:
	{
					m_pix = createPix(width, height,
						image.elemSize() * 8,
						image.step,
						reinterpret_cast<const int*>(image.data));
	}
		break;
	default:
		break;
	}

	m_tessBase.SetImage(m_pix);
}


/**
* \brief setWhiteList - This method sets characters white list.
* \param [in] const char *whitelist - white list.
* \return bool - true if white list is setted, otherwise false.
*/
bool TesseractEngine::setWhiteList(const char *whitelist)
{
	return setVariable("tessedit_char_whitelist", whitelist);
}


/**
* \brief setVariable - This method sets tesseract variable.
* \param [in] std::string name - variable name.
* \param [in] std::string value - variable value.
* \return bool - true if variable is setted, otherwise false.
*/
bool TesseractEngine::setVariable(std::string name, std::string value)
{
	return m_tessBase.SetVariable(name.c_str(), value.c_str());
}


/**
* \brief processPage - This method runs tesseract OCR.
* \return STRING - OCR result.
*/
STRING TesseractEngine::processPage()
{
	STRING text_out;
	if (!m_tessBase.ProcessPage(m_pix, NULL, 0, NULL, 5000, &text_out))
	{
		printf("Error during processing.\n");
	}

	pixDestroy(&m_pix);

	return text_out;
}
