
#ifndef OBJECTS_H
#define OBJECTS_H

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
  Sphere(glm::vec3 color) { this->color = color; }

  /**
   * @brief Constructor for the sphere with a specified material.
   * @param material Material of the sphere.
   */
  Sphere(Material material) { this->material = material; }

  /**
   * @brief Implementation of the intersection function.
   * @param ray The ray to check for intersection.
   * @return The Hit structure representing the intersection.
   */
  Hit intersect(Ray &ray) {

    Ray newRay = toLocalRay(ray);
    glm::vec3 newOrigin = newRay.origin;
    glm::vec3 newDirection = newRay.direction;

    glm::vec3 c = center - newOrigin;
    float cdotc = glm::dot(c, c);
    float cdotd = glm::dot(c, newDirection);

    Hit hit;

    float D = 0;
    if (cdotc > cdotd * cdotd) {
      D = sqrt(cdotc - cdotd * cdotd);
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

      hit.uv.s = (asin(newNormal.y) + M_PI / 2) / M_PI;
      hit.uv.t = (atan2(newNormal.z, newNormal.x) + M_PI) / (2 * M_PI);
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
   * @brief Constructor for the plane with a specified point and normal.
   * @param point A point on the plane.
   * @param normal Normal vector of the plane.
   */
  Plane(glm::vec3 point, glm::vec3 normal) : point(point), normal(normal) {}

  /**
   * @brief Constructor for the plane with a specified point, normal, and
   * material.
   * @param point A point on the plane.
   * @param normal Normal vector of the plane.
   * @param material Material of the plane.
   */
  Plane(glm::vec3 point, glm::vec3 normal, Material material)
      : point(point), normal(normal) {
    this->material = material;
  }

  /**
   * @brief Implementation of the intersection function for the plane.
   * @param ray The ray to check for intersection.
   * @return The Hit structure representing the intersection.
   */
  Hit intersect(Ray &ray) {
    Hit hit;
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
  Cone(Material material) {
    this->material = material;
    base = new Plane(glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), material);
  }

  /**
   * @brief Implementation of the intersection function for the cone.
   * @param ray The ray to check for intersection.
   * @return The Hit structure representing the intersection.
   */
  Hit intersect(Ray &ray) {
    Hit hit;
    hit.hit = false;
    float height = 1.0;

    glm::vec3 newOrigin =
        glm::vec3(inverseTransformationMatrix * glm::vec4(ray.origin, 1.0));
    glm::vec3 newDirection = glm::normalize(
        glm::vec3(inverseTransformationMatrix * glm::vec4(ray.direction, 0.0)));

    // Solve for the intersection of the ray and the cone
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

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

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
    return hit;
  }
};

#endif