#include <functional>

namespace t8::scene {
    struct SceneRecord {
        std::function<void()> update;
        std::function<void()> draw;
        std::function<void()> enter;
        std::function<void()> leave;
    };

    void scene_register(uint32_t id, SceneRecord record);

    void scene_swap(uint32_t id);

    void scene_update();

    void scene_draw();
}