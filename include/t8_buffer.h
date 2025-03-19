#pragma once

namespace t8 {
    
    template <size_t W, size_t H>
    class _4BitBuffer {
    private:
        struct HL {
            char lo : 4;
            char hi : 4;
        };

        HL *buf{nullptr};

    public:
        _4BitBuffer(uint8_t *ptr) {
            buf = reinterpret_cast<HL *>(ptr);
        };

        uint8_t get(int x, int y) {
            if (x < 0 || x >= W || y < 0 || y >= H)
                return 0;
            auto t = (y * W + x);
            auto i = t >> 1;
            return ((t & 1) ? (buf[i].lo) : (buf[i].hi)) & 0xF;
        }

        void set(int x, int y, uint8_t val) {
            if (x < 0 || x >= W || y < 0 || y >= H)
                return;
            auto t = (y * W + x);
            auto i = t >> 1;
            (t & 1) ? (buf[i].lo = val) : (buf[i].hi = val);
        }
    };

    template <size_t W, size_t H>
    class _1BitBuffer {
    private:
        uint8_t *buf{nullptr};

    public:
        _1BitBuffer(uint8_t *ptr) {
            buf = ptr;
        };

        bool get(int x, int y) {
            if (x < 0 || x >= W || y < 0 || y >= H)
                return false;
            auto t = (y * W + x);
            auto i = t >> 3;
            return buf[i] & (1 << (t & 0b111));
        }

        void set(int x, int y, bool val) {
            if (x < 0 || x >= W || y < 0 || y >= H)
                return;
            auto t = (y * W + x);
            auto i = t >> 3;
            if (val) {
                buf[i] |= (1 << (t & 0b111));
            } else {
                buf[i] &= ~(1 << (t & 0b111));
            }
        }
    };
    
}