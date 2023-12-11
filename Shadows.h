//
// Created by sofi on 10/12/23.
//

#ifndef RAYTRACER_SHADOWS_H
#define RAYTRACER_SHADOWS_H

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include <vector>
#include <omp.h>


class SoftShadow {
private:
    glm::vec3 lightPosition;
    int numSamples = 8;
public:

    explicit SoftShadow(const glm::vec3 &lightPosition)
            : lightPosition(lightPosition) {
    }

    static glm::vec3 randomDirection(const glm::vec3 &normal, float m = 1.0f) {
        // Generate a random direction in the local tangent space
        glm::vec3 tangentX, tangentY;
        glm::vec3 tmp = glm::abs(normal.y) > 0.99f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        tangentX = glm::normalize(glm::cross(normal, tmp));
        tangentY = glm::cross(normal, tangentX);

        // Generate random spherical coordinates
        float theta = glm::acos(glm::pow(1 - glm::linearRand(0.0f, 1.0f), 1 / (1 + m)));  // Polar angle
        float phi = glm::linearRand(0.0f, 2.0f * static_cast<float>(M_PI));       // Azimuthal angle

        // Convert spherical coordinates to Cartesian coordinates in the local tangent space
        float sinTheta = glm::sin(theta);
        float x = sinTheta * glm::cos(phi);
        float y = sinTheta * glm::sin(phi);
        float z = glm::cos(theta);

        // Transform the direction to the world coordinate system
        return glm::normalize(tangentX * x + tangentY * y + normal * z);
    }


    glm::vec3 computeSoftShadow(const glm::vec3 &point, const glm::vec3 &normal, const std::vector<Object *> &objects) {
        int blockedRays = 0;

#pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < numSamples; ++i) {
            // Generate random direction within a hemisphere
            glm::vec3 randomDir = randomDirection(normal);

            // Shoot shadow ray towards the light source
            Ray ray(point + 0.001f * normal, randomDir);
            float r = glm::distance(point, lightPosition);
            r = std::max(r, 0.1f);

            // Check for intersections with scene objects
            for (const auto &object: objects) {
                Hit hit = object->intersect(ray);
                if (hit.hit && hit.distance < r) {
                    blockedRays++;
                    break;
                }
            }
        }

        // Compute the percentage of unblocked rays
        float shadowFactor = 1.0f - (static_cast<float>(blockedRays) / numSamples);
        return glm::vec3(shadowFactor);
    }
};

#endif //RAYTRACER_SHADOWS_H
