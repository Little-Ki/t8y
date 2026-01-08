#include "t8_scene.h"
#include <unordered_map>

namespace t8::scene {
    std::unordered_map<uint32_t, SceneRecord> scene_records;
    SceneRecord *current_scene{nullptr};

    void scene_register(uint32_t id, SceneRecord record) {
        if (scene_records.find(id) == scene_records.end()) {
            scene_records.emplace(id, record);
        }
    }

    void scene_swap(uint32_t id) {
        auto next_scene = scene_records.find(id);

        if (next_scene == scene_records.end() ||
            &next_scene->second == current_scene) {
            return;
        }

        if (current_scene) {
            current_scene->leave();
        }

        current_scene = &next_scene->second;
        current_scene->enter();
    }

    void scene_update() {
        if (current_scene) {
            current_scene->update();
        }
    }

    void scene_draw() {
        if (current_scene) {
            current_scene->draw();
        }
    }
}