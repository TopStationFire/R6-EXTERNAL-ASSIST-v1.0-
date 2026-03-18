#include "aim.hpp"
#include "../core/memory.hpp"
#include <cmath>

namespace aim {

constexpr uintptr_t OFFSET_LOCAL_PLAYER = 0x5A0B8D0;
constexpr uintptr_t OFFSET_VIEW_ANGLES = 0x2C0;
constexpr uintptr_t OFFSET_CAMERA_POSITION = 0x2D0;

Vector3 GetNearestTarget() {
    uintptr_t local_player = memory::Read<uintptr_t>(memory::module_base + OFFSET_LOCAL_PLAYER);
    if (!local_player) return Vector3{ 0, 0, 0 };

    Vector3 camera_pos = memory::Read<Vector3>(local_player + OFFSET_CAMERA_POSITION);
    Vector3 current_angles = memory::Read<Vector3>(local_player + OFFSET_VIEW_ANGLES);
    
    uintptr_t entity_list = memory::Read<uintptr_t>(memory::module_base + 0x5A0B7C8);
    if (!entity_list) return Vector3{ 0, 0, 0 };

    int local_team = memory::Read<int>(local_player + 0x148);
    
    Vector3 nearest_angles{ 0, 0, 0 };
    float nearest_dist = field_of_view;
    
    for (int i = 0; i < 64; i++) {
        uintptr_t entity = memory::Read<uintptr_t>(entity_list + (i * sizeof(uintptr_t)));
        if (!entity || entity == local_player) continue;

        int team = memory::Read<int>(entity + 0x148);
        if (team == local_team) continue;

        float health = memory::Read<float>(entity + 0x140);
        if (health <= 0) continue;

        Vector3 head = memory::Read<Vector3>(entity + 0x3A0);
        Vector3 target_angles = math::CalculateAngle(camera_pos, head);
        
        float dist = math::GetDistance(current_angles, target_angles);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_angles = target_angles;
        }
    }

    return nearest_angles;
}

void SmoothAim(Vector3 current, Vector3 target, float smooth) {
    Vector3 delta;
    delta.x = (target.x - current.x) / smooth;
    delta.y = (target.y - current.y) / smooth;
    delta.z = 0.0f;

    uintptr_t local_player = memory::Read<uintptr_t>(memory::module_base + OFFSET_LOCAL_PLAYER);
    if (!local_player) return;

    Vector3 new_angles;
    new_angles.x = current.x + delta.x;
    new_angles.y = current.y + delta.y;
    new_angles.z = 0.0f;

    memory::Write(local_player + OFFSET_VIEW_ANGLES, new_angles);
}

void Update() {
    if (!enable_correction || !memory::attached) return;

    if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) {
        Vector3 target = GetNearestTarget();
        if (target.x != 0 || target.y != 0) {
            uintptr_t local = memory::Read<uintptr_t>(memory::module_base + OFFSET_LOCAL_PLAYER);
            Vector3 current = memory::Read<Vector3>(local + OFFSET_VIEW_ANGLES);
            SmoothAim(current, target, smoothness);
        }
    }
}

}
