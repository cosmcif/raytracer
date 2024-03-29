
#ifndef OBJECTS_H
#define OBJECTS_H

#include <cmath>

#include "Object.h"

/**
 * @brief Class representing a sphere object.
 */
class Sphere : public Object {
private:
    // Radius of the sphere
    float radius = 1.0;

    // Center of the sphere
    glm::vec3 center = glm::vec3(0.0);

public:
    /**
     * @brief Constructor for the sphere with a specified color.
     * @param color Color of the sphere.
     */
    explicit Sphere(glm::vec3 color) { this->color = color; }

    /**
     * @brief Constructor for the sphere with a specified material.
     * @param material Material of the sphere.
     */
    explicit Sphere(Material material) { this->material = material; }

    /**
     * @brief Implementation of the intersection function.
     * @param ray The ray to check for intersection.
     * @return The Hit structure representing the intersection.
     */
    Hit intersect(Ray &ray) override {

        glm::vec3 newOrigin =
                glm::vec3(inverseTransformationMatrix * glm::vec4(ray.origin, 1.0));
        glm::vec3 newDirection = glm::normalize(
                glm::vec3(inverseTransformationMatrix * glm::vec4(ray.direction, 0.0)));

        glm::vec3 c = center - newOrigin;
        float cdotc = glm::dot(c, c);
        float cdotd = glm::dot(c, newDirection);

        Hit hit{};

        double D = 0;
        if (cdotc > cdotd * cdotd) {
            D = std::sqrt(cdotc - cdotd * cdotd);
        }

        if (D <= radius) {
            hit.hit = true;
            float t1 = cdotd - sqrt(radius * radius - D * D);
            float t2 = cdotd + sqrt(radius * radius - D * D);

            float t = (t1 < 0) ? t2 : t1;
            if (t < 0) {
                hit.hit = false;
                return hit;
            }

            glm::vec3 newIntersection = newOrigin + t * newDirection;
            glm::vec3 newNormal = glm::normalize(newIntersection - center);

            hit.intersection =
                    glm::vec3(transformationMatrix * glm::vec4(newIntersection, 1.0));
            hit.distance = glm::distance(ray.origin, hit.intersection);
            hit.object = this;

            glm::vec3 newNormalGlobal =
                    glm::normalize(glm::vec3(normalMatrix * glm::vec4(newNormal, 0.0)));
            hit.normal = newNormalGlobal;
            hit.normalShading = newNormalGlobal;

            hit.uv.s = (asin(newNormal.y) + M_PI / 2) / M_PI;
            hit.uv.t = (atan2(newNormal.z, newNormal.x) + M_PI) / (2 * M_PI);

            // FEAT: NORMAL MAPS
            if (material.hasNormalMap) {
                glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), hit.intersection));
                glm::vec3 bitangent = glm::normalize(glm::cross(hit.normal, tangent));
                hit.tangent = tangent;
                hit.bitangent = bitangent;

                glm::vec3 normal_map = glm::normalize(material.normalMap(hit.uv));

                glm::mat3 TBN = glm::mat3(tangent, bitangent, hit.normal);

                hit.normalShading = glm::normalize(TBN * normal_map);
            }



        } else {
            hit.hit = false;
        }
        return hit;
    }
};

/**
 * @brief Class representing a plane object.
 */
class Plane : public Object {

private:
    // Normal vector of the plane.
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

    // A point on the plane.
    glm::vec3 point = glm::vec3(0.0f);

public:
    /**
     * @brief Constructor for the plane with a specified point, normal, and
     * material.
     * @param point A point on the plane.
     * @param normal Normal vector of the plane.
     * @param hasMaterial If it has a material.
     * @param material Material of the plane.
     */
    Plane(glm::vec3 point, glm::vec3 normal, bool hasMaterial, Material material = Material())
            : point(point), normal(normal) {
        if (hasMaterial) {
            this->material = material;
        }
    }
    /**
     * @brief Implementation of the intersection function for the plane.
     * @param ray The ray to check for intersection.
     * @return The Hit structure representing the intersection.
     */
    Hit intersect(Ray &ray) override {
        Hit hit{};
        hit.hit = false;

        float ddotN = glm::dot(ray.direction, normal);

        if (ddotN == 0) {
            // The ray is parallel to the plane.
            return hit;
        }

        float podotN = glm::dot(point - ray.origin, normal);
        float t = podotN / ddotN;

        if (t < 0) {
            return hit;
        }

        hit.intersection = ray.origin + t * ray.direction;
        hit.normal = normal;
        hit.distance = t;
        hit.object = this;
        hit.hit = true;
        hit.uv.x = 0.1f * hit.intersection.x;
        hit.uv.y = 0.1f * hit.intersection.z;
        hit.normalShading = normal;

        // FEAT: NORMAL MAPS
        if (material.hasNormalMap) {
            glm::vec3 tangent = glm::vec3(0, 0, 1);
            glm::vec3 bitangent = glm::vec3(1, 0, 0);

            hit.tangent = tangent;
            hit.bitangent = bitangent;

            glm::vec3 normal_map = glm::normalize(material.normalMap(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, hit.normal);

            hit.normalShading = glm::normalize(TBN * normal_map);

        }

        return hit;
    }
};

/**
 * @brief Class representing a cone object.
 */
class Cone : public Object {
private:
    // Base of the cone represented as a plane.
    Plane *base;

public:
    /**
     * @brief Constructor for the cone with a specified material.
     * @param material Material of the cone.
     */
    explicit Cone(Material material) {
        this->material = material;
        base = new Plane(glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), true, material);
    }

    /**
     * @brief Implementation of the intersection function for the cone.
     * @param ray The ray to check for intersection.
     * @return The Hit structure representing the intersection.
     */
    Hit intersect(Ray &ray) override {

        Hit hit{};
        hit.hit = false;
        float height = 1.0;

        glm::vec3 newOrigin =
                glm::vec3(inverseTransformationMatrix * glm::vec4(ray.origin, 1.0));
        glm::vec3 newDirection = glm::normalize(
                glm::vec3(inverseTransformationMatrix * glm::vec4(ray.direction, 0.0)));

        float a = newDirection.x * newDirection.x +
                  newDirection.z * newDirection.z - newDirection.y * newDirection.y;
        float b = 2 * (newOrigin.x * newDirection.x + newOrigin.z * newDirection.z -
                       newOrigin.y * newDirection.y);
        float c = newOrigin.x * newOrigin.x + newOrigin.z * newOrigin.z -
                  newOrigin.y * newOrigin.y;

        float delta = b * b - 4 * a * c;

        if (delta < 0) {
            return hit;
        }

        float t1 = (-b - std::sqrt(delta)) / (2 * a);
        float t2 = (-b + std::sqrt(delta)) / (2 * a);

        float t = t1;
        glm::vec3 newIntersection = newOrigin + t * newDirection;
        if (t < 0 || newIntersection.y > height || newIntersection.y < 0) {
            t = t2;
            newIntersection = newOrigin + t * newDirection;
            if (t < 0 || newIntersection.y > height || newIntersection.y < 0) {
                return hit;
            }
        }

        glm::vec3 newNormal =
                glm::vec3(newIntersection.x, -newIntersection.y, newIntersection.z);
        newNormal = glm::normalize(newNormal);

        Ray rayy(newOrigin, newDirection);
        Hit basehit = base->intersect(rayy);

        if (basehit.hit && basehit.distance < t &&
            glm::length(basehit.intersection - glm::vec3(0, 1, 0)) <= 1.0) {
            newIntersection = basehit.intersection;
            newNormal = basehit.normal;
        }

        hit.hit = true;
        hit.object = this;

        glm::vec3 newIntersectionGlobal =
                glm::vec3(transformationMatrix * glm::vec4(newIntersection, 1.0));
        hit.intersection = newIntersectionGlobal;

        float newT = glm::distance(newIntersectionGlobal, ray.origin);
        hit.distance = newT;

        glm::vec3 newNormalGlobal =
                glm::normalize(glm::vec3(normalMatrix * glm::vec4(newNormal, 0.0)));
        hit.normal = newNormalGlobal;
        hit.normalShading = newNormalGlobal; // cone has no normal map available lol

        hit.uv.s = (asin(newNormal.y) + M_PI / 2) / M_PI;
        hit.uv.t = (atan2(newNormal.z, newNormal.x) + M_PI) / (2 * M_PI);

        return hit;
    }
};

#endif