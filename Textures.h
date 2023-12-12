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

class TextureNoise : public Texture {
protected:
    const glm::vec3 u_scales; // Scales of the noise space along the u-axis for colors.
    const glm::vec3 v_scales; // Scales of the noise space along the v-axis for colors.
    const glm::vec3 offsets;  // Offsets on the noise space for colors.

public:
    TextureNoise(glm::vec3 u_scales, glm::vec3 v_scales, glm::vec3 offsets)
            : u_scales{u_scales}, v_scales{v_scales}, offsets{offsets} {};

    glm::vec3 texture_color(const glm::vec2 &uv) { return {0, 0, 0}; }

    glm::vec3 texture_normal(const glm::vec2 &uv) { return {0, 0, 0}; }

    float_t texture_ambient_occlusion(const glm::vec2 &uv) { return 0; }

    float_t texture_roughness(const glm::vec2 &uv) { return 0; }

    glm::vec3 operator()(glm::vec2 &uv) const {
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


};

#endif /* Textures_h */