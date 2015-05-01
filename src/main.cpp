#include <cstdint>
#include <string>
#include <iostream>

#include "GeoTool.h"

int main(int argc, char *argv[])
{
    std::string input, output, method;
    uint32_t meth = NUM_METHODS;
    for (uint32_t i = 1; i < argc; ++i)
    {
        // input image
        if ((std::string(argv[i]) == "-i" || std::string(argv[i]) == "--input") && i + 1 < argc) {
            input = argv[++i];
        }

        if ((std::string(argv[i]) == "-o" || std::string(argv[i]) == "--output") && i + 1 < argc) {
            output = argv[++i];
        }

        if ((std::string(argv[i]) == "-m" || std::string(argv[i]) == "--method") && i + 1 < argc) {
            method = argv[++i];

            if (method == "slope")            
                meth = METHOD_SLOPE;            
            else
            if (method == "shaded_relief")            
                meth = METHOD_SHADED_RELIEF;           
        }
    }

    if (input.empty())
    {
        std::cerr << "No input file selected." << std::endl;
        return EXIT_FAILURE;
    }

    if (output.empty())
    {
        std::cerr << "No output file selected." << std::endl;
        return EXIT_FAILURE;
    }

    if (meth == NUM_METHODS)
    {
        std::cerr << "No method selected." << std::endl;
        return EXIT_FAILURE;
    }

    GeoTool gt;

    if (gt.load_file(input))
    {
        gt.set_output(output);

        cv::Mat res;
        switch (meth)
        {
            case METHOD_SLOPE:
                gt.slope();
                break;
            case METHOD_SHADED_RELIEF:
                gt.shaded_relief();
                break;
            default:
                break;                            
        }
        
        gt.free_file();
        
        return EXIT_SUCCESS;
    }
}

