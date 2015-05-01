#include <iostream>
#include "GeoTool.h"
#include <algorithm>

GeoTool::GeoTool() : _dataset(nullptr)
{    
    GDALAllRegister();    
}

bool GeoTool::load_file(std::string const& filename)
{
    _dataset = reinterpret_cast<GDALDataset*>(GDALOpen(filename.c_str(), GA_ReadOnly));          
    return _dataset != nullptr;
}

void GeoTool::print_info()
{   
    std::cout << "Width: " << _dataset->GetRasterXSize() << std::endl;
    std::cout << "Height: " << _dataset->GetRasterYSize() << std::endl;    
}

bool GeoTool::load_image(int32_t width, int32_t height, int32_t offset_x, int32_t offset_y)
{
    if ((width + offset_x > _dataset->GetRasterXSize()) || (height + offset_y > _dataset->GetRasterYSize()))
    {
        std::cerr << "Trying to load bigger raster than the actual size." << std::endl;
        return false;
    }

    if (_dataset->GetRasterCount() > 1)
    {
        std::cerr << "More than one raster." << std::endl;
        return false;
    }
    
    if (width == GT_ALL && height == GT_ALL && offset_x == 0 && offset_y == 0)
    {
        width = _dataset->GetRasterXSize();
        height = _dataset->GetRasterYSize();
    }         

    cv::Rect roi(offset_x, offset_y, width, height);
    _image.create(roi.size(), CV_32F);

    GDALRasterBand* band = _dataset->GetRasterBand(1);
    band->RasterIO(GF_Read, 0, 0, width, height, _image.data, width, height, GDT_Float32, 0, 0);

    _info.width = width;
    _info.height = height;   
    _info.offset_x = offset_x;
    _info.offset_y = offset_y;    

    return true;
}

void GeoTool::display_image(cv::Mat& image, uint32_t const& width, uint32_t const& height)
{    
    double min_value, max_value;
    cv::minMaxLoc(image, &min_value, &max_value);
    for (uint32_t x = 1; x < image.cols - 1; ++x)
    {
        for (uint32_t y = 1; y < image.rows - 1; ++y)        
            image.at<float>(y, x) /= max_value;        
    }

    cv::Mat resized;
    cv::resize(image, resized, cv::Size(width, height), 0.0, 0.0, CV_INTER_LANCZOS4);

    cv::imshow("Display", resized);
    cv::waitKey();    
}

void GeoTool::get3x3kernel(float* kernel, uint32_t const& x, uint32_t const& y)
{
    kernel[0] = _image.at<float>(y-1, x-1);
    kernel[1] = _image.at<float>(y-1, x);
    kernel[2] = _image.at<float>(y-1, x+1);

    kernel[3] = _image.at<float>(y, x-1);
    kernel[4] = _image.at<float>(y, x);
    kernel[5] = _image.at<float>(y, x+1);

    kernel[6] = _image.at<float>(y+1, x-1);
    kernel[7] = _image.at<float>(y+1, x);
    kernel[8] = _image.at<float>(y+1, x+1);
}

cv::Mat GeoTool::calc_slope()
{    
    cv::Mat slope(_image.rows, _image.cols, CV_32FC1);
    for (uint32_t x = 1; x < _image.cols-1; ++x)
    {
        for (uint32_t y = 1; y < _image.rows-1; ++y)
        {            
            float kernel[9];
            get3x3kernel(kernel, x, y);

            float s_ew = ((kernel[0] + kernel[3] + kernel[3] + kernel[6]) - (kernel[2] + kernel[5] + kernel[5] + kernel[8])) / 8.f;
            float s_ns = ((kernel[0] + kernel[1] + kernel[1] + kernel[2]) - (kernel[6] + kernel[7] + kernel[7] + kernel[8])) / 8.f;
            
            float tmp = sqrtf(s_ew * s_ew + s_ns * s_ns); 
            slope.at<float>(y, x) = (tmp != tmp) ? 0 : tmp;            
        }            
    }

    return slope;
}

cv::Mat GeoTool::calc_shaded_relief()
{
    float alt = 45;
    float az = 315;

    cv::Mat relief(_image.rows, _image.cols, CV_32FC1);
    for (uint32_t x = 1; x < _image.cols - 1; ++x)
    {
        for (uint32_t y = 1; y < _image.rows - 1; ++y)
        {
            float kernel[9];
            get3x3kernel(kernel, x, y);

            float s_ew = ((kernel[0] + kernel[3] + kernel[3] + kernel[6]) - (kernel[2] + kernel[5] + kernel[5] + kernel[8])) / 8.f;
            float s_ns = ((kernel[0] + kernel[1] + kernel[1] + kernel[2]) - (kernel[6] + kernel[7] + kernel[7] + kernel[8])) / 8.f;

            float slope = 90.0 - atan(sqrtf(s_ew * s_ew + s_ns * s_ns)) * RAD2DEG;
            float aspect = atan2(s_ew, s_ns);

            float shaded = sin(alt * DEG2RAD) * sin(slope * DEG2RAD) +
                cos(alt * DEG2RAD) * cos(slope * DEG2RAD) *
                cos((az - 90.0) * DEG2RAD - aspect);

            relief.at<float>(y, x) = shaded;
        }
    }

    return relief;
}

void GeoTool::free_file()
{
    GDALClose(_dataset);
}
