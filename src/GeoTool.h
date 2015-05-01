#ifndef GEOTOOL_H
#define GEOTOOL_H

#include <string>
#include <cstdint>

#include "gdal_priv.h"
#include "cpl_conv.h"

#define GT_NO_IMAGE -1
#define GT_ALL -1

const float RAD2DEG = 180.0 / 3.14159;
const float DEG2RAD = 3.14159 / 180.0;

enum Methods
{
    METHOD_SLOPE,
    METHOD_SHADED_RELIEF,
    
    NUM_METHODS
};

struct Info
{
    double min_value, max_value;

    std::string input_filename;

    int32_t width = GT_NO_IMAGE;
    int32_t height = GT_NO_IMAGE;
    int32_t offset_x = GT_NO_IMAGE;
    int32_t offset_y = GT_NO_IMAGE;

    float null_value;

    double geo_transform[6];
};

// north-south resolution
#define NS_RES geo_transform[5]
// east-west resolution
#define EW_RES geo_transform[1]

const std::string FORMAT = "GTiff";
    
class GeoTool
{
    public:
        GeoTool();

        bool load_file(std::string const& filename);                
        void set_output(std::string const& filename);                

        void slope();
        void shaded_relief(float altitude = 45.f, float azimuth = 315.f);                               
                
        void free_file();

    protected:
        bool get3x3kernel(float* kernel, uint32_t const& x, uint32_t const& y);        

    private:             
        GDALDataset* _indataset;
        GDALDataset* _outdataset;
        GDALRasterBand* _inband;
        GDALRasterBand* _outband;
                
        Info _info;
};

#endif
