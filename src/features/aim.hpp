#pragma once
#include "../utils/math.hpp"

namespace aim {
    inline bool enable_correction = false;
    inline float smoothness = 5.0f;
    inline float field_of_view = 90.0f;

    Vector3 GetNearestTarget();
    void SmoothAim(Vector3 current, Vector3 target, float smooth);
    void Update();
}
