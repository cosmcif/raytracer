/**
@file main.cpp
*/

#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

#include "Image.h"
#include "Material.h"

using namespace std;

/**
 Class representing a single ray.
 */
class Ray {
public:
  glm::vec3 origin;    ///< Origin of the ray
  glm::vec3 direction; ///< Direction of the ray
                       /**
                        Contructor of the ray
                        @param origin Origin of the ray
                        @param direction Direction of the ray
                        */
  Ray(glm::vec3 origin, glm::vec3 direction)
      : origin(origin), direction(direction) {}
};

class Object;

/**
 Structure representing the even of hitting an object
 */
struct Hit {
  bool hit;         ///< Boolean indicating whether there was or there was no
                    ///< intersection with an object
  glm::vec3 normal; ///< Normal vector of the intersected object at the
                    ///< intersection point
  glm::vec3 intersection; ///< Point of Intersection
  float distance; ///< Distance from the origin of the ray to the intersection
                  ///< point
  Object *object; ///< A pointer to the intersected object
  glm::vec2 uv; ///< Coordinates for computing the texture (texture coordinates)
};

/**
 General class for the object
 */
class Object {
public:
  glm::vec3 color;   ///< Color of the object
  Material material; ///< Structure describing the material of the object
  /** A function computing an intersection, which returns the structure Hit */
  virtual Hit intersect(Ray ray) = 0;

  /** Function that returns the material struct of the object*/
  Material getMaterial() { return material; }
  /** Function that set the material
   @param material A structure describing the material of the object
  */
  void setMaterial(Material material) { this->material = material; }
};

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

class Triangle : public Object {
private:
  glm::vec3 vertexA;
  glm::vec3 vertexB;
  glm::vec3 vertexC;
  glm::vec3 normalA;
  glm::vec3 normalB;
  glm::vec3 normalC;
  bool vertexNormals = false;
  glm::vec3 normal;

public:
  /**
   Constructor for triangle, given vertices and point normals
  */
  Triangle(glm::vec3 vertexA, glm::vec3 vertexB, glm::vec3 vertexC,
           glm::vec3 normalA, glm::vec3 normalB, glm::vec3 normalC)
      : vertexA(vertexA), vertexB(vertexB), vertexC(vertexC), normalA(normalA),
        normalB(normalB), normalC(normalC), vertexNormals(true) {
    // normal = glm::normalize(normalA + normalB + normalC);
    glm::vec3 AB = vertexB - vertexA;
    glm::vec3 AC = vertexC - vertexA;
    normal = glm::normalize(glm::cross(AB, AC));
  }

  /**
 Constructor for triangle, given vertices and point normals and material
*/
  Triangle(glm::vec3 vertexA, glm::vec3 vertexB, glm::vec3 vertexC,
           glm::vec3 normalA, glm::vec3 normalB, glm::vec3 normalC,
           Material material)
      : vertexA(vertexA), vertexB(vertexB), vertexC(vertexC), normalA(normalA),
        normalB(normalB), normalC(normalC), vertexNormals(true) {
    // normal = glm::normalize(normalA + normalB + normalC);
    glm::vec3 AB = vertexB - vertexA;
    glm::vec3 AC = vertexC - vertexA;
    normal = glm::normalize(glm::cross(AB, AC));
    this->material = material;
  }

  /**
   Constructor for triangle, given only vertices
  */
  Triangle(glm::vec3 vertexA, glm::vec3 vertexB, glm::vec3 vertexC)
      : vertexA(vertexA), vertexB(vertexB), vertexC(vertexC) {
    glm::vec3 AB = vertexB - vertexA;
    glm::vec3 AC = vertexC - vertexA;
    normal = glm::normalize(glm::cross(AB, AC));
  }

  /**
   Constructor for triangle, given only vertices & material
  */
  Triangle(glm::vec3 vertexA, glm::vec3 vertexB, glm::vec3 vertexC,
           Material material)
      : vertexA(vertexA), vertexB(vertexB), vertexC(vertexC) {
    glm::vec3 AB = vertexB - vertexA;
    glm::vec3 AC = vertexC - vertexA;
    normal = glm::normalize(glm::cross(AB, AC));
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

    // a plane is a point with a normal
    float podotN = glm::dot(vertexA - ray.origin, normal);
    float t = podotN / ddotN;
    if (t < 0) {
      return hit;
    }
    // the point is coplanar with the triangle

    // find intersection with the plane
    glm::vec3 td = ray.origin + t * ray.direction;

    glm::vec3 w0 = glm::cross(vertexB - td, vertexC - td); //
    glm::vec3 w1 = glm::cross(vertexC - td, vertexA - td); //
    glm::vec3 w2 = glm::cross(vertexA - td, vertexB - td); //

    if (glm::dot(normal, w0) < 0 || glm::dot(normal, w1) < 0 ||
        glm::dot(normal, w2) < 0) {
      return hit; // outside the triangle
    }

    if (vertexNormals) {
      float a0 =
          glm::length(w0) * (glm::dot(normal, w0) >= 0 ? 1 : -1) * 0.5; //
      float a1 =
          glm::length(w1) * (glm::dot(normal, w1) >= 0 ? 1 : -1) * 0.5; //
      float a2 =
          glm::length(w2) * (glm::dot(normal, w2) >= 0 ? 1 : -1) * 0.5; //
      float totA = a0 + a1 + a2;
      hit.normal = normalize((a0 / totA) * normalA + (a1 / totA) * normalB +
                             (a2 / totA) * normalC);
    } else {
      hit.normal = normal;
    }

    hit.intersection = td;
    hit.distance = t;
    hit.object = this;
    hit.hit = true;

    return hit;
  }
};

class BoundingBox : public Object {
public:
  glm::vec3 minBounds;
  glm::vec3 maxBounds;

  BoundingBox()
      : minBounds(glm::vec3(numeric_limits<float>::infinity())),
        maxBounds(glm::vec3(-numeric_limits<float>::infinity())) {}

  BoundingBox(const glm::vec3 &minBounds, const glm::vec3 &maxBounds)
      : minBounds(minBounds), maxBounds(maxBounds) {}

  Hit intersect(Ray ray) {
    Hit hit;
    hit.hit = false;

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (minBounds.x - ray.origin.x) / ray.direction.x;
    tmax = (maxBounds.x - ray.origin.x) / ray.direction.x;

    if (tmin > tmax)
      swap(tmin, tmax);

    tymin = (minBounds.y - ray.origin.y) / ray.direction.y;
    tymax = (maxBounds.y - ray.origin.y) / ray.direction.y;

    if (tymin > tymax)
      swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
      return hit;

    if (tymin > tmin)
      tmin = tymin;
    if (tymax < tmax)
      tmax = tymax;

    tzmin = (minBounds.z - ray.origin.z) / ray.direction.z;
    tzmax = (maxBounds.z - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax)
      swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
      return hit;

    // Choose the entry and exit points based on the t values
    float t_enter = max(max(tmin, tymin), tzmin);
    float t_exit = min(min(tmax, tymax), tzmax);

    // Check if there is a valid intersection
    if (t_enter > t_exit || t_exit < 0)
      return hit;

    // Calculate the intersection point
    glm::vec3 intersection_point = ray.origin + t_enter * ray.direction;

    // Set the hit information
    hit.hit = true;
    hit.intersection = intersection_point;
    hit.distance = t_enter;

    // Calculate the normal based on which axis has the maximum overlap
    if (t_enter == tmin) {
      hit.normal = glm::vec3(-1, 0, 0); // Intersection with the left face
    } else if (t_enter == tymin) {
      hit.normal = glm::vec3(0, -1, 0); // Intersection with the bottom face
    } else {
      hit.normal = glm::vec3(0, 0, -1); // Intersection with the front face
    }

    hit.object = this;
    return hit;
  }
};

/*
  Takes an Obj file and parses it, creating a list of Triangles
*/
class MeshLoader : public Object {
private:
  vector<glm::vec3> vertices;
  vector<glm::vec3> normals;
  vector<Triangle> triangles;
  BoundingBox boundingBox;

public:
  MeshLoader(string filename, glm::vec3 translation) {
    ifstream file(filename);
    if (!file.is_open()) {
      cout << "Could not open file " << filename << endl;
      return;
    }

    string line;

    float x, y, z, nx, ny, nz;
    int smoothShading = 0;
    glm::vec3 minBounds = glm::vec3(numeric_limits<float>::infinity());
    glm::vec3 maxBounds = glm::vec3(-numeric_limits<float>::infinity());

    while (getline(file, line)) {
      if (line[1] == 'n') {
        // normal
        sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
        normals.push_back(
            glm::vec3(x + translation.x, y + translation.y, z + translation.z));
      } else if (line[0] == 'v') {
        // vertex
        sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
        glm::vec3 vertex(x + translation.x, y + translation.y,
                         z + translation.z);
        vertices.push_back(vertex);
        minBounds.x = min(minBounds.x, vertex.x);
        minBounds.y = min(minBounds.y, vertex.y);
        minBounds.z = min(minBounds.z, vertex.z);

        maxBounds.x = max(maxBounds.x, vertex.x);
        maxBounds.y = max(maxBounds.y, vertex.y);
        maxBounds.z = max(maxBounds.z, vertex.z);
      } else if (line[0] == 's') {
        // smooth shading
        sscanf(line.c_str(), "s %d", &smoothShading);
      } else if (line[0] == 'f') {
        // face
        // if smoothShading == 0, there are no normals
        if (smoothShading == 0) {
          sscanf(line.c_str(), "f %f %f %f", &x, &y, &z);
          triangles.push_back(
              Triangle(vertices[x - 1], vertices[y - 1], vertices[z - 1]));
        } else {
          sscanf(line.c_str(), "f %f//%f %f//%f %f//%f", &x, &nx, &y, &ny, &z,
                 &nz);
          triangles.push_back(Triangle(vertices[x - 1], vertices[y - 1],
                                       vertices[z - 1], normals[nx - 1],
                                       normals[ny - 1], normals[nz - 1]));
        }
      }
    }

    file.close();

    boundingBox = BoundingBox(minBounds, maxBounds);
  }

  MeshLoader(string filename, glm::vec3 translation, Material material) {
    this->setMaterial(material);
    ifstream file(filename);
    if (!file.is_open()) {
      cout << "Could not open file " << filename << endl;
      return;
    }

    string line;

    float x, y, z, nx, ny, nz;
    int smoothShading = 0;

    glm::vec3 minBounds = glm::vec3(numeric_limits<float>::infinity());
    glm::vec3 maxBounds = glm::vec3(-numeric_limits<float>::infinity());

    while (getline(file, line)) {
      if (line[1] == 'n') {
        // normal
        sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
        normals.push_back(
            glm::vec3(x + translation.x, y + translation.y, z + translation.z));
      } else if (line[0] == 'v') {
        // vertex
        sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
        // vertices.push_back(
        //     glm::vec3(x + translation.x, y + translation.y, z +
        //     translation.z));
        glm::vec3 vertex(x + translation.x, y + translation.y,
                         z + translation.z);
        vertices.push_back(vertex);
        minBounds.x = min(minBounds.x, vertex.x);
        minBounds.y = min(minBounds.y, vertex.y);
        minBounds.z = min(minBounds.z, vertex.z);

        maxBounds.x = max(maxBounds.x, vertex.x);
        maxBounds.y = max(maxBounds.y, vertex.y);
        maxBounds.z = max(maxBounds.z, vertex.z);

      } else if (line[0] == 's') {
        // smooth shading
        sscanf(line.c_str(), "s %d", &smoothShading);
      } else if (line[0] == 'f') {
        // face
        // if smoothShading == 0, there are no normals
        if (smoothShading == 0) {
          sscanf(line.c_str(), "f %f %f %f", &x, &y, &z);
          triangles.push_back(Triangle(vertices[x - 1], vertices[y - 1],
                                       vertices[z - 1], material));
        } else {
          sscanf(line.c_str(), "f %f//%f %f//%f %f//%f", &x, &nx, &y, &ny, &z,
                 &nz);
          triangles.push_back(Triangle(
              vertices[x - 1], vertices[y - 1], vertices[z - 1],
              normals[nx - 1], normals[ny - 1], normals[nz - 1], material));
        }
      }
    }

    file.close();
    boundingBox = BoundingBox(minBounds, maxBounds);
  }

  Hit intersect(Ray ray) {
    Hit closest_hit;

    closest_hit.hit = false;
    closest_hit.distance = INFINITY;

    for (int k = 0; k < triangles.size(); k++) {
      if (boundingBox.intersect(ray).hit) {
        Hit hit = triangles[k].intersect(ray);
        if (hit.hit == true && hit.distance < closest_hit.distance) {
          // cout << hit.hit << endl;
          closest_hit = hit;
        }
      }
    }
    closest_hit.object = this;
    return closest_hit;
  }
};

/**
 Light class
 */
class Light {
public:
  glm::vec3 position; ///< Position of the light source
  glm::vec3 color;    ///< Color/intentisty of the light source
  Light(glm::vec3 position) : position(position) { color = glm::vec3(1.0); }
  Light(glm::vec3 position, glm::vec3 color)
      : position(position), color(color) {}
};

vector<Light *> lights; ///< A list of lights in the scene
glm::vec3 ambient_light(0.7);
vector<Object *> objects; ///< A list of all objects in the scene

/** Function for computing color of an object according to the Phong Model
 @param point A point belonging to the object for which the color is computer
 @param normal A normal vector the the point
 @param uv Texture coordinates
 @param view_direction A normalized direction from the point to the
 viewer/camera
 @param material A material structure representing the material of the object
*/
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec2 uv,
                     glm::vec3 view_direction, Material material) {

  glm::vec3 color(0.0);
  for (int light_num = 0; light_num < lights.size(); light_num++) {

    glm::vec3 light_direction =
        glm::normalize(lights[light_num]->position - point);
    glm::vec3 reflected_direction = glm::reflect(-light_direction, normal);

    float NdotL = glm::clamp(glm::dot(normal, light_direction), 0.0f, 1.0f);
    float VdotR =
        glm::clamp(glm::dot(view_direction, reflected_direction), 0.0f, 1.0f);
    glm::vec3 diffuse = material.diffuse * glm::vec3(NdotL);

    if (material.texture) {
      diffuse = material.texture(uv) * glm::vec3(NdotL);
    }

    glm::vec3 specular =
        material.specular * glm::vec3(pow(VdotR, material.shininess));

    float att = glm::distance(point, lights[light_num]->position);
    att = 1 / pow(max(0.5f, att), 2);
    color += lights[light_num]->color * (diffuse + specular) * att;
  }
  color += ambient_light * material.ambient;

  color = glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
  return color;
}

/**
 Functions that computes a color along the ray
 @param ray Ray that should be traced through the scene
 @return Color at the intersection point
 */
glm::vec3 trace_ray(Ray ray) {

  Hit closest_hit;

  closest_hit.hit = false;
  closest_hit.distance = INFINITY;

  for (int k = 0; k < objects.size(); k++) {
    Hit hit = objects[k]->intersect(ray);
    if (hit.hit == true && hit.distance < closest_hit.distance)
      closest_hit = hit;
  }

  glm::vec3 color(0.0);

  if (closest_hit.hit) {
    color = PhongModel(closest_hit.intersection, closest_hit.normal,
                       closest_hit.uv, glm::normalize(-ray.direction),
                       closest_hit.object->getMaterial());
  } else {
    color = glm::vec3(0.0, 0.0, 0.0);
  }
  return color;
}
/**
 Function defining the scene
 */
void sceneDefinition() {

  Material green_diffuse;
  green_diffuse.ambient = glm::vec3(0.07f, 0.09f, 0.07f);
  green_diffuse.diffuse = glm::vec3(0.5f, 1.0f, 0.5f);

  Material red_specular;
  red_specular.diffuse = glm::vec3(1.0f, 0.1f, 0.1f);
  red_specular.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
  red_specular.specular = glm::vec3(0.5);
  red_specular.shininess = 10.0;

  Material blue_specular;
  blue_specular.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
  blue_specular.diffuse = glm::vec3(0.3f, 0.7f, 1.0f);
  blue_specular.specular = glm::vec3(0.6);
  blue_specular.shininess = 100.0;

  Material yellow_specular;
  yellow_specular.diffuse = glm::vec3(1.0f, 0.9f, 0.2f);
  yellow_specular.ambient = glm::vec3(0.01f, 0.02f, 0.04f);
  yellow_specular.specular = glm::vec3(0.5);
  yellow_specular.shininess = 10.0;

  Material orange_specular;
  orange_specular.diffuse = glm::vec3(1.0f, 0.6f, 0.1f);
  orange_specular.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
  orange_specular.specular = glm::vec3(0.5);
  orange_specular.shininess = 10.0;

  Material blue_copper_specular;
  blue_copper_specular.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
  blue_copper_specular.diffuse = glm::vec3(0.2f, 0.8f, 0.8f);
  blue_copper_specular.specular = glm::vec3(0.6);
  blue_copper_specular.shininess = 100.0;

  Material cream_specular;
  cream_specular.diffuse = glm::vec3(1.0f, 0.8f, 0.9f);
  cream_specular.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
  cream_specular.specular = glm::vec3(0.5);
  cream_specular.shininess = 10.0;

  Material unounouno;
  unounouno.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
  unounouno.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
  unounouno.specular = glm::vec3(0.5);
  unounouno.shininess = 10.0;

  Material textured;
  textured.texture = &rainbowTexture;

  // add file obj at path meshes/armadillo.obj
  objects.push_back(
      new MeshLoader("./meshes/bunny.obj", glm::vec3(0, -3, 8), red_specular));
  objects.push_back(new MeshLoader("./meshes/armadillo.obj",
                                   glm::vec3(-4, -3, 10), unounouno));
  objects.push_back(
      new MeshLoader("./meshes/lucy.obj", glm::vec3(4, -3, 10), unounouno));

  // plane in the front
  objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, -0.1f),
                              glm::vec3(0.0f, 0.0f, 1.0f), green_diffuse));
  // plane in the back
  objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, 30.0f),
                              glm::vec3(0.0f, 0.0f, -1.0f), green_diffuse));

  // plane on the left
  objects.push_back(new Plane(glm::vec3(-15.0f, 12.0f, 14.995f),
                              glm::vec3(1.0f, 0.0f, 0.0f), red_specular));
  // plane on the right
  objects.push_back(new Plane(glm::vec3(15.0f, 12.0f, 14.995f),
                              glm::vec3(-1.0f, 0.0f, 0.0f), blue_specular));

  // plane on bottom
  objects.push_back(new Plane(glm::vec3(0.0f, -3.0f, 14.995f),
                              glm::vec3(0.0f, 1.0f, 0.0f), yellow_specular));
  // plane on top
  objects.push_back(new Plane(glm::vec3(0.0f, 27.0f, 14.995f),
                              glm::vec3(0.0f, -1.0f, 0.0f), yellow_specular));

  lights.push_back(
      new Light(glm::vec3(0, 26, 5), glm::vec3(130.0))); // top light
  lights.push_back(
      new Light(glm::vec3(0, 1, 12), glm::vec3(15.0))); // floor light
  lights.push_back(new Light(glm::vec3(0, 5, 1), glm::vec3(45.0)));
}

/**
 Function performing tonemapping of the intensities computed using the raytracer
 @param intensity Input intensity
 @return Tonemapped intensity in range (0,1)
 */
glm::vec3 toneMapping(glm::vec3 intensity) {

  float alpha = 1.5f;
  float beta = 1.8f;
  float gamma = 2.2f;
  float oneOverGamma = 1.0 / gamma;

  glm::vec3 new_intensity;

  // new_intensity =
  //     alpha * pow(pow(intensity, glm::vec3(beta)), glm::vec3(oneOverGamma));
  new_intensity.x = pow(alpha * pow(intensity.x, beta), oneOverGamma);
  new_intensity.y = pow(alpha * pow(intensity.y, beta), oneOverGamma);
  new_intensity.z = pow(alpha * pow(intensity.z, beta), oneOverGamma);
  // cout << new_intensity.x << endl;
  // cout << new_intensity.y << endl;
  // cout << new_intensity.z << endl;

  glm::vec3 tonemapped =
      min(new_intensity, glm::vec3(1.0)); // tonemapped intensity
  return glm::clamp(tonemapped, glm::vec3(0.0), glm::vec3(1.0));
}

int main(int argc, const char *argv[]) {

  clock_t t = clock(); // variable for keeping the time of the rendering

  // int width = 1024; // width of the image
  int width = 320;
  // int height = 768; // height of the image
  int height = 210;
  float fov = 90; // field of view

  sceneDefinition(); // Let's define a scene

  Image image(width, height); // Create an image where we will store the result

  float s = 2 * tan(0.5 * fov / 180 * M_PI) / width;
  float X = -s * width / 2;
  float Y = s * height / 2;

  for (int i = 0; i < width; i++)
    for (int j = 0; j < height; j++) {

      float dx = X + i * s + s / 2;
      float dy = Y - j * s - s / 2;
      float dz = 1;

      glm::vec3 origin(0, 0, 0);
      glm::vec3 direction(dx, dy, dz);
      direction = glm::normalize(direction);

      Ray ray(origin, direction);

      image.setPixel(i, j, toneMapping(trace_ray(ray)));
    }

  t = clock() - t;
  cout << "It took " << ((float)t) / CLOCKS_PER_SEC
       << " seconds to render the image." << endl;
  cout << "I could render at " << (float)CLOCKS_PER_SEC / ((float)t)
       << " frames per second." << endl;

  // Writing the final results of the rendering
  if (argc == 2) {
    image.writeImage(argv[1]);
  } else {
    image.writeImage("./result.ppm");
  }

  return 0;
}
