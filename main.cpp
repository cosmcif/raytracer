/**
@file main.cpp
*/

#include "glm/geometric.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/trigonometric.hpp"

#include <ctime>
#include <iostream>
#include <omp.h>
#include <chrono>

#include "Objects.h"
#include "MeshLoader.h"
#include "Image.h"

using namespace std;

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
    for (auto &light: lights) {

        glm::vec3 light_direction =
                glm::normalize(light->position - point);
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

        float att = glm::distance(point, light->position);
        att = 1 / pow(max(0.5f, att), 2);
        color += light->color * (diffuse + specular) * att;
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

    Hit closest_hit{};

    closest_hit.hit = false;
    closest_hit.distance = INFINITY;

    for (auto &object: objects) {
        Hit hit = object->intersect(ray);
        if (hit.hit && hit.distance < closest_hit.distance)
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

    objects.push_back(new MeshLoader("./meshes/armadillo.obj",
                                     glm::vec3(0, -3, 9), true, orange_specular));

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
                                glm::vec3(0.0f, 1.0f, 0.0f), true, blue_copper_specular));
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

    lights.push_back(
            new Light(glm::vec3(0, 26, 5), glm::vec3(130.0))); // top light
    lights.push_back(
            new Light(glm::vec3(0, 1, 12), glm::vec3(15.0))); // floor light
    lights.push_back(new Light(glm::vec3(0, 5, 1), glm::vec3(45.0)));
}

int main(int argc, const char *argv[]) {
    cout << "Running on " << omp_get_max_threads() << " threads\n";

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    int width = 1024; // width of the image
    // int width = 320;
    int height = 768; // height of the image
    // int height = 210;
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

                    pixelColor += trace_ray(ray);
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