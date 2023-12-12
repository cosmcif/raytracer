#ifndef Textures_h
#define Textures_h

#include <iostream>

#include "glm/fwd.hpp"
#include "bmpmini.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"

glm::vec3 perlinTerrain(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(30.0f);
    glm::vec3 v_scales = glm::vec3(30.0f);
    glm::vec3 offsets = glm::vec3(0.0f);

    float r = glm::perlin(glm::vec2{uv.x * u_scales.r, uv.y * v_scales.r + offsets.r});
    float g = glm::perlin(glm::vec2{uv.x * u_scales.g, uv.y * v_scales.g + offsets.g});
    float b = glm::perlin(glm::vec2{uv.x * u_scales.b, uv.y * v_scales.b + offsets.b});

    float brownR = 0.6f + 0.4f * r; // Scale towards brown-red
    float brownG = 0.4f + 0.3f * g; // Scale towards brown-green
    float brownB = 0.2f + 0.2f * b; // Scale towards brown-blue

    return glm::vec3(brownR, brownG, brownB);
}

glm::vec3 perlinIceTerrain(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(10);
    glm::vec3 v_scales = glm::vec3(10);
    glm::vec3 offsets = glm::vec3(10);

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


#endif /* Textures_h */