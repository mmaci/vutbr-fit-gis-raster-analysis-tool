#include <cstdint>
#include <string>
#include <iostream>

#include "GeoTool.h"

int main(int argc, char *argv[])
{
    std::string input, option;
    uint32_t opt;
    for (uint32_t i = 1; i < argc; ++i)
    {
        // input image
        if ((std::string(argv[i]) == "-i" || std::string(argv[i]) == "--input") && i + 1 < argc) {
            input = argv[++i];
        }

        if ((std::string(argv[i]) == "-o" || std::string(argv[i]) == "--option") && i + 1 < argc) {
            option = argv[++i];

            if (option == "slope")            
                opt = OPT_SLOPE;            
            else
            if (option == "shaded_relief")            
                opt = OPT_SHADED_RELIEF;           
        }
    }

    GeoTool gt;

    if (gt.load_file(input))
    {
        gt.load_image();
        cv::Mat res;
        switch (opt)
        {
            case OPT_SLOPE:
                res = gt.calc_slope();
                break;
            case OPT_SHADED_RELIEF:
                res = gt.calc_shaded_relief();
                break;
            default:
            {
                std::cerr << "No option selected." << std::endl;
                return EXIT_FAILURE;
            }
                            
        }

        gt.display_image(res, 1024, 1024 / (res.cols / res.rows));        
        gt.free_file();

        

        return EXIT_SUCCESS;
    }
}

