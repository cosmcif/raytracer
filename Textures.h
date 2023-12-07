#ifndef Textures_h
#define Textures_h
#include <iostream>

#include "glm/fwd.hpp"
#include "glm/glm.hpp"

glm::vec3 checkerboardTexture(glm::vec2 uv) {
  float n = 50;
  float f = int(floor(n * uv.s) + floor(n * uv.t)) % 2;
  return glm::vec3(f);
}
glm::vec3 rainbowTexture(glm::vec2 uv) {
  float n = 40;
  int value = int(floor(n * uv.y + 0.5 * n * uv.x)) % 3;
  switch (value) {
  case 0:
    return glm::vec3(1.0, 0.0, 0.0); // Red
  case 1:
    return glm::vec3(0.0, 1.0, 0.0); // Green
  default:
    return glm::vec3(0.0, 0.0, 1.0); // Blue
  }
}

/* EXTRA EXTRA STUFF
 * The following code wasn't requested by the assignment, it
 * was created simply to have fun and experiment with textures.
 *
 * This section contains:
 *
 * - Color definition
 *
 * - Auxiliary functions
 *   - ellipseFormula
 *   - eyesAuxiliary
 *   - edgeEquation
 *   - triangleAuxiliary
 *   - crossAuxiliary
 *
 * - Textures
 *   - polandballTexture
 *   - donatelloTexture
 *   - calabriaTexture
 */

/* COLOR DEFINITION */
const glm::vec3 BLACK(0.0, 0.0, 0.0);
const glm::vec3 BLUE(0.04, 0.31, 0.64);
const glm::vec3 DARK_GREEN(0.3, 0.4, 0.1);
const glm::vec3 GREEN(0.0, 1.0, 0.0);
const glm::vec3 LIGHT_BLUE(0.0f, 0.7f, 1.0f);
const glm::vec3 PURPLE(0.4, 0.0, 0.5);
const glm::vec3 RED(1.0, 0.0, 0.0);
const glm::vec3 WHITE(1.0, 1.0, 1.0);
const glm::vec3 YELLOW(1.0f, 0.9f, 0.2f);
// glm::vec3(0.6, 0.2, 0.9); // purple
// glm::vec3(0.3, 0.3, 0.0); // Green

/* AUXILIARY FUNCTIONS */

/**
 * Use the ellipse formula to calculate if a point is in it.
 * NOTE: on the mesh, the ellipse looks like a circle in some
 * cases. Also, for a = b = 1, this becomes a generalized
 * formula for circles.
 *
 * @param uv point coordinates
 * @param c  ellipse's center's coordinates
 * @param a  axis length
 * @param b  the other axis length
 * @return   the result of the formula
 */
float ellipseFormula(glm::vec2 uv, glm::vec2 c, float a, float b) {
  return pow((uv.x - c.x), 2) / a + pow((uv.y - c.y), 2) / b;
}

/**
 * Create two eyes.
 *
 * This function is useful as there are multiple textures with
 * "eyes", so the code would be the same everytime.
 *
 * @param uv point coordinates
 * @return an integer representing the appearance of the eye:
 * - 0: the point is in the inner white part of the eye
 * - 1: the point is in the black outline of the eye
 * - 2: the point is not in the eye
 */
int eyesAuxiliary(glm::vec2 uv) {
  float a = 4.0;
  float b = 1.0;

  // black outline
  float black_radius = 0.001;

  // white inside
  float white_radius = 0.0005;

  float circleLeft = ellipseFormula(uv, glm::vec2(0.6, 0.25), a, b);
  float circleRight = ellipseFormula(uv, glm::vec2(0.6, 0.35), a, b);

  if (circleLeft < white_radius && circleLeft > 0 ||
      circleRight < white_radius && circleRight > 0) {
    return 0;
  } else if (circleLeft < black_radius && circleLeft > 0 ||
             circleRight < black_radius && circleRight > 0) {
    return 1;
  }
  return 2;
}

/**
 * Compute the edge equation for a point uv with respect to a line segment
 * defined by a and b.
 *
 * @param a first point of the line segment
 * @param b second point of the line segment
 * @param uv point for which the edge equation is calculated
 * @return result of the edge equation:
 * - > 0: uv is on the left side
 * - < 0: uv is on the right side
 * - = 0: uv is collinear
 */
float edgeEquation(glm::vec2 a, glm::vec2 b, glm::vec2 uv) {
  return (uv.x - a.x) * (b.y - a.y) - (uv.y - a.y) * (b.x - a.x);
}

/**
 * Determines if a point uv is inside a triangle defined by vertices a, b and c.
 *
 * Calculates the barycentric coords w0, w1, and w2 for the point uv with
 * respect to the triangle formed by a, b, and c. It then checks if uv is inside
 * it.
 *
 * If w0, w1, and w2 are all greater than or equal to zero, the point is inside
 * the triangle. Otherwise, it is outside the triangle.
 *
 * @param a first vertex of the triangle
 * @param b second vertex of the triangle
 * @param c third vertex of the triangle
 * @param uv point to be tested
 * @return 1 if the point is inside the triangle, 0 if it's outside the triangle
 */
int triangleAuxiliary(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 uv) {
  float w0 = edgeEquation(b, c, uv);
  float w1 = edgeEquation(c, a, uv);
  float w2 = edgeEquation(a, b, uv);

  if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
    return 1; // inside the triangle
  } else {
    return 0; // outside the triangle
  }
}

/**
 * Creates two black crosses on white background. Specific function for
 * calabriaTexture.
 *
 * @param uv the point coordinates
 * @return integer representing the appearance point:
 * - 0: the point is inside the white part of the symbol, which cancels the
 * cross
 * - 1: the point is inside the black part of the symbol, which represents the
 * cross
 * - 2: the point has nothing to do with the cross
 */
int crossAuxiliary(glm::vec2 uv) {

  float a = 4.0;
  float b = 1.0;

  // left side
  float blackLeft = ellipseFormula(uv, glm::vec2(0.45, 0.265), a, b);
  float whiteLeft1 = ellipseFormula(uv, glm::vec2(0.423, 0.251), a, b);
  float whiteLeft2 = ellipseFormula(uv, glm::vec2(0.477, 0.251), a, b);
  float whiteLeft3 = ellipseFormula(uv, glm::vec2(0.423, 0.279), a, b);
  float whiteLeft4 = ellipseFormula(uv, glm::vec2(0.477, 0.279), a, b);

  // right side
  float blackRight = ellipseFormula(uv, glm::vec2(0.45, 0.335), a, b);
  float whiteRight1 = ellipseFormula(uv, glm::vec2(0.423, 0.321), a, b);
  float whiteRight2 = ellipseFormula(uv, glm::vec2(0.477, 0.321), a, b);
  float whiteRight3 = ellipseFormula(uv, glm::vec2(0.423, 0.349), a, b);
  float whiteRight4 = ellipseFormula(uv, glm::vec2(0.477, 0.349), a, b);

  // black cross
  float black_radius = 0.0003;
  // hite canceling
  float white_radius = 0.0001;

  if (whiteLeft1 < white_radius && whiteLeft1 > 0 ||
      whiteLeft2 < white_radius && whiteLeft2 > 0 ||
      whiteLeft3 < white_radius && whiteLeft3 > 0 ||
      whiteLeft4 < white_radius && whiteLeft4 > 0 ||
      whiteRight1 < white_radius && whiteRight1 > 0 ||
      whiteRight2 < white_radius && whiteRight2 > 0 ||
      whiteRight3 < white_radius && whiteRight3 > 0 ||
      whiteRight4 < white_radius && whiteRight4 > 0) {
    return 0;
  } else if (blackLeft < black_radius && blackLeft > 0 ||
             blackRight < black_radius && blackRight > 0) {
    return 1;
  }
  return 2;
}

/* TEXTURES */

glm::vec3 polandballTexture(glm::vec2 uv) {
  int value = round(uv.x); // greater than half: red, else white

  int eyes = eyesAuxiliary(uv); // 0 = white, 1 = black

  switch (eyes) {
  case 0: // white
    value = 0;
    break;
  case 1: // black
    value = 2;
    break;
  }

  switch (value) {
  case 0:
    return WHITE; // White
  case 1:
    return RED; // Red
  default:
    return BLACK; // Black
  }
}

glm::vec3 donatelloTexture(glm::vec2 uv) {
  int value = 3;

  if (uv.x < 0.7 && uv.x > 0.5) {
    value = 2; // purple bandana
  }

  int eyes = eyesAuxiliary(uv); // 0 = white, 1 = black, 2 = otherwise
  if (eyes != 2) {
    value = eyes;
  }

  switch (value) {
  case 0:
    return WHITE;
  case 1:
    return BLACK;
  case 2:
    return PURPLE;
  default:
    return DARK_GREEN;
  }
}

glm::vec3 calabriaTexture(glm::vec2 uv) {
  int value = 0;
  float r2 = 0.005;
  float a2 = 4.0;
  float b2 = 0.75;

  // float circle = pow((uv.x - 0.45), 2) / a2 + pow((uv.y - 0.3), 2) / b2;
  // big circle
  float circle = ellipseFormula(uv, glm::vec2(0.45, 0.3), a2, b2);
  if (circle < r2 && circle > 0) {
    value = 1;
  }

  // top triangle
  glm::vec2 a = glm::vec2(0.57, 0.27);
  glm::vec2 b = glm::vec2(0.45, 0.3);
  glm::vec2 c = glm::vec2(0.57, 0.33);

  // bottom triangle
  glm::vec2 d = glm::vec2(0.33, 0.27);
  glm::vec2 e = glm::vec2(0.33, 0.33);
  // glm::vec2 f = glm::vec2(0.45, 0.3); same as b

  if (triangleAuxiliary(a, b, c, uv) || triangleAuxiliary(d, e, b, uv)) {
    value = 2;
  }

  // tree / green triangle
  glm::vec2 g = glm::vec2(0.49, 0.29);
  glm::vec2 h = glm::vec2(0.49, 0.31);
  glm::vec2 i = glm::vec2(0.55, 0.3);
  if (triangleAuxiliary(g, h, i, uv)) {
    value = 4;
  }

  int cross = crossAuxiliary(uv); // 0 = white, 1 = black
  switch (cross) {
  case 0: // white
    value = 1;
    break;
  case 1: // black
    value = 3;
    break;
  }

  float a_2 = 4.0;
  float b_2 = 1.0;
  float r_2 = 0.0001;

  // float c2 = pow((uv.x - 0.38), 2) / a_2 + pow((uv.y - 0.3), 2) / b_2;
  // blue circle / simplified column
  float c2 = ellipseFormula(uv, glm::vec2(0.38, 0.3), a_2, b_2);
  if (c2 < r_2 && c2 > 0) {
    value = 5;
  }

  int eyes = eyesAuxiliary(uv); // 0 = white, 1 = black
  switch (eyes) {
  case 0: // white
    value = 1;
    break;
  case 1: // black
    value = 3;
    break;
  }

  switch (value) {
  case 1:
    return WHITE; // White
  case 2:
    return YELLOW; // yellow
  case 3:
    return BLACK; // Black
  case 4:
    return GREEN; // Green
  case 5:
    return LIGHT_BLUE; // light blue
  default:
    return BLUE; // blue
  }
}
#endif /* Textures_h */