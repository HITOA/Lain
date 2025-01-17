#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>
#include <numeric>
#include <sys/stat.h>

#include "image.hpp"
#include "quantizer.hpp"
#include "theme.hpp"
#include "inja.hpp"

struct Options {
    const char* inputFile;
    enum class QuantizationAlgorithm {
        MedianCut,
        KMean
    } quantizer = QuantizationAlgorithm::MedianCut;
    uint32_t paletteSize = 32;
    float themeLuminosity = 0.20;
    bool print = true;
    std::vector<std::pair<std::string, std::string>> templates{};
    unsigned int seed = 0;
};

bool FileExists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

//Not very clean but will do
bool GetOptions(int argc, const char** argv, Options& options) {
    int idx = 1;
    if (argc <= idx) {
        std::cout << "Missing arguments. Use -h if you need to see the list of arguments available." << std::endl;
        return false;
    }
    
    while (idx < argc) {
        if (strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0) {
            std::cout << 
            "\n-h, --help: show this help message." 
            "\n-i, --input <image>: input image to be used to generate a color palette."
            "\n-q, --quantizer <median-cut/k-mean>: set wich quantizer to use. (Default is median cut)"
            "\n-t, --template <template file> <output file>: add a template to be rendered. this argument is repeatable." 
            "\n-s, --silent: make the app not print anything in the console except for error."
            "\n--size <size>: specify the size of the intermediate palette. (Default is 32)"
            "\n--dark: generate a dark theme. (Default)"
            "\n--light: generate a light theme."
            "\n--luminosity <value>: set theme overall luminosity between 0 and 100." 
            "\n--seed <value>: set quantizer seed (this has no effect with median cut)."<< std::endl;
            return false;
        }

        if (strcmp(argv[idx], "-i") == 0 || strcmp(argv[idx], "--input") == 0) {
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing filename for -i." << std::endl;
                return false;
            }
            options.inputFile = argv[idx];
            ++idx;
            continue;
        }

        if (strcmp(argv[idx], "-q") == 0 || strcmp(argv[idx], "--quantizer") == 0) {
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing quantizer for --quantizer." << std::endl;
                return false;
            }
            if (strcmp(argv[idx], "median-cut") == 0) {
                options.quantizer = Options::QuantizationAlgorithm::MedianCut;
            }
            else if (strcmp(argv[idx], "k-mean") == 0) {
                options.quantizer = Options::QuantizationAlgorithm::KMean;
            }
            else {
                std::cout << "Invalid input for --quantizer." << std::endl;
                return false;
            }
            ++idx;
            continue;
        }

        if (strcmp(argv[idx], "--size") == 0) {
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing size for -s." << std::endl;
                return false;
            }
            try {
                options.paletteSize = std::stoi(argv[idx]);
            } catch (std::exception& e) {
                std::cout << "Invalid size for -s" << std::endl;
                return false;
            }
            ++idx;
            continue;
        }

        if (strcmp(argv[idx], "-t") == 0 || strcmp(argv[idx], "--templates") == 0) {
            ++idx;
            std::string input;
            std::string output;
            if (idx >= argc) {
                std::cout << "Missing template file for -t." << std::endl;
                return false;
            }
            if (!FileExists(argv[idx])) {
                std::cout << "Template file doesn't exists!" << std::endl;
                return false;
            }
            input = argv[idx];
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing output file for -t." << std::endl;
                return false;
            }
            output = argv[idx];
            ++idx;
            options.templates.push_back(std::make_pair(input, output));
            continue;
        }

        if (strcmp(argv[idx], "--dark") == 0) {
            ++idx;
            options.themeLuminosity = 0.20f;
            continue;
        }

        if (strcmp(argv[idx], "--light") == 0) {
            ++idx;
            options.themeLuminosity = 0.99f;
            continue;
        }

        if (strcmp(argv[idx], "--luminosity") == 0) {
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing value for --luminosity." << std::endl;
                return false;
            }
            try {
                int l = std::stoi(argv[idx]);
                options.themeLuminosity = (float)std::clamp(l, 0, 100) / 100.0f;
            } catch (std::exception& e) {
                std::cout << "Invalid value for --luminosity" << std::endl;
                return false;
            }
            ++idx;
            continue;
        }

        if (strcmp(argv[idx], "--seed") == 0) {
            ++idx;
            if (idx >= argc) {
                std::cout << "Missing value for --seed." << std::endl;
                return false;
            }
            try {
                options.seed = std::stoi(argv[idx]);
            } catch (std::exception& e) {
                std::cout << "Invalid value for --seed" << std::endl;
                return false;
            }
            ++idx;
            continue;
        }

        if (strcmp(argv[idx], "-s") == 0 || strcmp(argv[idx], "--silent") == 0) {
            ++idx;
            options.print = false;
            continue;
        }
        
        std::cout << "Wrong argument \"" << argv[idx] << "\". This argument does not exist." << std::endl;
        return false;
    }
    
    return true;
}

void PrintTheme(ThemeRGB& theme) {
    RGB* colors = (RGB*)&theme.background;
    for (uint32_t i = 0; i < 16; ++i) {
        if (i % 8 == 0 && i != 0)
            std::cout << "\n";
        PrintRGB(colors[i]);
    }
    std::cout << std::endl;
}

void AddColorData(inja::json& data, RGB color) {
    data["hex"] = RGB2HexString(color);
    data["rgb"] = RGB2String(color);
    data["L"] = ColorTo<LCh>(color).L;
    data["C"] = ColorTo<LCh>(color).C;
    data["hue"] = ColorTo<LCh>(color).h * (180.0f / M_PI);
}

void AddNamedColorData(inja::json& data, const std::string& name, RGB color) {
    AddColorData(data[name], color);
}

int main(int argc, const char** argv) {
    Options options{};
    if (!GetOptions(argc, argv, options))
        return -1;
    
    if (!options.inputFile) {
        std::cout << "Input missing. use -i <image> to specify an input image." << std::endl;
        return -1;
    }

    std::shared_ptr<Image> img = Image::Open(options.inputFile);

    if (!img->GetData()) {
        std::cout << "Failed to open \"" << options.inputFile << "\"." << std::endl;
        return -1;
    }

    std::shared_ptr<Quantizer> quantizer = nullptr;

    switch (options.quantizer) {
        case Options::QuantizationAlgorithm::MedianCut:
            quantizer = std::make_shared<MedianCut>();
            break;
        case Options::QuantizationAlgorithm::KMean:
            {
                auto q = std::make_shared<KMean>();
                q->SetSeed(options.seed);
                quantizer = q;
            }
            break;
        default:
            quantizer = std::make_shared<MedianCut>();
            break;
    }

    std::vector<Lab> palette( options.paletteSize );
    quantizer->Quantize(img, palette.data(), palette.size());

    ThemeMaker maker{};
    ThemeRGB theme = maker.Make(img, palette.data(), palette.size(), options.themeLuminosity);

    if (options.print)
        PrintTheme(theme);

    inja::json data;

    AddNamedColorData(data, "background",    theme.background);
    AddNamedColorData(data, "foreground",    theme.foreground);
    AddNamedColorData(data, "surface0",      theme.surface[0]);
    AddNamedColorData(data, "surface1",      theme.surface[1]);
    AddNamedColorData(data, "surface2",      theme.surface[2]);
    AddNamedColorData(data, "surface3",      theme.surface[3]);
    AddNamedColorData(data, "text",          theme.text);
    AddNamedColorData(data, "subtext",       theme.subtext);
    AddNamedColorData(data, "primary",       theme.primary);
    AddNamedColorData(data, "accent0",       theme.accents[0]);
    AddNamedColorData(data, "accent1",       theme.accents[1]);
    AddNamedColorData(data, "accent2",       theme.accents[2]);
    AddNamedColorData(data, "accent3",       theme.accents[3]);
    AddNamedColorData(data, "accent4",       theme.accents[4]);
    AddNamedColorData(data, "accent5",       theme.accents[5]);
    AddNamedColorData(data, "accent6",       theme.accents[6]);

    data["averageAccentLuminosity"] = theme.averageAccentLuminosity;
    data["averageAccentChroma"] = theme.averageAccentChroma;

    inja::Environment env;

    env.add_callback("closestHUE", 1, [&](inja::Arguments& args) {
        int hueDeg = args.at(0)->get<int>();
        float hueRad = (float)hueDeg * (M_PI / 180.0f);
        float a = cosf(hueRad);
        float b = sinf(hueRad);
        float smallestD = 10000.0f;
        RGB currentPickedColor{};

        RGB* colors = (RGB*)&theme.primary;
        for (uint32_t i = 0; i < 8; ++i) {
            Lab current = ColorTo<Lab>(colors[i]);
            float m = sqrtf(current.a * current.a + current.b * current.b);
            float currentNormalizedA = current.a / m;
            float currentNormalizedB = current.b / m;
            float d = sqrtf((currentNormalizedA - a) * (currentNormalizedA - a) + (currentNormalizedB - b) * (currentNormalizedB - b));
            if (d < smallestD) {
                smallestD = d;
                currentPickedColor = colors[i];
            }
        }

        inja::json currentPickedColorData;
        AddColorData(currentPickedColorData, currentPickedColor);

        return currentPickedColorData;
    });

    env.add_callback("makeColorLCh", 3, [&](inja::Arguments& args) {
        float L = args.at(0)->get<float>();
        float C = args.at(1)->get<float>();
        int hueDeg = args.at(2)->get<int>();
        float hueRad = (float)hueDeg * (M_PI / 180.0f);
        
        RGB color = ColorTo<RGB>(LCh{
            L,
            C,
            hueRad
        });

        inja::json colorData;
        AddColorData(colorData, color);

        return colorData;
    });

    for (auto& t : options.templates) {
        env.write(t.first, data, t.second);
    }
    
    return 0;
}