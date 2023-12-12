#ifndef Textures_h
#define Textures_h

#include <iostream>

#include "glm/fwd.hpp"
#include "bmpmini.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"

class Texture {
public:
    virtual glm::vec3 operator()(glm::vec2 uv) const = 0;

    virtual glm::vec3 texture_color(const glm::vec2 &uv) = 0;

    virtual glm::vec3 texture_normal(const glm::vec2 &uv) = 0;

    virtual float_t texture_ambient_occlusion(const glm::vec2 &uv) = 0;

    virtual float_t texture_roughness(const glm::vec2 &uv) = 0;
};

glm::vec3 noisy(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(1, 1, 0.5f);
    glm::vec3 v_scales = glm::vec3(0.5f, 1, 1);
    glm::vec3 offsets = glm::vec3(0.5f, 0, 0);

    const float r = glm::simplex(glm::vec2{(uv.x) * u_scales.r,
                                           uv.y * v_scales.r + offsets.r}),
            g = glm::simplex(glm::vec2{(uv.x) * u_scales.g,
                                       uv.y * v_scales.g + offsets.g}),
            b = glm::simplex(glm::vec2{(uv.x) * u_scales.b,
                                       uv.y * v_scales.b + offsets.b});

    return {
            (r > 0.1 && r < 0.2) || (r > 0.7 && r < 0.8),
            (g > 0.1 && g < 0.2) || (g > 0.7 && g < 0.8),
            (b > 0.1 && b < 0.2) || (b > 0.7 && b < 0.8)
    };
}

glm::vec3 terrainNoisy(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Adjust scales for more terrain-like features
    glm::vec3 v_scales = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f); // Keep offsets minimal

    // Generate noise values for red, green, and blue channels
    float r = glm::simplex(glm::vec2{(uv.x) * u_scales.r, uv.y * v_scales.r + offsets.r});
    float g = glm::simplex(glm::vec2{(uv.x) * u_scales.g, uv.y * v_scales.g + offsets.g});
    float b = glm::simplex(glm::vec2{(uv.x) * u_scales.b, uv.y * v_scales.b + offsets.b});

    // Combine and scale the noise values to create a brownish color
    float brownR = 0.6f + 0.4f * r; // Scale towards brown-red
    float brownG = 0.4f + 0.3f * g; // Scale towards brown-green
    float brownB = 0.2f + 0.2f * b; // Scale towards brown-blue

    return glm::vec3(brownR, brownG, brownB);
}

glm::vec3 perlinNoisy(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Adjusted scales for terrain-like features
    glm::vec3 v_scales = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f); // Minimal offsets

    // Generate Perlin noise values for each channel
    float r = glm::perlin(glm::vec2{uv.x * u_scales.r, uv.y * v_scales.r + offsets.r});
    float g = glm::perlin(glm::vec2{uv.x * u_scales.g, uv.y * v_scales.g + offsets.g});
    float b = glm::perlin(glm::vec2{uv.x * u_scales.b, uv.y * v_scales.b + offsets.b});

    // Adjust noise values to create a brown color
    float brownR = 0.6f + 0.4f * r; // Scale towards brown-red
    float brownG = 0.4f + 0.3f * g; // Scale towards brown-green
    float brownB = 0.2f + 0.2f * b; // Scale towards brown-blue

    return glm::vec3(brownR, brownG, brownB);
}


// Icy terrain using Perlin noise
glm::vec3 icy_perlin(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(5.0f, 5.0f, 5.0f); // Smooth, large-scale features
    glm::vec3 v_scales = glm::vec3(5.0f, 5.0f, 5.0f); // Smooth, large-scale features
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f); // Minimal offsets

    // Generate Perlin noise for the texture
    float r = glm::perlin(glm::vec2{uv.x * u_scales.r, uv.y * v_scales.r + offsets.r});
    float g = glm::perlin(glm::vec2{uv.x * u_scales.g, uv.y * v_scales.g + offsets.g});
    float b = glm::perlin(glm::vec2{uv.x * u_scales.b, uv.y * v_scales.b + offsets.b});

    // Adjust noise values to create an icy appearance
    // Adjust noise values to create an icy appearance
    float blue = 1.0f + 1.0f * b; // Deeper blue for areas of thicker ice
    float green = 1.0f + 1.0f * g; // A bit more green for depth
    float red = 1.0f + 0.5 * r; // A hint of red to avoid a monochromatic blue
    return glm::vec3(red, green, blue);
}

// Icy terrain using Simplex noise
glm::vec3 icy_simplex(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Smooth, large-scale features
    glm::vec3 v_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Smooth, large-scale features
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f); // Minimal offsets

    // Generate Simplex noise for the texture
    float r = glm::simplex(glm::vec2{(uv.x) * u_scales.r, uv.y * v_scales.r + offsets.r});
    float g = glm::simplex(glm::vec2{(uv.x) * u_scales.g, uv.y * v_scales.g + offsets.g});
    float b = glm::simplex(glm::vec2{(uv.x) * u_scales.b, uv.y * v_scales.b + offsets.b});

    // Adjust noise values to create an icy appearance
    float blue = 1.0f + 1.0f * b; // Deeper blue for areas of thicker ice
    float green = 1.0f + 1.0f * g; // A bit more green for depth
    float red = 1.0f + 0.5 * r; // A hint of red to avoid a monochromatic blue
    return glm::vec3(red, green, blue);
}

glm::vec3 ice_perlin_noise(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Smooth, large-scale features
    glm::vec3 v_scales = glm::vec3(10.0f, 10.0f, 10.0f); // Smooth, large-scale features
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 0.0f); // Minimal offsets

    // Generate Simplex noise for the texture
    glm::vec3 v;

    v.r = glm::perlin(glm::vec2{(uv.x) * u_scales.r, uv.y * v_scales.r + offsets.r});
    v.g = glm::perlin(glm::vec2{(uv.x) * u_scales.g, uv.y * v_scales.g + offsets.g});
    v.b = glm::perlin(glm::vec2{(uv.x) * u_scales.b, uv.y * v_scales.b + offsets.b});

    glm::vec3 col1_rgb = glm::vec3(0.722, 0.961, 0.937);
    glm::vec3 col2_rgb = glm::vec3(0.075, 0.482, 0.631);
    glm::vec3 result;
    result.r = v.r * col1_rgb.r + ((1.0f - v.r) * col2_rgb.r);
    result.g = v.g * col1_rgb.g + ((1.0f - v.g) * col2_rgb.g);
    result.b = v.b * col1_rgb.b + ((1.0f - v.b) * col2_rgb.b);

    return result;
}


float simplexNoise(int octaves, float persistence, float scale, float x, float y) {
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0; // Used for normalizing result to 0.0 - 1.0

    for (int i = 0; i < octaves; ++i) {
        total += glm::simplex(glm::vec2(x * frequency / scale, y * frequency / scale)) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2;
    }

    // Normalize to range [0, 1]
    return total / maxValue;
}

glm::vec3 perlinoNoisy(glm::vec2 uv) {
    float v = simplexNoise(5, 0.5, 63, uv.x, uv.y);
    glm::vec3 col1_rgb = glm::vec3(0.722, 0.961, 0.937);
    glm::vec3 col2_rgb = glm::vec3(0.075, 0.482, 0.631);
    glm::vec3 result;
    result.r = v * col1_rgb.r + ((1.0f - v) * col2_rgb.r);
    result.g = v * col1_rgb.g + ((1.0f - v) * col2_rgb.g);
    result.b = v * col1_rgb.b + ((1.0f - v) * col2_rgb.b);

    return result;
}


#endif /* Textures_h */