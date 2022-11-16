#include "Material.h"

Material::Material() : ka(0.5f), kd(0.5f), ks(0.5f), s(10.0f), a(1.0f) {};
Material::Material(glm::vec3 ka) : ka(ka), kd(0.5f), ks(0.5f), s(10.0f), a(1.0f) {};