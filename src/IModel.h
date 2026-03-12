#include <string>

class Shader;

class IModel
{
  public:
    virtual void load(const std::string& i_path) = 0;
    virtual void draw(const Shader& i_shader) = 0;
    virtual void transform(const float& i_timeInSeconds) = 0;
};