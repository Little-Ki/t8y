#pragma once

#include "scene.h"

namespace t8y {
    class ConsoleView : public IScene {
    private:
        struct Line {
            std::string content;
            uint8_t color;
            bool prefix;
        };

        using Lines = std::vector<Line>;

    public:
        ConsoleView(App *app);

        virtual void update();

        virtual void draw();

        virtual void enter();

        virtual void leave();

        void join(const std::string &text, bool prefix = false, uint8_t color = 1);

    private:
        bool command();

        void sanitize();

        int measureHeight(const Line &line);

        int measureLines();

        int measureInput();

        void load(const std::string &filename);

        void save(const std::string &filename);

        bool validate(const std::vector<std::string> payload, uint32_t count);

        void clear();

    private:
        std::string m_input;

        size_t m_cursor{0};

        Lines m_lines;

        bool m_first_time{true};

        struct {
            std::vector<std::string> items;
            size_t index;
            bool use{false};
        } m_history;
    };
}