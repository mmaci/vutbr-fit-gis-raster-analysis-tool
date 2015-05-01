#include "GeoTool.h"

GeoTool::GeoTool()
{    
    GDALAllRegister();    
}

bool GeoTool::load_file(std::string const& filename)
{
    _indataset = reinterpret_cast<GDALDataset*>(GDALOpen(filename.c_str(), GA_ReadOnly));          
    if (_indataset == nullptr)
        return false;

    _inband = _indataset->GetRasterBand(1);
    _indataset->GetGeoTransform(_info.geo_transform);
    _info.width = _indataset->GetRasterXSize();
    _info.height = _indataset->GetRasterYSize();
    _info.input_filename = filename;
    _info.null_value = static_cast<float>(_inband->GetNoDataValue());

    return true;
}


void GeoTool::set_output(std::string const& filename)
{
    GDALDriver* driver;
    driver = GetGDALDriverManager()->GetDriverByName(FORMAT.c_str());

    _outdataset = driver->Create(filename.c_str(), _info.width, _info.height, 1, GDT_Byte, NULL);
    _outdataset->SetGeoTransform(_info.geo_transform);
    _outdataset->SetProjection(_indataset->GetProjectionRef());

    _outband = _outdataset->GetRasterBand(1);
    _outband->SetNoDataValue(0.0);

}

bool GeoTool::get3x3kernel(float* kernel, uint32_t const& x, uint32_t const& y)
{
    _inband->RasterIO(GF_Read, x - 1, y - 1, 3, 3, kernel, 3, 3, GDT_Float32, 0, 0);

    for (int i = 0; i < 9; ++i)
    {
        if (kernel[i] == _info.null_value)
            return false;
    }

    return true;
}

void GeoTool::slope()
{        
    float* linebuffer = reinterpret_cast<float*>(CPLMalloc(sizeof(float) * _info.width));
    
    for (uint32_t i = 0; i < _info.width; ++i)
        linebuffer[i] = 0.f;

    // first and last line
    _outband->RasterIO(GF_Write, 0, 0, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);
    _outband->RasterIO(GF_Write, 0, _info.height-1, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);
    
    for (uint32_t y = 1; y < _info.height - 1; ++y)
    {
        linebuffer[0] = 0.f;
        linebuffer[_info.width - 1] = 0.0;

        for (uint32_t x = 1; x < _info.width - 1; ++x)
        {                                
            float kernel[9];            
            if (get3x3kernel(kernel, x, y))
            {
                float s_ew = ((kernel[0] + kernel[3] + kernel[3] + kernel[6]) - (kernel[2] + kernel[5] + kernel[5] + kernel[8])) / (8.f * _info.EW_RES);
                float s_ns = ((kernel[0] + kernel[1] + kernel[1] + kernel[2]) - (kernel[6] + kernel[7] + kernel[7] + kernel[8])) / (8.f * _info.NS_RES);
                float slope = sqrtf(s_ew * s_ew + s_ns * s_ns);
                linebuffer[x] = slope * 255.f;
            }
            else {
                linebuffer[x] = 0.f;
            }                          
        }    

        _outband->RasterIO(GF_Write, 0, y, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);
    }    
}

void GeoTool::shaded_relief(float altitude, float azimuth)
{
    float* linebuffer = reinterpret_cast<float*>(CPLMalloc(sizeof(float) * _info.width));

    for (uint32_t i = 0; i < _info.width; ++i)
        linebuffer[i] = 0.f;

    // first and last line
    _outband->RasterIO(GF_Write, 0, 0, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);
    _outband->RasterIO(GF_Write, 0, _info.height - 1, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);

    for (uint32_t y = 1; y < _info.height - 1; ++y)
    {
        linebuffer[0] = 0.f;
        linebuffer[_info.width - 1] = 0.0;

        for (uint32_t x = 1; x < _info.width - 1; ++x)
        {
            float kernel[9];
            if (get3x3kernel(kernel, x, y))
            {
                float s_ew = ((kernel[0] + kernel[3] + kernel[3] + kernel[6]) - (kernel[2] + kernel[5] + kernel[5] + kernel[8])) / (8.f * _info.EW_RES);
                float s_ns = ((kernel[0] + kernel[1] + kernel[1] + kernel[2]) - (kernel[6] + kernel[7] + kernel[7] + kernel[8])) / (8.f * _info.NS_RES);
                
                float slope = 90.0 - atan(sqrtf(s_ew * s_ew + s_ns * s_ns)) * RAD2DEG;
                float aspect = atan2(s_ew, s_ns);

                linebuffer[x] = (sin(altitude * DEG2RAD) * sin(slope * DEG2RAD) +
                    cos(altitude * DEG2RAD) * cos(slope * DEG2RAD) *
                    cos((azimuth - 90.0) * DEG2RAD - aspect)) * 255.f;                 
            }
            else {
                linebuffer[x] = 0.f;
            }            
        }

        _outband->RasterIO(GF_Write, 0, y, _info.width, 1, linebuffer, _info.width, 1, GDT_Float32, 0, 0);
    }
}

void GeoTool::free_file()
{
    GDALClose(_indataset);
}
