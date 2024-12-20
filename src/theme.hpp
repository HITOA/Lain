#pragma once

#include "color.hpp"
#include "image.hpp"
#include "quantizer.hpp"


template<typename ColorT = RGB>
struct Theme {
    ColorT background; //Background foreground and surfaces should share a similare tint and temperature and should be close together in brightness
    ColorT foreground;
    ColorT surface[4];
    ColorT text; //Almost alwayse ~black or ~white to contrast with background/foreground and surfaces
    ColorT subtext; //Less bright than text
    ColorT primary; //The first and main accent color
    ColorT accents[7]; //This should cover a big part of the HUE wheel while still respecting the image tint and temperature
};

typedef Theme<RGB> ThemeRGB;

template<typename T, typename U>
inline Theme<T> ThemeTo(Theme<U> theme) {
    Theme<T> newTheme{};
    newTheme.background = ColorTo<T>(theme.background);
    newTheme.foreground = ColorTo<T>(theme.foreground);
    newTheme.surface[0] = ColorTo<T>(theme.surface[0]);
    newTheme.surface[1] = ColorTo<T>(theme.surface[1]);
    newTheme.surface[2] = ColorTo<T>(theme.surface[2]);
    newTheme.surface[3] = ColorTo<T>(theme.surface[3]);
    newTheme.text       = ColorTo<T>(theme.text);
    newTheme.subtext = ColorTo<T>(theme.subtext);
    newTheme.primary    = ColorTo<T>(theme.primary);
    newTheme.accents[0] = ColorTo<T>(theme.accents[0]);
    newTheme.accents[1] = ColorTo<T>(theme.accents[1]);
    newTheme.accents[2] = ColorTo<T>(theme.accents[2]);
    newTheme.accents[3] = ColorTo<T>(theme.accents[3]);
    newTheme.accents[4] = ColorTo<T>(theme.accents[4]);
    newTheme.accents[5] = ColorTo<T>(theme.accents[5]);
    newTheme.accents[6] = ColorTo<T>(theme.accents[6]);
    return newTheme;
}

class ThemeMaker {
public:
    ThemeRGB Make(std::shared_ptr<Image> img, Lab* palette, uint32_t size, float themeLuminosity);

private:
    Lab GetClosestColorByLuminosity(Lab* colors, uint32_t size, float luminosity);
};