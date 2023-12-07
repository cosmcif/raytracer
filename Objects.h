
#ifndef OBJECTS_H
#define OBJECTS_H

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include "Object.h"

using namespace std;

/**
 Implementation of the class Object for sphere shape.
 */
class Sphere : public Object {
private:
  float radius;     ///< Radius of the sphere
  glm::vec3 center; ///< Center of the sphere

public:
  /**
   The constructor of the sphere
   @param radius Radius of the sphere
   @param center Center of the sphere
   @param color Color of the sphere
   */
  Sphere(float radius, glm::vec3 center, glm::vec3 color)
      : radius(radius), center(center) {
    this->color = color;
  }
  Sphere(float radius, glm::vec3 center, Material material)
      : radius(radius), center(center) {
    this->material = material;
  }
  /** Implementation of the intersection function*/
  Hit intersect(Ray ray) {

    glm::vec3 c = center - ray.origin;

    float cdotc = glm::dot(c, c);
    float cdotd = glm::dot(c, ray.direction);

    Hit hit;

    float D = 0;
    if (cdotc > cdotd * cdotd) {
      D = sqrt(cdotc - cdotd * cdotd);
    }
    if (D <= radius) {
      hit.hit = true;
      float t1 = cdotd - sqrt(radius * radius - D * D);
      float t2 = cdotd + sqrt(radius * radius - D * D);

      float t = t1;
      if (t < 0)
        t = t2;
      if (t < 0) {
        hit.hit = false;
        return hit;
      }

      hit.intersection = ray.origin + t * ray.direction;
      hit.normal = glm::normalize(hit.intersection - center);
      hit.distance = glm::distance(ray.origin, hit.intersection);
      hit.object = this;

      hit.uv.s = (asin(hit.normal.y) + M_PI / 2) / M_PI;
      hit.uv.t = (atan2(hit.normal.z, hit.normal.x) + M_PI) / (2 * M_PI);

    } else {
      hit.hit = false;
    }
    return hit;
  }
};

class Plane : public Object {

private:
  glm::vec3 normal;
  glm::vec3 point;

public:
  Plane(glm::vec3 point, glm::vec3 normal) : point(point), normal(normal) {}
  Plane(glm::vec3 point, glm::vec3 normal, Material material)
      : point(point), normal(normal) {
    this->material = material;
  }
  Hit intersect(Ray ray) {

    Hit hit;
    hit.hit = false;

    float ddotN = glm::dot(ray.direction, normal);

    if (ddotN == 0) {
      // the ray is parallel to the plane
      return hit;
    }

    float podotN = glm::dot(point - ray.origin, normal);
    float t = podotN / ddotN;
    if (t < 0) {
      return hit;
    }
    hit.intersection = ray.origin + t * ray.direction;
    // hit.normal = glm::normalize(hit.intersection);
    hit.normal = normal;
    // hit.distance = glm::distance(ray.origin, hit.intersection);
    hit.distance = t;
    hit.object = this;
    hit.hit = true;

    return hit;
  }
};


class Cone : public Object {
private:
  Plane *base;

public:
  Cone(Material material) {
    this->material = material;
    base = new Plane(glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), material);
  }

  Hit intersect(Ray ray) {
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
        length(basehit.intersection - glm::vec3(0, 1, 0)) <= 1.0) {
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