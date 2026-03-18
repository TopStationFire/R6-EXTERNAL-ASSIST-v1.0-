#pragma once
#include "../utils/math.hpp"
#include <d3d11.h>

namespace visuals {
    inline bool enable_boxes = true;
    inline bool enable_skeleton = true;
    inline bool enable_health = true;
    inline bool enable_names = true;

    struct EntityInfo {
        bool valid;
        float health;
        char name[64];
        Vector3 position;
        Vector3 head_position;
        Vector3 bone_positions[20];
    };

    void RenderESP(ID3D11DeviceContext* context, ID3D11RenderTargetView* target);
    void DrawBox(float x, float y, float w, float h, float* color);
    void DrawSkeleton(EntityInfo& entity, float* color);
    void DrawHealthBar(float x, float y, float health);
}
