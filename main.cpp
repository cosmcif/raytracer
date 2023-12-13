/**
@file main.cpp
*/

#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <iostream>
#include <omp.h>
#include <chrono>

#include "Objects.h"
#include "MeshLoader.h"
#include "Image.h"

using namespace std;

constexpr float EPSILON = 0.001f;

/**
 Light class
 */
class Light {
public:
    glm::vec3 position; ///< Position of the light source
    glm::vec3 color;    ///< Color/intentisty of the light source
    explicit Light(glm::vec3 position) : position(position), color(glm::vec3(1.0)) {}

    Light(glm::vec3 position, glm::vec3 color)
            : position(position), color(color) {}
};

vector<Light *> lights; ///< A list of lights in the scene
glm::vec3 ambient_light(0.7);
vector<Object *> objects; ///< A list of all objects in the scene


bool is_shadowed(glm::vec3 point, glm::vec3 normal, glm::vec3 direction,
                 const float distance) {
    // avoid sending shadow rays towards lights behind the surface
    if (glm::dot(normal, direction) < 0) {
        return true;
    }

    // origin of the shadow ray is moved a little to avoid self intersection
    Ray shadowRay = Ray(point + EPSILON * direction, direction);
    for (Object *object: objects) {
        Hit hit = object->intersect(shadowRay);
        if (hit.hit && hit.distance <= distance) {
            return true;
        }
    }
    return false;
}

Hit closest(Ray ray) {
    Hit closest_hit{};

    closest_hit.hit = false;
    closest_hit.distance = INFINITY;

    for (auto &object: objects) {
        Hit hit = object->intersect(ray);
        if (hit.hit && hit.distance < closest_hit.distance)
            closest_hit = hit;
    }
    return closest_hit;
}

/** Function for computing color of an object according to the Phong Model
 @param point A point belonging to the object for which the color is computer
 @param normal A normal vector the the point
 @param uv Texture coordinates
 @param view_direction A normalized direction from the point to the
 viewer/camera
 @param material A material structure representing the material of the object
*/
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec2 uv,
                     glm::vec3 view_direction, Material material,
                     const int maxBounces) {

    glm::vec3 color(0.0);

    for (Light *light: lights) {
        glm::vec3 light_direction = glm::normalize(light->position - point);
        const float distance_from_light = glm::distance(point, light->position);

        if (!is_shadowed(point, normal, light_direction, distance_from_light)) {

            glm::vec3 diffuse_color =
                    material.texture != nullptr ? material.texture(uv) : material.diffuse;
            const float diffuse = max(0.0f, glm::dot(light_direction, normal));

            glm::vec3 h =
                    glm::normalize(light_direction + view_direction); // half vector
            const float specular =
                    max(0.0f, glm::pow(glm::dot(h, normal), 4 * material.shininess));

            // Attenuation
            const float distance = max(0.1f, distance_from_light);
            const float attenuation = 1.0f / glm::pow(distance, 2);

            color += attenuation * light->color *
                     (diffuse_color * diffuse + material.specular * specular);
        }
    }
    if (maxBounces > 0) {

        glm::vec3 reflection(0.0f);

        if (material.reflection > 0) {
            color *= 1 - material.reflection;
            glm::vec3 reflection_direction = glm::reflect(-view_direction, normal);
            glm::vec3 reflection_position = point + EPSILON * reflection_direction;
            Ray reflection_ray = Ray(reflection_position, reflection_direction);

            Hit closest_hit = closest(reflection_ray);

            if (closest_hit.hit) {
                reflection =
                        material.reflection *
                        PhongModel(closest_hit.intersection, closest_hit.normal,
                                   closest_hit.uv, glm::normalize(-reflection_direction),
                                   closest_hit.object->getMaterial(), maxBounces - 1);
            }
        }

        glm::vec3 refraction(0.0f);

        if (material.refraction > 0) {
            color *= (1 - material.refraction);
            const bool is_entering = glm::dot(normal, -view_direction) < 0.0f;

            const float n1 = is_entering ? 1.0f : material.sigma;
            const float n2 = is_entering ? material.sigma : 1.0f;
            const float eta = n1 / n2;

            glm::vec3 refraction_direction =
                    glm::refract(-view_direction, is_entering ? normal : -normal, eta);
            glm::vec3 refraction_position = point + EPSILON * refraction_direction;

            Ray refraction_ray = Ray(refraction_position, refraction_direction);

            Hit closest_hit = closest(refraction_ray);

            if (closest_hit.hit) {
                refraction =
                        material.refraction *
                        PhongModel(closest_hit.intersection, closest_hit.normal,
                                   closest_hit.uv, glm::normalize(-refraction_direction),
                                   closest_hit.object->getMaterial(), maxBounces - 1);

                float O1 = cos(glm::angle(normal, view_direction));
                float O2 = cos(glm::angle(-normal, refraction_direction));

                float R = 0.5 * (pow((n1 * O1 - n2 * O2) / (n1 * O1 + n2 * O2), 2) +
                                 pow((n1 * O2 - n2 * O1) / (n1 * O2 + n2 * O1), 2));
                float T = 1 - R;

                reflection *= R;
                refraction *= T;
            }
        }
        color += reflection + refraction;
    }

    color += ambient_light * material.ambient;
    return color;
}

/**
 Functions that computes a color along the ray
 @param ray Ray that should be traced through the scene
 @param bounces number of bounces
 @return Color at the intersection point
 */
glm::vec3 trace_ray(Ray ray, int bounces) {
    Hit closest_hit = closest(ray);

    glm::vec3 color(0.0);

    if (closest_hit.hit) {
        color = PhongModel(closest_hit.intersection, closest_hit.normal,
                           closest_hit.uv, glm::normalize(-ray.direction),
                           closest_hit.object->getMaterial(), bounces);
    }
    // clamp the final color to [0,1]
    return glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
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

    new_intensity.x = pow(alpha * pow(intensity.x, beta), oneOverGamma);
    new_intensity.y = pow(alpha * pow(intensity.y, beta), oneOverGamma);
    new_intensity.z = pow(alpha * pow(intensity.z, beta), oneOverGamma);

    glm::vec3 tonemapped =
            min(new_intensity, glm::vec3(1.0)); // tonemapped intensity
    return glm::clamp(tonemapped, glm::vec3(0.0), glm::vec3(1.0));
}

/**
 Function defining the scene
 */
void sceneDefinition() {

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

    Material terrain;
    terrain.texture = &perlinTerrain;

    Material ice;
    ice.texture = &perlinIceTerrain;

    Material glass;
    glass.ambient = glm::vec3(0.0f);
    glass.diffuse = glm::vec3(0.0f);
    glass.specular = glm::vec3(0.0f);
    glass.shininess = 0.0;
    glass.refraction = 1.0f;
    glass.reflection = 1.0f;
    glass.sigma = 2.0f;

    Material mirror;
    mirror.ambient = glm::vec3(0.0f);
    mirror.diffuse = glm::vec3(0.0f);
    mirror.specular = glm::vec3(0.0f);
    mirror.shininess = 0.0;
    mirror.reflection = 1.0f;

    objects.push_back(new MeshLoader("./meshes/crystals.obj",
                                     glm::vec3(0, 0, 9), true, glass));

    // plane in the front
    objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, -0.1f),
                                glm::vec3(0.0f, 0.0f, 1.0f), true, blue_copper_specular));
    // plane in the back
    objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, 30.0f),
                                glm::vec3(0.0f, 0.0f, -1.0f), true, blue_copper_specular));

    // plane on the left
    objects.push_back(new Plane(glm::vec3(-15.0f, 12.0f, 14.995f),
                                glm::vec3(1.0f, 0.0f, 0.0f), true, blue_copper_specular));
    // plane on the right
    objects.push_back(new Plane(glm::vec3(15.0f, 12.0f, 14.995f),
                                glm::vec3(-1.0f, 0.0f, 0.0f), true, blue_copper_specular));

    // plane on bottom
    objects.push_back(new Plane(glm::vec3(0.0f, -3.0f, 14.995f),
                                glm::vec3(0.0f, 1.0f, 0.0f), true, ice));
    // plane on top
    objects.push_back(new Plane(glm::vec3(0.0f, 27.0f, 14.995f),
                                glm::vec3(0.0f, -1.0f, 0.0f), true, blue_copper_specular));

/*
    for (int x = -20; x <= 20; x += 2) {
        double z = 20 + sqrt(100 - 0.3075 * x * x);

        if (z < 20.0f || z > 30.0f) {
            continue;
        }

        Cone *c = new Cone(blue_copper_specular);

        glm::mat4 transformationMatrix =
                glm::translate(glm::vec3(x, 5.0f, z)) *
                glm::rotate(glm::radians(180.0f), glm::vec3(0, 0, 1)) *
                glm::scale(glm::vec3(5.0f, 20.0f, 1.0f));

        c->setTransformation(transformationMatrix);
        objects.push_back(c);

        Cone *c2 = new Cone(blue_copper_specular);

        glm::mat4 transformationMatrix2 =
                glm::translate(glm::vec3(x, 10.0f, z)) *
                glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) *
                glm::scale(glm::vec3(5.0f, 20.0f, 1.0f));

        c2->setTransformation(transformationMatrix2);
        objects.push_back(c2);
    }
    */

    auto *redSphere = new Sphere(terrain);
    redSphere->setTransformation(glm::translate(glm::vec3(3, -2, 6)) *
                                 glm::scale(glm::vec3(1)));
    objects.push_back(redSphere);


    auto *texturedSphere = new Sphere(ice);
    glm::mat4 texturedMatrix = glm::translate(glm::vec3(-6, 4, 23)) * glm::scale(glm::vec3(7.0));
    texturedSphere->setTransformation(texturedMatrix);
    objects.push_back(texturedSphere);

    auto *glassSphere = new Sphere(glass);
    glm::mat4 glassMatrix = glm::translate(glm::vec3(-4,-1,7)) * glm::scale(glm::vec3(2.0));
    glassSphere->setTransformation(glassMatrix);
    objects.push_back(glassSphere);

    auto *mirrorSphere = new Sphere(mirror);
    glm::mat4 mirrorMatrix = glm::translate(glm::vec3(4, -2, 14)) * glm::scale(glm::vec3(1.0));
    mirrorSphere->setTransformation(mirrorMatrix);
    objects.push_back(mirrorSphere);
    lights.push_back(
            new Light(glm::vec3(0, 26, 5), glm::vec3(130.0))); // top light
    lights.push_back(
            new Light(glm::vec3(0, 1, 12), glm::vec3(15.0))); // floor light
    lights.push_back(new Light(glm::vec3(0, 5, 1), glm::vec3(45.0)));
}

int main(int argc, const char *argv[]) {
    cout << "Running on " << omp_get_max_threads() << " threads\n";

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    int width = /*320 1024 2048*/ 1024; // width of the image
    int height = /*210 768 1536*/ 768; // height of the image
    float fov = 90; // field of view

    sceneDefinition(); // Let's define a scene

    Image image(width, height); // Create an image where we will store the result

    const float s = 2 * tan(0.5 * fov / 180 * M_PI) / width;
    const float X = -s * width / 2;
    const float Y = s * height / 2;

    // Define tiles for parallelization
    // Tiles are a good way to parallelize ray tracing since we can expect that rays in the same tile will behave similarly (i.e. they will hit the same objects).
    const int tile_size = 16;
    const int tiles_x = (width + tile_size - 1) / tile_size;   // add one tile if width is not a multiple of tile_size
    const int tiles_y = (height + tile_size - 1) / tile_size;  // add one tile if height is not a multiple of tile_size
    const int tile_count = tiles_x * tiles_y;
    glm::vec3 origin(0, 0, 0);


    float jitterMatrix[4 * 2] = {
            -1.0 / 4.0, 3.0 / 4.0,
            3.0 / 4.0, 1.0 / 3.0,
            -3.0 / 4.0, -1.0 / 4.0,
            1.0 / 4.0, -3.0 / 4.0,
    };
#pragma omp parallel for schedule(dynamic, 1)
    for (int tile = 0; tile < tile_count; tile++) {
        if (omp_get_thread_num() == 0) {
            cout << "Progress: " << ceil((float) tile / tile_count * 10000) / 100 << "%\r";
            cout.flush();
        }
        // Compute the tile coordinates
        const int tile_j = tile / tiles_x;                             // the tile column number
        const int tile_i = tile - tile_j * tiles_x;                    // the tile row number
        const int tile_i_start = tile_i * tile_size;                   // the x coordinate of the tile
        const int tile_j_start = tile_j * tile_size;                   // the y coordinate of the tile
        const int tile_i_end = min(tile_i_start + tile_size, width);   // the x coordinate of the tile + tile_size
        const int tile_j_end = min(tile_j_start + tile_size, height);  // the y coordinate of the tile + tile_size

        for (int i = tile_i_start; i < tile_i_end; i++)
            for (int j = tile_j_start; j < tile_j_end; j++) {
                glm::vec3 pixelColor(0.0f);

                // super sampling anti aliasing
                for (int sample = 0; sample < 4; ++sample) {
                    float jitterX = jitterMatrix[2 * sample];
                    float jitterY = jitterMatrix[2 * sample + 1];

                    float dx = X + (i + jitterX) * s + s / 2;
                    float dy = Y - (j + jitterY) * s - s / 2;
                    float dz = 1;

                    // Create ray
                    glm::vec3 direction(dx, dy, dz);
                    direction = glm::normalize(direction);

                    Ray ray(origin, direction);

                    pixelColor += trace_ray(ray, 3);
                }

                pixelColor /= 4.0f;
                image.setPixel(i, j, toneMapping(pixelColor));
            }

    }
    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(end - start);
    cout << "It took " << time_span.count() << " seconds to render the image." << endl;

    // Writing the final results of the rendering
    if (argc == 2) {
        image.writeImage(argv[1]);
    } else {
        image.writeImage("./result.ppm");
    }

    return 0;
}