#ifndef GEOTOOL_H
#define GEOTOOL_H

#include <string>
#include <cstdint>

#include "gdal_priv.h"
#include "cpl_conv.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>

#define GT_NO_IMAGE -1
#define GT_ALL -1

const float RAD2DEG = 180.0 / 3.14159;
const float DEG2RAD = 3.14159 / 180.0;

enum Options
{
    OPT_SLOPE,
    OPT_SHADED_RELIEF,
    
    NUM_OPTIONS
};

struct Info
{
    double min_value, max_value;
    int32_t width = GT_NO_IMAGE;
    int32_t height = GT_NO_IMAGE;
    int32_t offset_x = GT_NO_IMAGE;
    int32_t offset_y = GT_NO_IMAGE;
};
    
class GeoTool
{
    public:
        GeoTool();

        bool load_file(std::string const& filename);
        void print_info();
        bool load_image(int32_t width = GT_ALL, int32_t height = GT_ALL, int32_t offset_x = 0, int32_t offset_y = 0);
        
        void display_image(cv::Mat& image, uint32_t const& width, uint32_t const& height);

        cv::Mat calc_slope();
        cv::Mat calc_shaded_relief();

        uint32_t get_num_bands() const { return _dataset->GetRasterCount(); } 
        Info getInfo() const { return _info; }       
               
                
        void free_file();

    protected:
        void get3x3kernel(float* kernel, uint32_t const& x, uint32_t const& y);

    private:             
        GDALDataset* _dataset;
        
        cv::Mat _image;
        Info _info;
};

#endif
