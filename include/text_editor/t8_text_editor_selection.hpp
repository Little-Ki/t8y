#pragma once
#include <algorithm>

namespace t8::text_editor {

    struct EditSelection {
        size_t anchor = 0;
        size_t cursor = 0;

        bool empty() const {
            return anchor == cursor;
        }

        size_t start() const {
            return std::min(anchor, cursor);
        }

        size_t end() const {
            return std::max(anchor, cursor);
        }

        size_t size() const {
            return end() - start();
        }

        void collapse(size_t pos) {
            anchor = cursor = pos;
        }
    };

}