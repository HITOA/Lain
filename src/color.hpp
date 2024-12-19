#pragma once

#include <cstdio>
#include <cmath>
#include <iostream>


struct RGB {
    float r;
    float g;
    float b;
};

struct Lab {
    float L;
    float a;
    float b;
};

struct LCh {
    float L;
    float C;
    float h;
};

inline void PrintRGB(RGB color) {
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", (int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f));
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm \033[0m", (int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f));
}

inline std::string RGB2HexString(RGB color) {
    unsigned int r = color.r * 255.0f;
    unsigned int g = color.g * 255.0f;
    unsigned int b = color.b * 255.0f;
    static char result[256];

    snprintf(result, 256, "#%02X%02X%02X", r, g, b);
    return result;
}

inline std::string RGB2String(RGB color) {
    unsigned int r = color.r * 255.0f;
    unsigned int g = color.g * 255.0f;
    unsigned int b = color.b * 255.0f;
    static char result[256];

    snprintf(result, 256, "%u, %u, %u", r, g, b);
    return result;
}

//https://bottosson.github.io/posts/colorwrong
inline float LinearToGamma(float x)
{
    if (x >= 0.0031308f)
        return (1.055f) * powf(x, (1.0f/2.4f)) - 0.055f;
    else
        return 12.92f * x;
}

//https://bottosson.github.io/posts/colorwrong
inline float GammaToLinear(float x)
{
    if (x >= 0.04045f)
        return powf((x + 0.055f)/(1.0f + 0.055f), 2.4f);
    else 
        return x / 12.92f;
}

template<typename T, typename U>
inline T ColorTo(U color) {
    return T{};
}

//https://bottosson.github.io/posts/oklab/
template<>
inline Lab ColorTo<Lab, RGB>(RGB color) {
    float r = GammaToLinear(color.r);
    float g = GammaToLinear(color.g);
    float b = GammaToLinear(color.b);

    float l = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
	float m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
	float s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    return {
        0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
        1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
        0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
    };
}

//https://bottosson.github.io/posts/oklab/
template<>
inline RGB ColorTo<RGB, Lab>(Lab color) {
    float l_ = color.L + 0.3963377774f * color.a + 0.2158037573f * color.b;
    float m_ = color.L - 0.1055613458f * color.a - 0.0638541728f * color.b;
    float s_ = color.L - 0.0894841775f * color.a - 1.2914855480f * color.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

	float r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
	float g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
	float b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;

    r = LinearToGamma(r);
    g = LinearToGamma(g);
    b = LinearToGamma(b);

    r = r > 1.0f ? 1.0f : r < 0.0f ? 0.0f : r;
    g = g > 1.0f ? 1.0f : g < 0.0f ? 0.0f : g;
    b = b > 1.0f ? 1.0f : b < 0.0f ? 0.0f : b;

    return RGB{ r, g, b };
}

template<>
inline LCh ColorTo<LCh, Lab>(Lab color) {
    return {
        color.L,
        sqrtf(color.a * color.a + color.b * color.b),
        atan2f(color.b, color.a)
    };
}

template<>
inline Lab ColorTo<Lab, LCh>(LCh color) {
    return {
        color.L,
        color.C * cosf(color.h),
        color.C * sinf(color.h)
    };
}

template<>
inline LCh ColorTo<LCh, RGB>(RGB color) {
    return ColorTo<LCh>(ColorTo<Lab>(color));
}

template<>
inline RGB ColorTo<RGB, LCh>(LCh color) {
    return ColorTo<RGB>(ColorTo<Lab>(color));
}

inline LCh LerpLCh(LCh a, LCh b, float t) {
    float x = std::lerp(cosf(a.h), cosf(b.h), t);
    float y = std::lerp(sinf(a.h), sinf(b.h), t);
    return LCh{
        std::lerp(a.L, b.L, t),
        std::lerp(a.C, b.C, t),
        atan2f(y, x)
    };
}