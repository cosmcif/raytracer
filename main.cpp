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
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec3 normalShading, glm::vec2 uv,
                     glm::vec3 view_direction, Material material,
                     const int maxBounces, Hit hit) {

    glm::vec3 color(0.0);

    // flip normal if it is pointing away from the view direction
    if (glm::dot(normal, view_direction) < 0) {
        normal = -normal;
    }
    // flip shading normal if it is pointing away from the view direction
    if (glm::dot(normalShading, view_direction) < 0) {
        normalShading = -normalShading;
    }

    for (Light *light: lights) {
        glm::vec3 light_direction = glm::normalize(light->position - point);
        const float distance_from_light = glm::distance(point, light->position);

        if (!is_shadowed(point, normal, light_direction, distance_from_light)) {

            glm::vec3 diffuse_color =
                    material.texture != nullptr ? material.texture(uv) : material.diffuse;
            const float diffuse = max(0.0f, glm::dot(light_direction, normalShading));

            glm::vec3 h =
                    glm::normalize(light_direction + view_direction); // half vector

            const float distance = max(0.1f, distance_from_light);
            const float attenuation = 1.0f / glm::pow(distance, 2);
            glm::vec3 diffusion = attenuation * light->color * diffuse_color * diffuse;

            glm::vec3 specular_term = glm::vec3(0.0f); // Initialize to zero
            if (material.isAnisotropic) {
                //cout << "anis" <<endl;
                float NdotL = glm::dot(normalShading, light_direction);
                float NdotV = glm::dot(normalShading, view_direction);

                if (NdotL > 0 && NdotV > 0) {
                    float HdotTangent = glm::dot(h, hit.tangent);
                    float HdotBitangent = glm::dot(h, hit.bitangent);
                    float HdotN = glm::dot(h, normalShading);

                    float exponent = -2.0f * (glm::pow((HdotTangent / material.alpha_x), 2.0f)
                                              * glm::pow((HdotBitangent / material.alpha_y), 2.0f)) / (1 + HdotN);

                    specular_term = (material.specular * NdotL * exp(exponent)) /
                                    (sqrt(NdotL * NdotV) * 4 * glm::pi<float>() * material.alpha_x * material.alpha_y);
                }


                //glm::vec3 TBNnormal = glm::normalize(normalShading);
                //glm::vec3 X = glm::normalize(glm::cross(normalShading, TBNnormal));


                /*glm::vec3 halfway = glm::normalize(light_direction + view_direction);

                // Anisotropic attenuation factors
                float cos_theta_h = glm::dot(halfway, normalShading);
                float cos_theta_h_squared = cos_theta_h * cos_theta_h;
                float sin_theta_h_squared = glm::max(0.0f, 1.0f - cos_theta_h_squared);
                float tan_theta_h_x_squared = sin_theta_h_squared / (cos_theta_h_squared + material.alpha_x * material.alpha_x);
                float tan_theta_h_y_squared = sin_theta_h_squared / (cos_theta_h_squared + material.alpha_y * material.alpha_y);

                // Anisotropic specular distribution
                float exp_x = glm::exp(-tan_theta_h_x_squared / (2.0f * material.alpha_x * material.alpha_x));
                float exp_y = glm::exp(-tan_theta_h_y_squared / (2.0f * material.alpha_y * material.alpha_y));
                float D = exp_x * exp_y / (glm::pi<float>() * material.alpha_x * material.alpha_y * cos_theta_h_squared * cos_theta_h_squared);

                // Anisotropic specular reflection
                float fresnel = glm::pow(1.0f - glm::dot(light_direction, halfway), 5.0f);
                glm::vec3 specular_reflection = material.specular * fresnel * D / (4.0f * glm::dot(normalShading, light_direction));

                specular_term = attenuation * light->color * specular_reflection;
                */
            } else {
                const float specular = max(0.0f, glm::pow(glm::dot(h, normalShading), 4 * material.shininess));
                specular_term = attenuation * light->color * material.specular * specular;
            }
            // Attenuation

            color += diffusion + specular_term;
        }
    }
    if (maxBounces > 0) {

        glm::vec3 reflection(0.0f);

        if (material.reflection > 0) {
            color *= 1 - material.reflection;
            glm::vec3 reflection_direction = glm::reflect(-view_direction, normalShading);
            glm::vec3 reflection_position = point + EPSILON * reflection_direction;
            Ray reflection_ray = Ray(reflection_position, reflection_direction);

            Hit closest_hit = closest(reflection_ray);

            if (closest_hit.hit) {
                reflection =
                        material.reflection *
                        PhongModel(closest_hit.intersection, closest_hit.normal, closest_hit.normalShading,
                                   closest_hit.uv, glm::normalize(-reflection_direction),
                                   closest_hit.object->getMaterial(), maxBounces - 1, closest_hit);
            }
        }

        glm::vec3 refraction(0.0f);

        if (material.refraction > 0) {
            color *= (1 - material.refraction);
            const bool is_entering = glm::dot(normalShading, -view_direction) < 0.0f;

            const float n1 = is_entering ? 1.0f : material.sigma;
            const float n2 = is_entering ? material.sigma : 1.0f;
            const float eta = n1 / n2;

            glm::vec3 refraction_direction =
                    glm::refract(-view_direction, is_entering ? normalShading : -normalShading, eta);
            glm::vec3 refraction_position = point + EPSILON * refraction_direction;

            Ray refraction_ray = Ray(refraction_position, refraction_direction);

            Hit closest_hit = closest(refraction_ray);

            if (closest_hit.hit) {
                refraction =
                        material.refraction *
                        PhongModel(closest_hit.intersection, closest_hit.normal, closest_hit.normalShading,
                                   closest_hit.uv, glm::normalize(-refraction_direction),
                                   closest_hit.object->getMaterial(), maxBounces - 1, closest_hit);

                float O1 = cos(glm::angle(normalShading, view_direction));
                float O2 = cos(glm::angle(-normalShading, refraction_direction));

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
        color = PhongModel(closest_hit.intersection, closest_hit.normal, closest_hit.normalShading,
                           closest_hit.uv, glm::normalize(-ray.direction),
                           closest_hit.object->getMaterial(), bounces, closest_hit);
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
    orange_specular.specular = glm::vec3(0.33);
    orange_specular.shininess = 10.0;

    Material blue_copper_specular;
    blue_copper_specular.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    blue_copper_specular.diffuse = glm::vec3(0.2f, 0.8f, 0.8f);
    blue_copper_specular.specular = glm::vec3(0.6);
    blue_copper_specular.shininess = 100.0;

    Material terrain;
    terrain.texture = &perlinTerrain;

    Material normal;
    //normal.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    //normal.diffuse = glm::vec3(0.2f, 0.8f, 0.8f);
    //normal.texture = &perlinNormal;
    normal.hasNormalMap = true;
    normal.normalMap = &perlinNormal;
    normal.refraction = 1.0f;
    normal.reflection = 0.5f;
    normal.sigma = 2.0f;


    Material water;
    //water.hasNormalMap = true;
    //water.normalMap = &perlinWater;
    //water.refraction = 1.0f;
    //water.reflection = 0.5f;
    //water.sigma = 2.0f;
    water.alpha_x = 0.7f;
    water.alpha_y = 0.3f;
    water.isAnisotropic = true;
    water.shininess = 0.6f;
    water.specular = glm::vec3(0.2f, 0.8f, 0.8f);
    water.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    water.diffuse = glm::vec3(0.2f, 0.8f, 0.8f);



    Material ice;
    ice.texture = &perlinIceTerrain;
    ice.refraction = 0.5f;
    ice.reflection = 0.5f;
    ice.sigma = 2.0f;

    Material perla;
    perla.texture = &opal;
    perla.shininess = 0.9;
    perla.refraction = 0.8f;
    perla.reflection = 0.1f;
    perla.sigma = 2.0f;

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

    //objects.push_back(new MeshLoader("./meshes/bunny.obj",
    //                                 glm::vec3(0, -3, 9), true, glass));

    // plane in the front
    objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, -0.1f),
                                glm::vec3(0.0f, 0.0f, 1.0f), true, blue_copper_specular));
    // plane in the back
    objects.push_back(new Plane(glm::vec3(0.0f, 12.0f, 30.0f),
                                glm::vec3(0.0f, 0.0f, -1.0f), true, orange_specular));

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

    /*auto *redSphere = new Sphere(terrain);
    redSphere->setTransformation(glm::translate(glm::vec3(5, -2, 7)) *
                                 glm::scale(glm::vec3(1)));
    objects.push_back(redSphere);

      auto *mirrorSphere = new Sphere(perla);
    glm::mat4 mirrorMatrix = glm::translate(glm::vec3(4, 2, 14)) * glm::scale(glm::vec3(1.0));
    mirrorSphere->setTransformation(mirrorMatrix);
    objects.push_back(mirrorSphere);

    auto *texturedSphere = new Sphere(ice);
    glm::mat4 texturedMatrix = glm::translate(glm::vec3(-6, 4, 23)) * glm::scale(glm::vec3(7.0));
    texturedSphere->setTransformation(texturedMatrix);
    objects.push_back(texturedSphere);
    */

    auto *glassSphere = new Sphere(water);
    glm::mat4 glassMatrix = glm::translate(glm::vec3(-5, -1, 12)) * glm::scale(glm::vec3(2.0));
    glassSphere->setTransformation(glassMatrix);
    objects.push_back(glassSphere);

    auto *glass2Sphere = new Sphere(water);
    glass2Sphere->setTransformation(glm::translate(glm::vec3(5, -1, 12)) * glm::scale(glm::vec3(2.0)));
    objects.push_back(glass2Sphere);


    lights.push_back(
            new Light(glm::vec3(0, 26, 5), glm::vec3(130.0))); // top light
    lights.push_back(
            new Light(glm::vec3(0, 1, 12), glm::vec3(15.0))); // floor light
    lights.push_back(new Light(glm::vec3(0, 5, 1), glm::vec3(45.0)));
}

void kyuremScene() {

    Material normal;
    //normal.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    //normal.diffuse = glm::vec3(0.2f, 0.8f, 0.8f);
    //normal.texture = &perlinNormal;
    normal.hasNormalMap = true;
    normal.normalMap = &perlinNormal;
    normal.refraction = 1.0f;
    normal.reflection = 0.5f;
    normal.sigma = 2.0f;


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

    Material grey;
    grey.ambient = glm::vec3(0.07f, 0.07f, 0.07f);
    grey.diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
    grey.specular = glm::vec3(0.6);
    grey.shininess = 100.0;

    Material terrain;
    terrain.texture = &perlinTerrain;

    Material ice;
    ice.texture = &perlinIceTerrain;
    ice.refraction = 0.3f;
    ice.reflection = 0.5f;
    ice.sigma = 2.0f;

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

    objects.push_back(new MeshLoader("./meshes/corridoio_leggero.obj",
                                     glm::vec3(0.3, -1, 0), true, terrain));

    objects.push_back(new MeshLoader("./meshes/kyurem_ice.obj",
                                     glm::vec3(-0.5, 0.1, 1), true, ice));
    objects.push_back(new MeshLoader("./meshes/kyurem_body.obj",
                                     glm::vec3(-0.5, 0.1, 1), true, grey));

    objects.push_back(new Plane(glm::vec3(0.0f, -1.0f, 14.995f),
                                glm::vec3(0.0f, 1.0f, 0.0f), true, ice));

    lights.push_back(
            new Light(glm::vec3(12, 26, -5), glm::vec3(130.0))); // top light
    lights.push_back(new Light(glm::vec3(3, 10, 0), glm::vec3(100.0)));


    /*
     *     glm::vec3 origin(-0.45, 0.5, 1.4); // z smaller value -> it goes forward
     *     float xTiltAngle = -1.0; // Adjust this value as needed
     *     float yTiltAngle = 0.4;
     *     glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), xTiltAngle, glm::vec3(1.0f, 0.0f, 0.0f));
     *     rotationMatrix = glm::rotate(rotationMatrix, yTiltAngle, glm::vec3(0.0f, 1.0f, 0.0f));
     */
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
    glm::vec3 origin(0.0);
    //glm::vec3 origin(-0.45, 0.5, 1.4); // z smaller value -> it goes forward
    // float xTiltAngle = -1.0; // Adjust this value as needed
    // float yTiltAngle = 0.4;
    // glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), xTiltAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    // rotationMatrix = glm::rotate(rotationMatrix, yTiltAngle, glm::vec3(0.0f, 1.0f, 0.0f));

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


                    // Tilt the camera down by adjusting the pitch angle
                    glm::vec4 direction4(dx, dy, dz, 0.0f);
                    // direction4 = rotationMatrix * direction4;

                    // Normalize the direction vector
                    glm::vec3 direction = glm::normalize(glm::vec3(direction4));


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