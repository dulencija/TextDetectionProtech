
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
	//All languages list
	//ara (Arabic), aze (Azerbauijani), bul (Bulgarian), cat (Catalan), ces (Czech), chi_sim (Simplified Chinese), chi_tra (Traditional Chinese), chr (Cherokee), dan (Danish), dan-frak (Danish (Fraktur)), deu (German), ell (Greek), eng (English), enm (Old English), epo (Esperanto), est (Estonian), fin (Finnish), fra (French), frm (Old French), glg (Galician), heb (Hebrew), hin (Hindi), hrv (Croation), hun (Hungarian), ind (Indonesian), ita (Italian), jpn (Japanese), kor (Korean), lav (Latvian), lit (Lithuanian), nld (Dutch), nor (Norwegian), pol (Polish), por (Portuguese), ron (Romanian), rus (Russian), slk (Slovakian), slv (Slovenian), sqi (Albanian), spa (Spanish), srp (Serbian), swe (Swedish), tam (Tamil), tel (Telugu), tgl (Tagalog), tha (Thai), tur (Turkish), ukr (Ukrainian), vie (Vietnamese)

	//eng(English)
	//chi_sim(Simplified Chinese), chi_tra(Traditional Chinese)
	//ind(Indonesian)
	//hin(Hindi)
	//jpn(Japanese)
	//kor(Korean)
	//tha(Thai)

	m_lang = "eng";
	setPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	m_mode = tesseract::OEM_TESSERACT_ONLY;
	bool suc = setVariable("load_system_dawg", "0");
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

	if (m_datas != NULL)
	{
		delete[] m_datas;
		m_datas = NULL;
	}
	m_datas = new l_uint32[length];

	// get a copy of the image data
	memcpy(m_datas, reinterpret_cast<const l_uint32*>(imageData), length);
	pixSetData(pix, m_datas);

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
	if (m_pix != 0)
	{
		m_tessBase.SetImage(m_pix);
		//std::cout << "Slika podesena! " << std::endl;
	}
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
	if (!m_tessBase.ProcessPage(m_pix, NULL, 0, NULL, 3000, &text_out))
	{
		;//printf("Error during processing.\n");
	}

	//pixDestroy(&m_pix);

	return text_out;
}

void TesseractEngine::getWordsCount(int & WCount, int & LCount)
{
	Boxa* bounds = m_tessBase.GetWords(NULL);
	WCount = (int)bounds->n;
	
	Boxa* boundsLines = m_tessBase.GetTextlines(NULL, NULL);
	LCount = (int)boundsLines->n;
}
