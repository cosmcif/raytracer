// core.h

#ifndef CORE_H
#define CORE_H

#include "glm/geometric.hpp"
#include "glm/glm.hpp"

/**
 * @brief Class representing a single ray.
 */
class Ray {
public:
  // Origin of the ray.
  glm::vec3 origin;

  // Direction of the ray.
  glm::vec3 direction;

  /**
   * @brief Constructor for the ray.
   * @param origin Origin of the ray.
   * @param direction Direction of the ray.
   */
  Ray(glm::vec3 origin, glm::vec3 direction)
      : origin(origin), direction(direction) {}
};

class Object;

/**
 * @brief Structure representing the result of hitting an object.
 */
struct Hit {
  // Boolean indicating whether there was an intersection with an object.
  bool hit;

  // Normal vector of the intersected object at the intersection point.
  glm::vec3 normal;

  // Point of intersection.
  glm::vec3 intersection;

  // Distance from the origin of the ray to the intersection point.
  float distance;

  // A pointer to the intersected object.
  Object *object;

  // Coordinates for computing the texture (texture coordinates).
  glm::vec2 uv;

  glm::vec3 normalShading;
};

// add bvh box

#endif // CORE_H
