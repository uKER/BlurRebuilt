#ifndef RESCALESURFACE_H__INCLUDED
#define RESCALESURFACE_H__INCLUDED

// Color and Alpha filters
#define CSTR_UNDEFINED       "UNDEFINED"
#define CSTR_POINT           "POINT"
#define CSTR_BOX             "BOX"
#define CSTR_TRIANGLE        "TRIANGLE"
#define CSTR_HERMITE         "HERMITE"
#define CSTR_HANNING         "HANNING"
#define CSTR_HAMMING         "HAMMING"
#define CSTR_BLACKMAN        "BLACKMAN"
#define CSTR_GAUSSIAN        "GAUSSIAN"
#define CSTR_QUADRATIC       "QUADRATIC"
#define CSTR_CUBIC           "CUBIC"
#define CSTR_CATROM          "CATROM"
#define CSTR_MITCHELL        "MITCHELL"
#define CSTR_LANCZOS         "LANCZOS"
#define CSTR_BESSEL          "BESSEL"
#define CSTR_SINC            "SINC"

// Edge filters
#define CSTR_CLAMP           "CLAMP"
#define CSTR_MIRROR          "MIRROR"
#define CSTR_TILE            "TILE"

#define DEFAULT_COLOR_FILTER "LANCZOS"
#define DEFAULT_ALPHA_FILTER "LANCZOS"

namespace Texutils
{

ACTRESULT RescaleTexture( ActTools::Texture* aTexture, float x_factor, float y_factor,
                          const char* aColorFilter, float aColorBlur,
                          const char* aAlphaFilter, float aAlphaBlur,
                          const char* aEdgeFilterX, const char* aEdgeFilterY );

ACTRESULT ResizeTexture( ActTools::Texture* aTexture, int xSize, int ySize,
                          const char* aColorFilter, float aColorBlur,
                          const char* aAlphaFilter, float aAlphaBlur,
                          const char* aEdgeFilterX, const char* aEdgeFilterY );

}


#endif // RESCALESURFACE_H__INCLUDED

