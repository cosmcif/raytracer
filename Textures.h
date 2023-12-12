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

#endif /* Textures_h */