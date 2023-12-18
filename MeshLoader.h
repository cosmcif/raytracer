
#ifndef MESHLOADER_H
#define MESHLOADER_H

#include "Object.h"
#include <cmath>
#include <fstream>
#include <vector>

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
    glm::vec3 vertices[3];

    /**
     Constructor for triangle, given vertices and optionally point normals
    */
    Triangle(glm::vec3 vertexA, glm::vec3 vertexB, glm::vec3 vertexC,
             glm::vec3 normalA = glm::vec3(0.0f),
             glm::vec3 normalB = glm::vec3(0.0f),
             glm::vec3 normalC = glm::vec3(0.0f))
            : vertexA(vertexA), vertexB(vertexB), vertexC(vertexC), normalA(normalA),
              normalB(normalB), normalC(normalC),
              normal(glm::normalize(glm::cross(vertexB - vertexA, vertexC - vertexA))),
              vertices{vertexA, vertexB, vertexC},
            // if one of these is NOT 0, then it's true
            // what all normals are 0? is it even possible??
              vertexNormals(normalA != glm::vec3(0.0f) ||
                            normalB != glm::vec3(0.0f) ||
                            normalC != glm::vec3(0.0f)) {}

    Hit intersect(Ray &ray) override {
        Hit hit{};
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
private:
    glm::vec3 minBounds = glm::vec3(INFINITY);
    glm::vec3 maxBounds = glm::vec3(-INFINITY);

public:
    BoundingBox()
            : minBounds(glm::vec3(INFINITY)), maxBounds(glm::vec3(-INFINITY)) {}

    BoundingBox(glm::vec3 &minBounds, glm::vec3 &maxBounds)
            : minBounds(minBounds), maxBounds(maxBounds) {}

    explicit BoundingBox(std::vector<Triangle> &triangles) {
        for (const Triangle &t: triangles) {
            for (const glm::vec3 &vertex: t.vertices) {
                minBounds.x = std::min(minBounds.x, vertex.x);
                minBounds.y = std::min(minBounds.y, vertex.y);
                minBounds.z = std::min(minBounds.z, vertex.z);

                maxBounds.x = std::max(maxBounds.x, vertex.x);
                maxBounds.y = std::max(maxBounds.y, vertex.y);
                maxBounds.z = std::max(maxBounds.z, vertex.z);
            }
        }
    }

    Hit intersect(Ray &ray) override {
        Hit hit{};
        hit.hit = false;

        float tmin, tmax, ymin, ymax, zmin, zmax;

        tmin = (minBounds.x - ray.origin.x) / ray.direction.x;
        tmax = (maxBounds.x - ray.origin.x) / ray.direction.x;

        if (tmin > tmax)
            std::swap(tmin, tmax);

        ymin = (minBounds.y - ray.origin.y) / ray.direction.y;
        ymax = (maxBounds.y - ray.origin.y) / ray.direction.y;

        if (ymin > ymax)
            std::swap(ymin, ymax);

        if ((tmin > ymax) || (ymin > tmax))
            return hit;

        if (ymin > tmin)
            tmin = ymin;
        if (ymax < tmax)
            tmax = ymax;

        zmin = (minBounds.z - ray.origin.z) / ray.direction.z;
        zmax = (maxBounds.z - ray.origin.z) / ray.direction.z;

        if (zmin > zmax)
            std::swap(zmin, zmax);

        if ((tmin > zmax) || (zmin > tmax))
            return hit;

        // Choose the entry and exit points based on the t values
        float t_enter = std::max(std::max(tmin, ymin), zmin);
        float t_exit = std::min(std::min(tmax, ymax), zmax);

        if (t_enter > t_exit || t_exit < 0)
            return hit;

        glm::vec3 intersection_point = ray.origin + t_enter * ray.direction;
        hit.hit = true;
        hit.intersection = intersection_point;
        hit.distance = t_enter;

        // Calculate the normal based on which axis has the maximum overlap
        if (t_enter == tmin) {
            hit.normal = glm::vec3(-1, 0, 0); // Intersection with the left face
        } else if (t_enter == ymin) {
            hit.normal = glm::vec3(0, -1, 0); // Intersection with the bottom face
        } else {
            hit.normal = glm::vec3(0, 0, -1); // Intersection with the front face
        }
        hit.object = this;
        return hit;
    }
};

class bvh_node {
private:
    BoundingBox *boundingBox;
    bvh_node *leftChild;
    bvh_node *rightChild;
    std::vector<Triangle> triangles; // Store triangles in leaf nodes

    std::pair<std::vector<Triangle>, std::vector<Triangle>>
    static splitMesh(std::vector<Triangle> &mesh, int a) {
        std::vector<Triangle> left;
        std::vector<Triangle> right;

        float c = 0;
        for (const Triangle &m: mesh) {
            for (const glm::vec3 &vertex: m.vertices) {
                c += vertex[a];
            }
        }
        c /= mesh.size() * 3;

        for (const Triangle &m: mesh) {
            bool isLeft =
                    false; // Flag to determine if the triangle belongs to the left side

            for (const glm::vec3 &vertex: m.vertices) {
                if (vertex[a] < c) {
                    isLeft = true; // At least one vertex is on the left side
                    break;
                }
            }

            if (isLeft) {
                left.push_back(m);
            } else {
                right.push_back(m);
            }
        }
        return {left, right};
    }

public:
    explicit bvh_node(std::vector<Triangle> &mesh, int a = 0) {
        int maxSize = 100;
        boundingBox = new BoundingBox(mesh);

        if (mesh.size() <= maxSize) {
            leftChild = nullptr;
            rightChild = nullptr;
            triangles = mesh;
        } else {
            std::pair<std::vector<Triangle>, std::vector<Triangle>> objs =
                    splitMesh(mesh, a);
            leftChild = new bvh_node(objs.first, (a + 1) % 3);
            rightChild = new bvh_node(objs.second, (a + 1) % 3);
        }
    }

    std::vector<Triangle> bhv_intersect(bvh_node *node, Ray &ray) {
        // leaf node
        if (node->leftChild == nullptr && node->rightChild == nullptr)
            return node->triangles;

        bool leftHit = node->leftChild->boundingBox->intersect(ray).hit;
        bool rightHit = node->rightChild->boundingBox->intersect(ray).hit;

        if (leftHit && rightHit) {
            std::vector<Triangle> left = bhv_intersect(node->leftChild, ray);
            std::vector<Triangle> right = bhv_intersect(node->rightChild, ray);
            left.insert(left.end(), right.begin(), right.end());
            return left;
        } else if (leftHit) {
            return bhv_intersect(node->leftChild, ray);
        } else if (rightHit) {
            return bhv_intersect(node->rightChild, ray);
        }
        return node->triangles;
    }
};

/*
  Takes an Obj file and parses it, creating a list of Triangles
*/
class MeshLoader : public Object {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Triangle> triangles;
    BoundingBox boundingBox;
    bvh_node *node;

public:
    MeshLoader(const std::string &filename, glm::vec3 translation, bool hasMaterial,
               Material material = Material()) {
        //std::cout << "in meshloader\n";

        if (hasMaterial) {
            this->setMaterial(material);
        }

        std::ifstream file(filename);
        //std::cout << "opening file\n";

        if (!file.is_open()) {
            //std::cout << "Could not open file " << filename << std::endl;
            return;
        }

        std::string line;

        float x, y, z, nx, ny, nz;
        int smoothShading = 0;

        glm::vec3 minBounds = glm::vec3(INFINITY);
        glm::vec3 maxBounds = glm::vec3(-INFINITY);
        //std::cout << "about to read\n";

        while (getline(file, line)) {
            if (line[1] == 'n') {
                // normal
                sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
                normals.emplace_back(x + translation.x, y + translation.y, z + translation.z);
            } else if (line[0] == 'v') {
                // vertex
                sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
                // vertices.push_back(
                //     glm::vec3(x + translation.x, y + translation.y, z +
                //     translation.z));
                glm::vec3 vertex(x + translation.x, y + translation.y,
                                 z + translation.z);
                vertices.push_back(vertex);
                minBounds.x = std::min(minBounds.x, vertex.x);
                minBounds.y = std::min(minBounds.y, vertex.y);
                minBounds.z = std::min(minBounds.z, vertex.z);

                maxBounds.x = std::max(maxBounds.x, vertex.x);
                maxBounds.y = std::max(maxBounds.y, vertex.y);
                maxBounds.z = std::max(maxBounds.z, vertex.z);

            } else if (line[0] == 's') {
                //std::cout << "s\n";

                // smooth shading
                sscanf(line.c_str(), "s %d", &smoothShading);
            } else if (line[0] == 'f') {
                // face
                // if smoothShading == 0, there are no normals
                if (smoothShading == 0) {
                    sscanf(line.c_str(), "f %f %f %f", &x, &y, &z);
                    Triangle triangle =
                            Triangle(vertices[x - 1], vertices[y - 1], vertices[z - 1]);
                    if (hasMaterial) {
                        triangle.setMaterial(material);
                    }
                    triangles.push_back(triangle);
                } else {
                    sscanf(line.c_str(), "f %f//%f %f//%f %f//%f", &x, &nx, &y, &ny, &z,
                           &nz);

                    Triangle triangle =
                            Triangle(vertices[x - 1], vertices[y - 1], vertices[z - 1],
                                     normals[nx - 1], normals[ny - 1], normals[nz - 1]);
                    if (hasMaterial) {
                        triangle.setMaterial(material);
                    }
                    triangles.push_back(triangle);
                }
            }
        }

        file.close();
        //std::cout << "file closed\n";
        boundingBox = BoundingBox(minBounds, maxBounds);
        //std::cout << "bounding box\n";

        node = new bvh_node(triangles);
        //std::cout << "new node\n";

    }

    Hit intersect(Ray &ray) override {
        Hit closest_hit{};

        closest_hit.hit = false;
        closest_hit.distance = INFINITY;
        if (!boundingBox.intersect(ray).hit) {
            return closest_hit;
        }

        for (Triangle &t: node->bhv_intersect(node, ray)) {
            Hit intersection = t.intersect(ray);
            if (intersection.hit && intersection.distance < closest_hit.distance) {
                closest_hit = intersection;
            }
        }

        // for (int k = 0; k < triangles.size(); k++) {
        //   Hit hit = triangles[k].intersect(ray);
        //   if (hit.hit == true && hit.distance < closest_hit.distance) {
        //     // cout << hit.hit << endl;
        //     closest_hit = hit;
        //   }
        // }
        closest_hit.object = this;
        return closest_hit;
    }
};

#endif