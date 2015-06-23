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

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#include "textDetector.h"


using namespace cv;
using namespace std;


string INPUT_FOLDER_PATH;
string OUTPUT_FOLDER_PATH;

/**
* \brief ModulePathA - This method finds directory where app executable is located.
* \return std::string directory path.
*/
std::string ModulePathA()
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
* \brief compareNat - Method which compares two file path and sort them in "natural ordering".
* \param [in] boost::filesystem::path& a1 - First input file path.
* \param [in] boost::filesystem::path& b1 - Second input file path.
* \return Returns true, if a1 is before b1, otherwise false.
*/
bool compareNat(const boost::filesystem::path& a1, const boost::filesystem::path& b1)
{
	std::string a = a1.string();
	std::string b = b1.string();
	if (a.empty())
		return true;
	if (b.empty())
		return false;
	if (std::isdigit(a[0]) && !std::isdigit(b[0]))
		return true;
	if (!std::isdigit(a[0]) && std::isdigit(b[0]))
		return false;
	if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
	{
		if (a[0] == b[0])
			return compareNat(a.substr(1), b.substr(1));
		return (std::toupper(a[0]) < std::toupper(b[0]));
	}

	// Both strings begin with digit --> parse both numbers
	std::istringstream issa(a);
	std::istringstream issb(b);
	int ia, ib;
	issa >> ia;
	issb >> ib;
	if (ia != ib)
		return ia < ib;

	// Numbers are the same --> remove numbers and recurse
	std::string anew, bnew;
	std::getline(issa, anew);
	std::getline(issb, bnew);
	return (compareNat(anew, bnew));
}


/**
* \brief listFiles - Method which lists files from folder.
* \param [in] std::string dirPath - Path to the folder with images.
* \return std::vector<boost::filesystem::path> - Vector of all image's paths in folder (with .jpg, .png and .bmp extensions).
*/
std::vector<boost::filesystem::path> listFiles(std::string dirPath)
{
	namespace fs = boost::filesystem;
	fs::path someDir(dirPath);
	fs::directory_iterator end_iter;

	std::vector<std::string> targetExtensions;
	targetExtensions.push_back(".JPG");
	targetExtensions.push_back(".JPEG");
	targetExtensions.push_back(".PNG");
	targetExtensions.push_back(".BMP");

	std::vector<fs::path> result_set;

	if (fs::exists(someDir) && fs::is_directory(someDir))
	{
		for (fs::directory_iterator dir_iter(someDir); dir_iter != end_iter; ++dir_iter)
		{
			if (fs::is_regular_file(dir_iter->status()))
			{
				std::string extension = dir_iter->path().extension().string();
				std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
				std::vector<std::string>::iterator it;
				for (it = targetExtensions.begin(); it != targetExtensions.end(); ++it)
				{
					if (extension.compare(*it) == 0)
					{
						result_set.push_back(dir_iter->path());
					}
				}
			}
		}
	}

	std::sort(result_set.begin(), result_set.end(), compareNat);

	return result_set;
}


/**
* \brief Method for writing execution time to a log.
* \param [in] boost::posix_time::ptime start Time when certain method is started.
* \param [in] boost::posix_time::ptime end Time when certain method is finished.
* \param [in] std::string sExecutionLocation Information which method is measured.
* \param [in] std::string description - additional description.
*/
void  log_execution_time(boost::posix_time::ptime start, boost::posix_time::ptime end, std::string sExecutionLocation, std::string description)
{
	boost::posix_time::time_duration duration = end - start;
	std::ofstream logExecution;
	logExecution.open(OUTPUT_FOLDER_PATH + "//ExecutionTime.csv", std::ios::app);
	logExecution << to_iso_string(boost::posix_time::microsec_clock::local_time()) << ";" << sExecutionLocation << ";" << duration.total_milliseconds() << ";ms;" << description << std::endl;
	logExecution.close();
}



int main(int argc, char *argv[])
{
	if (argc == 3)
	{
		INPUT_FOLDER_PATH = argv[1];
		OUTPUT_FOLDER_PATH = argv[2];
	}
	else
	{
		cout << "Not Enough Args..." << endl;
		system("pause");
		return -1;
	}

	boost::posix_time::ptime start;
	boost::posix_time::ptime end;

	protech::TextDetector textDetextor;
	textDetextor.initialize(OUTPUT_FOLDER_PATH);
	
	vector<boost::filesystem::path> m_ImagesFromFolder;
	m_ImagesFromFolder = listFiles(INPUT_FOLDER_PATH);

	int i = 0;
	while ((i < m_ImagesFromFolder.size()))
	{
		try
		{
			Mat large;
			start = boost::posix_time::microsec_clock::local_time();
			large = imread(m_ImagesFromFolder[i].string());
			end = boost::posix_time::microsec_clock::local_time();
			log_execution_time(start, end, "imread", "");

			if (large.data == NULL)
			{
				system("pause");
				return 1;
			}

			std::string img_name(m_ImagesFromFolder[i].string(), m_ImagesFromFolder[i].string().find_last_of('\\') + 1);
			img_name = img_name.substr(0, img_name.size() - 4);

			start = boost::posix_time::microsec_clock::local_time();
			textDetextor.textDetectionFunction(large, img_name);
			end = boost::posix_time::microsec_clock::local_time();
			log_execution_time(start, end, "contoursFunction", "");

			large.release();
		}
		catch (std::exception & ex)
		{
			cout << "Exception: " << ex.what() << endl;
		}
		catch (char * ex)
		{
			cout << "Exception: " << ex << endl;
		}
		catch (...)
		{
			;
		}
		i++;
	}

	system("pause");
	return 0;
}
