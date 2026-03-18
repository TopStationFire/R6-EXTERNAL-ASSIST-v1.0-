#include "visuals.hpp"
#include "../core/memory.hpp"
#include "../core/render.hpp"
#include <imgui.h>

namespace visuals {

constexpr uintptr_t OFFSET_ENTITY_LIST = 0x5A0B7C8;
constexpr uintptr_t OFFSET_LOCAL_PLAYER = 0x5A0B8D0;
constexpr uintptr_t OFFSET_VIEW_MATRIX = 0x5A0C9A0;
constexpr uintptr_t OFFSET_ENTITY_HEALTH = 0x140;
constexpr uintptr_t OFFSET_ENTITY_POSITION = 0x280;
constexpr uintptr_t OFFSET_ENTITY_HEAD = 0x3A0;
constexpr uintptr_t OFFSET_ENTITY_NAME = 0x520;
constexpr uintptr_t OFFSET_ENTITY_TEAM = 0x148;

void RenderESP(ID3D11DeviceContext* context, ID3D11RenderTargetView* target) {
    if (!memory::attached) return;

    uintptr_t local_player = memory::Read<uintptr_t>(memory::module_base + OFFSET_LOCAL_PLAYER);
    if (!local_player) return;

    Vector3 local_position = memory::Read<Vector3>(local_player + OFFSET_ENTITY_POSITION);
    int local_team = memory::Read<int>(local_player + OFFSET_ENTITY_TEAM);

    float view_matrix[16];
    memory::Read(memory::module_base + OFFSET_VIEW_MATRIX, view_matrix, sizeof(view_matrix));

    ImVec2 screen_center = ImGui::GetMainViewport()->GetCenter();

    uintptr_t entity_list = memory::Read<uintptr_t>(memory::module_base + OFFSET_ENTITY_LIST);
    if (!entity_list) return;

    for (int i = 0; i < 64; i++) {
        uintptr_t entity_ptr = memory::Read<uintptr_t>(entity_list + (i * sizeof(uintptr_t)));
        if (!entity_ptr || entity_ptr == local_player) continue;

        int entity_team = memory::Read<int>(entity_ptr + OFFSET_ENTITY_TEAM);
        if (entity_team == local_team) continue;

        float health = memory::Read<float>(entity_ptr + OFFSET_ENTITY_HEALTH);
        if (health <= 0) continue;

        Vector3 position = memory::Read<Vector3>(entity_ptr + OFFSET_ENTITY_POSITION);
        Vector3 head = memory::Read<Vector3>(entity_ptr + OFFSET_ENTITY_HEAD);
        
        char name[64];
        memory::Read(entity_ptr + OFFSET_ENTITY_NAME, name, sizeof(name));

        Vector2 screen_pos = math::WorldToScreen(position, view_matrix, screen_center.x * 2, screen_center.y * 2);
        Vector2 head_screen = math::WorldToScreen(head, view_matrix, screen_center.x * 2, screen_center.y * 2);

        if (screen_pos.x <= 0 || screen_pos.y <= 0) continue;

        float box_height = abs(head_screen.y - screen_pos.y);
        float box_width = box_height * 0.4f;

        float color_enemy[4] = { 1.0f, 0.2f, 0.2f, 1.0f };

        if (enable_boxes) {
            ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
            draw_list->AddRect(
                ImVec2(screen_pos.x - box_width / 2, head_screen.y),
                ImVec2(screen_pos.x + box_width / 2, screen_pos.y),
                ImColor(color_enemy[0], color_enemy[1], color_enemy[2], color_enemy[3]),
                0.0f, 0, 1.5f
            );
        }

        if (enable_health) {
            ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
            float health_width = (health / 100.0f) * box_height;
            draw_list->AddRectFilled(
                ImVec2(screen_pos.x - box_width / 2 - 5, screen_pos.y),
                ImVec2(screen_pos.x - box_width / 2 - 3, screen_pos.y - health_width),
                ImColor(0.0f, 1.0f, 0.0f, 1.0f)
            );
        }

        if (enable_names) {
            ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
            draw_list->AddText(
                ImVec2(screen_pos.x - box_width / 2, head_screen.y - 15),
                ImColor(1.0f, 1.0f, 1.0f, 1.0f),
                name
            );
        }
    }
}

}
