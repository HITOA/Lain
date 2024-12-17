#include "theme.hpp"

#include <cmath>
#include <vector>


ThemeRGB ThemeMaker::Make(std::shared_ptr<Image> img, Lab* palette, uint32_t size, float themeLuminosity) {
    Theme<LCh> theme{};
    float isDarkTheme = themeLuminosity < 0.5 ? 1.0f : -1.1f;
    
    //Base color

    LCh startColor = ColorTo<LCh>(GetClosestColorByLuminosity(palette, size, themeLuminosity));
    LCh midColor = ColorTo<LCh>(GetClosestColorByLuminosity(palette, size, themeLuminosity + 0.05f * isDarkTheme));
    LCh endColor = ColorTo<LCh>(GetClosestColorByLuminosity(palette, size, themeLuminosity + 0.15f * isDarkTheme));
    startColor.L = themeLuminosity;
    midColor.L = themeLuminosity + 0.05f * isDarkTheme;
    endColor.L = themeLuminosity + 0.15f * isDarkTheme;

    theme.background = startColor;
    theme.foreground = LerpLCh(startColor, midColor, 0.5f);
    theme.surface[0] = midColor;
    theme.surface[1] = LerpLCh(midColor, endColor, 0.5f);
    theme.surface[2] = endColor;
    
    if (themeLuminosity < 0.5) {
        theme.background = theme.foreground;
        theme.foreground = startColor;
    }

    float textLuminosity = themeLuminosity < 0.5f ? 0.8f : 0.20f;
    float targetMaximumChroma = 0.8f;

    //Text & Subtext

    theme.text = ColorTo<LCh>(GetClosestColorByLuminosity(palette, size, textLuminosity));
    theme.text.L = textLuminosity;
    theme.text.C = theme.text.C > targetMaximumChroma ? targetMaximumChroma : theme.text.C; //Reduce chroma for main text color to be close to white/black for readibility
    theme.subtext[1] = ColorTo<LCh>(GetClosestColorByLuminosity(palette, size, textLuminosity - 0.07f * isDarkTheme));
    theme.subtext[1].L = textLuminosity - 0.07f * isDarkTheme;
    theme.subtext[1].C = theme.subtext[1].C > targetMaximumChroma ? targetMaximumChroma : theme.subtext[1].C;
    theme.subtext[0] = LerpLCh(theme.text, theme.subtext[1], 0.5f);

    //Accents color

    float accentLuminosity = themeLuminosity < 0.5f ? 0.8f : 0.7f;
    float primaryHUE = 0.0f;
    float averageChroma = 0.0f;
    float count = 0;
    float targetMinimumChroma = 0.04f;

    std::vector<LCh> accentCandidat{};

    for (uint32_t i = 0; i < size; ++i) {
        LCh current = ColorTo<LCh>(palette[i]);
        float currentLuminosityDiff = fabs(current.L - accentLuminosity);
        if (currentLuminosityDiff > 0.3f)
            continue;
        if (current.C < targetMinimumChroma)
            continue;
        primaryHUE += current.h;
        averageChroma += current.C;
        ++count;
    }

    primaryHUE /= count;
    averageChroma /= count;

    float currentHUETarget = primaryHUE;

    for (uint32_t j = 0; j < 8; ++j) {
        LCh currentBest{};
        float distance = 1000.0f;

        for (uint32_t i = 0; i < size; ++i) {
            LCh current = ColorTo<LCh>(palette[i]);
            float currentLuminosityDiff = fabs(current.L - accentLuminosity);
            float currentHUEDiff = fabs(current.h - currentHUETarget);
            if (currentLuminosityDiff > 0.5f)
                continue;
            if (currentHUEDiff > 0.3f)
                continue;
            if (current.C < targetMinimumChroma)
                continue;
            float currentDistance = (currentLuminosityDiff * currentLuminosityDiff) + (currentHUEDiff * currentHUEDiff);
            if (distance > currentDistance) {
                distance = currentDistance;
                currentBest = current;
            }
        }
        if (distance < 100.0f)
            accentCandidat.push_back(currentBest);
        else {
            accentCandidat.push_back(LCh{
                accentLuminosity,
                averageChroma,
                currentHUETarget
            });
        }

        currentHUETarget = fmod(currentHUETarget + M_PI * 2 / 8, 2 * M_PI);
    }

    if (accentCandidat.size() >= 1) {
        theme.primary = accentCandidat[0];
        theme.primary.C = averageChroma;
        theme.primary.L = accentLuminosity;
    }
    else
        theme.primary = LCh{ accentLuminosity, averageChroma, primaryHUE };

    for (uint32_t i = 1; i < accentCandidat.size(); ++i) {
        if (i >= 8)
            break;
        theme.accents[i - 1] = accentCandidat[i];
        //theme.accents[i - 1].L = accentLuminosity;
    }

    return ThemeTo<RGB>(theme);
}

Lab ThemeMaker::GetClosestColorByLuminosity(Lab* colors, uint32_t size, float luminosity) {
    Lab closestColor{};
    float luminosityDiff = 1000.0f;

    for (uint32_t i = 0; i < size; ++i) {
        float currentLuminosityDiff = fabs(colors[i].L - luminosity);
        if (luminosityDiff > currentLuminosityDiff) {
            luminosityDiff = currentLuminosityDiff;
            closestColor = colors[i];
        }
    }

    return closestColor;
}