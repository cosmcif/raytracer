#ifndef OBJECT_H
#define OBJECT_H

#include "Core.h"
#include "Material.h"

/**
 * @brief General class for representing objects in a scene.
 */
class Object {

protected:
  // Matrix for local-to-global coordinate transformation.
  glm::mat4 transformationMatrix;

  // Matrix for global-to-local coordinate transformation.
  glm::mat4 inverseTransformationMatrix;

  // Matrix for transforming normal vectors from local to global coordinates.
  glm::mat4 normalMatrix;

public:
  // Color of the object.
  glm::vec3 color;

  // Structure describing the material of the object.
  Material material;
  /**
   * @brief Computes the intersection of the object with a given ray.
   * @param ray The ray to check for intersection.
   * @return The Hit structure representing the intersection.
   */
  virtual Hit intersect(Ray & ray) = 0;

  /**
   * @brief Gets the material structure of the object.
   * @return The Material structure describing the material of the object.
   */
  Material getMaterial() const { return material; }

  /**
   * @brief Sets the material of the object.
   * @param material A structure describing the material of the object.
   */
  void setMaterial(Material newMaterial) { this->material = newMaterial; }

  /**
   * @brief Sets up all the transformation matrices for the object.
   * @param matrix The matrix representing the transformation of the object in
   * global coordinates.
   */
  void setTransformation(glm::mat4 matrix) {
    transformationMatrix = matrix;
    inverseTransformationMatrix = glm::inverse(transformationMatrix);
    normalMatrix = glm::transpose(inverseTransformationMatrix);
  }

};

#endif // OBJECT_H
