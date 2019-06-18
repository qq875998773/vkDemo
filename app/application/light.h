#pragma once

#include <vector>

#include "entity.h"

namespace vv
{
    class Light : public Entity
    {
    public:
        glm::vec4 irradiance;
        float radius;

        Light();
        ~Light();

        /*
         * Creates a point light.
         */
        void create(glm::vec4 irradiance, float radius);

        /*
         *
         */
        void shutDown();

    private:
    };
}