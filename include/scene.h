#pragma once
#include <any>
#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>

#include "app.h"
#include "utils.h"

namespace t8y {

    class IScene {
    public:
        IScene(App *app) : app(app) {
        }

        virtual void update() = 0;
        virtual void draw() = 0;
        virtual void enter() = 0;
        virtual void leave() = 0;

    protected:
        App *app;
    };

    class SceneManager {
    protected:
        std::unordered_map<int, std::unique_ptr<IScene>> m_scenes;
        IScene *m_current{nullptr};

    public:
        template <typename T>
        T *emplace(int id, App *app) {
            static_assert(std::is_base_of<IScene, T>::value);

            if (m_scenes.find(id) == m_scenes.end()) {
                auto ptr = new T(app);
                m_scenes.emplace(id, ptr);
                m_current = ptr;
                return ptr;
            }

            return nullptr;
        }

        void swap(int id) {
            if (m_current)
                m_current->leave();

            m_current = nullptr;
            if (m_scenes.find(id) != m_scenes.end()) {
                m_current = m_scenes[id].get();
            }

            if (m_current)
                m_current->enter();
        }

        void update() {
            if (m_current)
                m_current->update();
        }

        void draw() {
            if (m_current)
                m_current->draw();
        }

        template <typename T>
        T *get(int id) {
            static_assert(std::is_base_of<IScene, T>::value);

            if (m_scenes.find(id) != m_scenes.end()) {
                return reinterpret_cast<T *>(m_scenes[id].get());
            }

            return nullptr;
        }
    };

}