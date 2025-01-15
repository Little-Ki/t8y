#pragma once
#include <memory>
#include <optional>
#include <tuple>

#include "app_context.h"

namespace t8y {
    class Graphic {
    public:
        Graphic(AppContext* ctx);

        void reset();

        void clip(int x = 0, int y = 0, int w = 128, int h = 128);

        void camera(int8_t x = 0, int8_t y = 0);

        void clear(uint8_t c);

        void setTrans(uint8_t color, bool t);
        void setTrans(uint16_t t = 1);

        void setPalt(uint8_t n, uint8_t remap);
        uint8_t getPalt(uint8_t n);

        void setPixel(int x, int y, uint8_t color);
        uint8_t getPixel(int x, int y);

        void setSprite(int x, int y, uint8_t color);
        uint8_t getSprite(int x, int y);

        void setMap(int x, int y, uint8_t n);
        uint8_t getMap(int x, int y);

        void setFlag(uint8_t n, uint8_t f);
        uint8_t getFlag(uint8_t n);

        void setFont(int x, int y, bool value, bool plane = false);
        bool getFont(int x, int y, bool plane = false);

        void drawChar(uint8_t n, int x, int y, uint8_t color = 0x1, bool plane = false);

        void drawLine(int x0, int y0, int x1, int y1, uint8_t color);

        void drawCircel(int x, int y, int r, uint8_t color, bool fill = false);

        void drawRect(int x, int y, int w, int h, uint8_t color, bool fill = false);

        void drawTTri(
            int x1, int y1, int z1, int u1, int v1,
            int x2, int y2, int z2, int u2, int v2,
            int x3, int y3, int z3, int u3, int v3);

    private:
        AppContext* ctx;

    };
}