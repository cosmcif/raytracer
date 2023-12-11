//
// Created by sofi on 11/12/23.
//

#ifndef RAYTRACER_LIGHTS_H
#define RAYTRACER_LIGHTS_H

/**
 * @brief Base class representing a generic light source.
 */
class Light {
public:
    glm::vec3 position; ///< Position of the light source
    glm::vec3 color;    ///< Color/intensity of the light source

    /**
     * @brief Constructor for initializing the light with a position.
     * @param position The position of the light source.
     */
    explicit Light(glm::vec3 position) : position(position), color(glm::vec3(1.0)) {}

    /**
     * @brief Constructor for initializing the light with a position and color.
     * @param position The position of the light source.
     * @param color The color/intensity of the light source.
     */
    Light(glm::vec3 position, glm::vec3 color)
            : position(position), color(color) {}

    virtual ~Light() = default;

};

/**
 * @brief Derived class representing an area light source.
 * Inspired from http://raytracerchallenge.com/bonus/area-light.html
 */

class AreaLight : public Light {
public:
    glm::vec3 position; ///< Position of one corner of the light source
    glm::vec3 uvec;   ///< Vector for the u edge (direction and length)
    int usteps;       ///< Number of sections along the u edge
    glm::vec3 vvec;   ///< Vector for the v edge (direction and length)
    int vsteps;       ///< Number of sections along the v edge

    /**
     * @brief Constructor for the AreaLight class.
     * @param position Position of one corner of the light source.
     * @param uvec Vector for the u edge (direction and length).
     * @param usteps Number of sections along the u edge.
     * @param vvec Vector for the v edge (direction and length).
     * @param vsteps Number of sections along the v edge.
     * @param color Color/intensity of the light source.
     *
     *  _ _ _ _
     * |_|_|_|_| vvec
     * |_|_|_|_|  ^
     * |_|_|_|_|  |
     * o_|_|_|_|
     * uvec ->
     * o -> corner
     */
    AreaLight(glm::vec3 position, glm::vec3 uvec, int usteps, glm::vec3 vvec, int vsteps, glm::vec3 color)
            : Light(position, color), position(position), uvec(uvec), usteps(usteps), vvec(vvec), vsteps(vsteps) {}

    /**
     * @brief Calculates the size of each cell in the area light grid.
     * @return Size of each cell as a vector (width, height, depth).
     */
    glm::vec3 getCellSize() const {
        return glm::vec3(uvec.x / usteps, uvec.y / usteps, uvec.z / usteps);
    }

    /**
     * @brief Gets the total number of cells in the area light grid.
     * @return Total number of cells in the area light.
     */
    int getTotalCells() const {
        return usteps * vsteps;
    }

    glm::vec3 point_on_light(int u, int v) const {
        return position + (u + glm::linearRand(0.0f, 1.0f)) * uvec +
               (v + glm::linearRand(0.0f, 1.0f)) * vvec;
    }

};

#endif //RAYTRACER_LIGHTS_H
