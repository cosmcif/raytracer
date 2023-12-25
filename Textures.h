#ifndef Textures_h
#define Textures_h

#include <iostream>

#include "glm/fwd.hpp"
#include "bmpmini.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"


glm::vec3 perlinCalculations(glm::vec2 uv, glm::vec3 u_scales, glm::vec3 v_scales, glm::vec3 offsets) {

    float r = glm::perlin(glm::vec3{uv.x * u_scales.r, uv.y * v_scales.r, offsets.r});
    float g = glm::perlin(glm::vec3{uv.x * u_scales.g, uv.y * v_scales.g, offsets.g});
    float b = glm::perlin(glm::vec3{uv.x * u_scales.b, uv.y * v_scales.b, offsets.b});

    return {r, g, b};
}

glm::vec3 perlinNormal(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(0.0f, 0.0f, 50.f);
    glm::vec3 v_scales = glm::vec3(0.0f, 0.0f, 50.0f);
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 50.0f);

    glm::vec3 p = perlinCalculations(uv, u_scales, v_scales, offsets);
    p = 2.0f * p - 1.0f; // to -1,1
    p = -p;
    p = (p + 1.0f) / 2.0f; // to 0,1

    glm::vec3 normal = glm::normalize(glm::vec3(uv, p.z));

    return normal;

}
glm::vec3 perlinWater(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(0.0f, 0.0f, 50.f);
    glm::vec3 v_scales = glm::vec3(0.0f, 0.0f, 50.0f);
    glm::vec3 offsets = glm::vec3(0.0f, 0.0f, 50.0f);

    glm::vec3 p = perlinCalculations(uv, u_scales, v_scales, offsets);
    p = 2.0f * p - 1.0f; // to -1,1
    p = glm::normalize(p);
    p = (p + 1.0f) / 2.0f; // to 0,1

    glm::vec3 normal = glm::normalize(glm::vec3(uv, p.z));

    return normal;


    /*
    glm::vec3 u_scales = glm::vec3(25.0f, 20.f, 15.f);
    glm::vec3 v_scales = glm::vec3(15.0f, 25.0f, 35.0f);
    glm::vec3 offsets = glm::vec3(0.0f, 0.1f, 0.2f);

    glm::vec3 p = perlinCalculations(uv, u_scales, v_scales, offsets);

    p = 2.0f * p - 1.0f;
    p = glm::normalize(p);
    p = (p + 1.0f) / 2.0f;
    glm::vec3 base = glm::vec3(0.5f, 0.5f, 1.0f);


    float r = 0.5f + p.r;
    float g = 0.5f + p.g;
    float b = 1.0f + p.b;
    glm::vec3 color = glm::vec3(r, g, b);
    color = 2.0f * color - 1.0f;
    color = glm::normalize(color);
    color = (color + 1.0f) / 2.0f;

    return color;*/

}

glm::vec3 opal(glm::vec2 uv) {
    glm::vec3 u_scales = glm::vec3(20.0f, 10.f, 15.0f);
    glm::vec3 v_scales = glm::vec3(20.0f);
    glm::vec3 offsets = glm::vec3(0.2f, 0.3f, 0.4f);

    glm::vec3 p = perlinCalculations(uv, u_scales, v_scales, offsets);
    p = 2.0f * p - 1.0f;
    p = glm::normalize(p);
    p = (p + 1.0f) / 2.0f;
    glm::vec3 base = glm::vec3(0.5f, 0.5f, 1.0f);
    float r = 0.5f + 0.4f * p.r;
    float g = 0.5f + 0.3f * p.g;
    float b = 0.5f + 0.2f * p.b;


    glm::vec3 color = glm::normalize(glm::vec3(r, g, b));

    return {r, g, b};

}

glm::vec3 perlinTerrain(glm::vec2 uv) {
    glm::vec3 p = perlinCalculations(uv, glm::vec3(30.0f), glm::vec3(30.0f), glm::vec3(0.0f));

    float r = 0.2f + 0.4f * p.r; //0.6
    float g = 0.2f + 0.3f * p.g; //0.4
    float b = 0.2f + 0.2f * p.b;

    return {r, g, b};
}

glm::vec3 perlinIceTerrain(glm::vec2 uv) {

    glm::vec3 p = perlinCalculations(uv, glm::vec3(10.0f), glm::vec3(10.0f), glm::vec3(10.0f));

    glm::vec3 col1_rgb = glm::vec3(0.722, 0.961, 0.937);
    glm::vec3 col2_rgb = glm::vec3(0.075, 0.482, 0.631);

    float r = p.r * col1_rgb.r + ((1.0f - p.r) * col2_rgb.r);
    float g = p.g * col1_rgb.g + ((1.0f - p.g) * col2_rgb.g);
    float b = p.b * col1_rgb.b + ((1.0f - p.b) * col2_rgb.b);

    return {r, g, b};
}

glm::vec3 snowTerrain(glm::vec2 uv) {

    glm::vec3 p = perlinCalculations(uv, glm::vec3(10.0f), glm::vec3(10.0f), glm::vec3(10.0f));

    glm::vec3 col1_rgb = glm::vec3(0.722, 0.961, 0.937);
    glm::vec3 col2_rgb = glm::vec3(0.212, 0.51, 0.62);

    float r = p.r * col1_rgb.r + ((1.0f - p.r) * col2_rgb.r);
    float g = p.g * col1_rgb.g + ((1.0f - p.g) * col2_rgb.g);
    float b = p.b * col1_rgb.b + ((1.0f - p.b) * col2_rgb.b);

    return {r, g, b};
}

glm::vec3 qwilfishTexture(glm::vec2 uv) {
    float normalizedY = 0.5f * (uv.y + 3.0f);

    //glm::vec3 blu(0, 0.416, 0.42);
    //glm::vec3 ylw(1.0f, 1.0f, 0.0f);

    //glm::vec3 result = (1.0f - normalizedY) * ylw + normalizedY * blu;

    if (normalizedY<1){
        return {0.937, 0.922, 0.392}; // yellow
    } else {
        return {0, 0.416, 0.42}; // blue
    }
}

#endif /* Textures_h */