#pragma once

#include "scene.h"
#include "text_editor.h"

namespace t8y {
    class EditView : public IScene {
    public:
        EditView(App *app);

        virtual void update();

        virtual void draw();

        virtual void enter();

        virtual void leave();

        void reset();

    private:
        void drawTab();
        void updateTab();

        void drawScriptEdit();
        void updateScriptEdit();

        void drawSheetEdit();
        void updateSheetEdit();

        void drawMapEdit();
        void updateMapEdit();

        void spraySheet(
            int x, int y, int size,
            int px, int py,
            uint8_t replace, uint8_t to,
            const std::function<uint8_t(int, int)> &getter,
            const std::function<void(int, int, uint8_t)> &setter);

        void drawSheet(
            int x, int y, int range,
            int px, int py, int psize,
            uint8_t color,
            const std::function<void(int, int, uint8_t)> &setter);

        void eraseSheet(
            int x, int y, int size,
            const std::function<void(int, int, uint8_t)> &setter);

        void rotateSheet(
            int x, int y, int size,
            const std::function<uint8_t(int, int)> &getter,
            const std::function<void(int, int, uint8_t)> &setter);

        void flipSheet(
            int x, int y, int size, bool vertical,
            const std::function<uint8_t(int, int)> &getter,
            const std::function<void(int, int, uint8_t)> &setter);

    private:
        enum class Tab {
            Script,
            Sheet,
            Map
        };

        enum class Tool {
            Pencil,
            Straw,
            Barrel
        };

        struct EditContext {
            int viewSize = 1;
            int pencilSize = 1;
            int group = 0;
            uint8_t id = 0;
            uint8_t color = 0;
            Tool tool = Tool::Pencil;
            bool fontMode = false;

            int mapX = 0;
            int mapY = 11;
            int mapZoom = 1;

            int editX = 0;
            int editY = 0;

            Tab tab = Tab::Sheet;
        };

        EditContext m_ctx;

        TextEditor m_editor;
    };
}