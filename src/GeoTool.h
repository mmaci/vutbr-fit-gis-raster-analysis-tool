/** @file GeoTool.h
 * @brief Geographical tool. 
 *
 * A set of tools for geographical raster analysis.
 *
 * @author Pavel Macenauer <macenauer.p@gmail.com>
 */

#ifndef GEOTOOL_H
#define GEOTOOL_H

#include <string>
#include <cstdint>
#include <cmath>
#include "gdal_priv.h"
#include "cpl_conv.h"

/** @brief No image set. */
#define GT_NO_IMAGE -1

/** @brief Conversion from radians to degrees. */
const float GT_RAD2DEG = 180.f / static_cast<float>(M_PI);
/** @brief Conversion from degrees to radians. */
const float GT_DEG2RAD = static_cast<float>(M_PI) / 180.f;

/** @brief Raster analysis methods. */
enum Methods
{
    /** @brief Slope from DEM. */
    METHOD_SLOPE,
    /** @brief Shaded relief from DEM. */
    METHOD_SHADED_RELIEF,
    /** @brief Drainage from DEM. */
    METHOD_DRAIN,
    
    /** @brief Total number of methods. */
    NUM_METHODS
};

struct Info
{
    double min_value;
    double max_value;

    std::string input_filename;

    int32_t width = GT_NO_IMAGE;
    int32_t height = GT_NO_IMAGE;
    int32_t offset_x = GT_NO_IMAGE;
    int32_t offset_y = GT_NO_IMAGE;

    float null_value;

    double geo_transform[6];
};

/** @brief North-South resolution. */
#define NS_RES geo_transform[5]
/** @brief East-West resolution. */
#define EW_RES geo_transform[1]

const std::string FORMAT = "GTiff";
    
/** @brief X,Y point structure. */
struct Point
{
    /** @brief A constructor. */
    Point() : cost(0.f) { };

    /** @brief A constructor
     *
     * @param ix Initial X-coordinate.
     * @param iy Initial Y-coordinate.
     */
    Point(float ix, float iy) : x(ix), y(iy), cost(0.f) { };

    /** @brief X-coordinate. */
    float x;
    /** @brief Y-coordinate */
    float y;
    /** @brief Cost, which might be used for calculating paths, drains, etc. */
    float cost;

    bool operator< (Point const& right) const
    { return cost < right.cost; }
};

/** @brief Handles geographical raster analysis. */
class GeoTool
{
    public:
        /** @brief A constructor. */
        GeoTool();

        /** @brief Loads file into memory. 
         * 
         * @param filename Filename.
         * @return Successfully loaded file.
         */
        bool load_file(std::string const& filename);                

        /** @brief Sets file as output. 
         *
         * @param filename Filename.
         * @return Void.
         */
        void set_output(std::string const& filename);                

        /** @brief Calculates slopes.
         * The steepest slopes have values close to 1.0 (white).
         */
        void slope();

        /** @brief Calculates a shaded relief of the landscape. 
         *
         * @param altitude
         * @param azimuth
         * @return Void.
         */
        void shaded_relief(float const& altitude = 45.f, float const& azimuth = 315.f);                               

        /** @brief Calculates drainage from a staring point. 
         * 
         * @param start_x Starting X-coordinate.
         * @param start_y Starting Y-coordinate.
         * @return Void.
         */
        void drain(float const& start_x, float const& start_y);

        /** @brief Calculates drainage from a staring point.
         *
         * @param start Starting X,Y coordinates.
         * @return Void.
         */
        void drain(Point const& start);

        /** @brief Calculates drainage from multiple starting points. 
         *
         * @param points A list of starting points.
         * @return Void.
         */
        void drain(std::vector<Point> const& points);
             
        /** @brief Frees memory. 
         *
         * @return Void. 
         */
        void free_file();

    protected:
        /** @brief Fills a 3x3 area around X,Y coordinates. 
         *
         * @param kernel 3x3 area.
         * @param y X-coordinate.
         * @param x Y-coordinate.
         * @return The kernel is OK = without any null values.
         */
        bool get3x3kernel(float* kernel, uint32_t const& x, uint32_t const& y);        

    private:             
        /** @brief Input dataset. */
        GDALDataset* _indataset;

        /** @brief Output dataset. */
        GDALDataset* _outdataset;

        /** @brief Input band (band is a dataset layer). */
        GDALRasterBand* _inband;

        /** @brief Output band (band is a dataset layer). */
        GDALRasterBand* _outband;
                
        /** @brief Info structure. */
        Info _info;
};

#endif
