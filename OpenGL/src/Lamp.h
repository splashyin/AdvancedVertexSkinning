#pragma once
#include "Shader.h"
#include <glm.hpp>

using namespace glm;

class Lamp {
public:
  // Constructor
  Lamp(const vec3 &i_position, const vec3 &i_color);

  Lamp(const Lamp &) = delete;
  Lamp(Lamp &&) = delete;

  // Render the lamp
  void Draw(Shader *i_shader);

  vec3 getPosition();
  vec3 getColor();

private:
  void setupLight();

  vec3 m_position;
  vec3 m_color;

  unsigned int m_lightVAO = 0;
  unsigned int m_lightVBO = 0;
};