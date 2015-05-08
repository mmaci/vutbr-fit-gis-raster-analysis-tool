/** @file main.cpp
 * @brief Main application file.
 *
 * An application for geographical raster analysis.
 *
 * @author Pavel Macenauer <macenauer.p@gmail.com>
 */

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "GeoTool.h"

int main(int argc, char *argv[])
{
    std::string input, output, method, coordsfile;
    std::vector<Point> coordslist;
    uint32_t meth = NUM_METHODS;
    float start_x = -1.f, start_y = -1.f;
    for (int32_t i = 1; i < argc; ++i)
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
            else
            if (method == "drain")
                meth = METHOD_DRAIN;
        }

        if ((std::string(argv[i]) == "-x") && i + 1 < argc) {
            start_x = std::stof(argv[++i]);
        }

        if ((std::string(argv[i]) == "-y") && i + 1 < argc) {
            start_y = std::stof(argv[++i]);
        }

        if ((std::string(argv[i]) == "-c" || std::string(argv[i]) == "--coords") && i + 1 < argc) {
            coordsfile = argv[++i];
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

    if (meth == METHOD_DRAIN && (start_x == -1 || start_y == -1) && coordsfile.empty())
    {
        std::cerr << "Drain must have starting X and Y coordinates." << std::endl;
        return EXIT_FAILURE;
    }

    GeoTool gt;

    if (gt.load_file(input))
    {
        gt.set_output(output);

        switch (meth)
        {
            case METHOD_SLOPE:
            {
                gt.slope();
                break;
            }
            case METHOD_SHADED_RELIEF:
            {
                gt.shaded_relief();
                break;
            }
            case METHOD_DRAIN:
            {
                std::string buffer;
                if (!coordsfile.empty())
                {
                    std::ifstream file;
                    file.open(coordsfile);
                    if (file.is_open())
                    {
                        while (std::getline(file, buffer))
                        {
                            uint8_t i = 0;
                            std::stringstream ss(buffer);                            
                            std::string item;
                            float point[2];

                            while (std::getline(ss, item, ','))                            
                                point[i++] = std::stof(item);  
                            
                            coordslist.push_back(Point(point[0], point[1]));
                        }
                        file.close();
                    }                    
                    gt.drain(coordslist);
                    
                }
                else
                if (start_x != -1.f && start_y != -1.f)
                    gt.drain(start_x, start_y);
                break;
            }
            default:
                break;                            
        }
        
        gt.free_file();
        
        return EXIT_SUCCESS;
    }
}

