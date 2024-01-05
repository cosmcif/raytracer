//
//  Material.h
//  Raytracer
//
#ifndef Material_h
#define Material_h

#include "Textures.h"
#include "glm/glm.hpp"

/**
 Structure describing a material of an object
 */
struct Material {
    glm::vec3 ambient = glm::vec3(0.0);
    glm::vec3 diffuse = glm::vec3(1.0);
    glm::vec3 specular = glm::vec3(0.0);

    float reflection = 0.0f;
    float refraction = 0.0f;
    float sigma = 1.0f;

    float shininess = 0.0f;

    glm::vec3 (*texture)(glm::vec2 uv) = nullptr;

    bool hasNormalMap = false;

    glm::vec3 (*normalMap)(glm::vec2 uv) = nullptr;

    bool isAnisotropic = false;
    float alpha_x = 0.05f;
    float alpha_y = 0.16f;
    // https://cseweb.ucsd.edu/~ravir/6998/papers/p265-ward.pdf

    bool hasImgTexture = false;
    float_t (*roughness)(glm::vec2 uv) = nullptr;
    float_t (*occlusion)(glm::vec2 uv) = nullptr;


};

#endif /* Material_h */
