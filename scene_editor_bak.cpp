
    struct EditorState {
        int view_size = 1;
        int pencil_size = 1;
        int page = 0;
        uint8_t id = 0;
        uint8_t color = 0;
        EditorTool tool = EditorTool::Pencil;
        bool edit_font = false;

        int map_x = 0;
        int map_y = 11;
        int map_zoom = 1;

        int edit_x = 0;
        int edit_y = 0;

        EditorPage tab = EditorPage::Sprite;

        t8::text_editor::TextEditor editor;
    };

    EditorState state;

    void editor_spray_sheet(
        int x, int y, int size, int px, int py,
        uint8_t replace, uint8_t to,
        const std::function<uint8_t(int, int)> &getter,
        const std::function<void(int, int, uint8_t)> &setter) {
        struct Point {
            int x, y;
        };

        std::stack<Point> points;

        points.push({px, py});

        while (!points.empty()) {
            const auto p = points.top();
            points.pop();

            if (p.x < 0 || p.x >= 128 || p.y < 0 || p.y >= 128)
                continue;

            if (p.x < x || p.x >= x + size || p.y < y || p.y >= y + size)
                continue;

            const auto it = getter(p.x, p.y);

            if (it == to)
                continue;

            if (it == replace) {
                setter(p.x, p.y, to);
                points.push({p.x - 1, p.y});
                points.push({p.x + 1, p.y});
                points.push({p.x, p.y - 1});
                points.push({p.x, p.y + 1});
            }
        }
    }

    void editor_draw_sheet(
        int x, int y, int range,
        int px, int py, int psize,
        uint8_t color,
        const std::function<void(int, int, uint8_t)> &setter) {
        auto l = std::max(px - (psize >> 1), x);
        auto t = std::max(py - (psize >> 1), y);
        auto r = std::min(px - (psize >> 1) + psize, x + range);
        auto b = std::min(py - (psize >> 1) + psize, y + range);

        for (auto y = t; y < b; y++) {
            for (auto x = l; x < r; x++) {
                setter(x, y, color);
            }
        }
    }

    void editor_erase_sheet(int x, int y, int size, const std::function<void(int, int, uint8_t)> &setter) {
        for (auto _y = y; _y < y + size; _y++) {
            for (auto _x = x; _x < x + size; _x++) {
                setter(_x, _y, 0);
            }
        }
    }

    void editor_rotate_sheet(int x, int y, int size,
                             const std::function<uint8_t(int, int)> &getter,
                             const std::function<void(int, int, uint8_t)> &setter) {
        const auto half = size >> 1;
        const auto next = [&](int &x, int &y) {
            auto t = y;
            y = x;
            x = size - t - 1;
        };

        for (auto dy = 0; dy < half; dy++) {
            for (auto dx = 0; dx < half; dx++) {
                auto tx = dx, ty = dy;
                auto prev = getter(x + dx, y + dy);
                for (auto i = 0; i < 4; i++) {
                    next(tx, ty);
                    auto it = getter(x + tx, y + ty);
                    setter(x + tx, y + ty, prev);
                    prev = it;
                }
            }
        }
    }

    void editor_flip_sheet(int x, int y, int size, bool vertical,
                           const std::function<uint8_t(int, int)> &getter,
                           const std::function<void(int, int, uint8_t)> &setter) {
        if (vertical) {
            for (auto dy = 0; dy < (size >> 1); dy++) {
                for (auto dx = 0; dx < size; dx++) {
                    auto that = getter(x + dx, y + size - dy - 1);
                    auto it = getter(x + dx, y + dy);
                    setter(x + dx, y + size - dy - 1, it);
                    setter(x + dx, y + dy, that);
                }
            }
        } else {
            for (auto dy = 0; dy < size; dy++) {
                for (auto dx = 0; dx < (size >> 1); dx++) {
                    auto that = getter(x + size - dx - 1, y + dy);
                    auto it = getter(x + dx, y + dy);
                    setter(x + size - dx - 1, y + dy, it);
                    setter(x + dx, y + dy, that);
                }
            }
        }
    }

    void editor_update_sheet_edit() {
        const auto id = state.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;
        const auto rangeSize = state.view_size << 3;

        const auto gf = [&](int x, int y) { return painter_font(x, y, true); };
        const auto sf = [&](int x, int y, uint8_t c) { painter_font(x, y, c, true); };
        const auto gs = [&](int x, int y) { return painter_sprite(x, y); };
        const auto ss = [&](int x, int y, uint8_t c) { painter_sprite(x, y, c); };

        // 选择工具
        if (mouse_clicked(8, 86, 7, 7)) {
            state.tool = EditorTool::Pencil;
        }
        if (mouse_clicked(16, 86, 7, 7)) {
            state.tool = EditorTool::Straw;
        }
        if (mouse_clicked(24, 86, 7, 7)) {
            state.tool = EditorTool::Barrel;
        }

        if (state.edit_font) {
            if (mouse_clicked(32, 86, 7, 7)) {
                editor_flip_sheet(spX, spY, rangeSize, false, gf, sf);
            }
            if (mouse_clicked(40, 86, 7, 7)) {
                editor_flip_sheet(spX, spY, rangeSize, true, gf, sf);
            }
            if (mouse_clicked(48, 86, 7, 7)) {
                editor_rotate_sheet(spX, spY, rangeSize, gf, sf);
            }
            if (mouse_clicked(56, 86, 7, 7)) {
                editor_erase_sheet(spX, spY, rangeSize, sf);
            }
        }

        if (!state.edit_font) {
            if (mouse_clicked(32, 86, 7, 7)) {
                editor_flip_sheet(spX, spY, rangeSize, false, gs, ss);
            }
            if (mouse_clicked(40, 86, 7, 7)) {
                editor_flip_sheet(spX, spY, rangeSize, true, gs, ss);
            }
            if (mouse_clicked(48, 86, 7, 7)) {
                editor_rotate_sheet(spX, spY, rangeSize, gs, ss);
            }
            if (mouse_clicked(56, 86, 7, 7)) {
                editor_erase_sheet(spX, spY, rangeSize, ss);
            }
        }

        if (mouse_clicked(85, 86, 7, 7)) {
            state.edit_font = !state.edit_font;
        }

        {
            // 绘制区域
            auto innerX = mouse_x() - 8;
            auto innerY = mouse_y() - 19;

            innerX /= 8 >> (state.view_size >> 1);
            innerY /= 8 >> (state.view_size >> 1);

            if (state.tool == EditorTool::Straw) {
                if (mouse_clicked(8, 19, 64, 64)) {
                    state.color = state.edit_font ? gf(spX + innerX, spY + innerY) : gs(spX + innerX, spY + innerY);
                    state.tool = EditorTool::Pencil;
                }
            }

            if (state.tool == EditorTool::Barrel) {
                if (mouse_clicked(8, 19, 64, 64)) {
                    auto replace = state.edit_font ? gf(spX + innerX, spY + innerY) : gs(spX + innerX, spY + innerY);
                    if (state.edit_font) {
                        editor_spray_sheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            replace,
                            state.color,
                            gf,
                            sf);
                    } else {
                        editor_spray_sheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            replace,
                            state.color,
                            gs,
                            ss);
                    }
                }
            }
            if (state.tool == EditorTool::Pencil) {
                if (mouse_dragging(8, 19, 64, 64)) {
                    if (state.edit_font) {
                        editor_draw_sheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            state.pencil_size, state.color, sf);
                    } else {
                        editor_draw_sheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            state.pencil_size, state.color, ss);
                    }
                }
            }
            if (mouse_inside(8, 19, 64, 64)) {
                if (mouse_z() < 0) {
                    state.view_size = std::clamp(state.view_size << 1, 1, 4);
                }
                if (mouse_z() > 0) {
                    state.view_size = std::clamp(state.view_size >> 1, 1, 4);
                }
            }
        }

        {
            // 设置精灵flag
            if (!state.edit_font) {
                auto old = 0;
                for (auto y = idY; y < std::min(16, idY + state.view_size); y++) {
                    for (auto x = idX; x < std::min(16, idX + state.view_size); x++) {
                        old |= painter_flags(((y << 4) | x) & 0xFF);
                    }
                }

                for (auto i = 0; i < 8; i++) {
                    if (mouse_clicked(85 + ((i << 2) + i), 81, 4, 4)) {
                        for (auto y = idY; y < std::min(16, idY + state.view_size); y++) {
                            for (auto x = idX; x < std::min(16, idX + state.view_size); x++) {
                                auto flag = painter_flags(((y << 4) | x) & 0xFF);
                                if (old & (1 << i)) {
                                    flag &= ~static_cast<uint8_t>(1 << i);
                                } else {
                                    flag |= static_cast<uint8_t>(1 << i);
                                }
                                painter_flags(((y << 4) | x) & 0xFF, flag);
                            }
                        }
                    }
                }
            }
        }

        // 选择调色板
        if (mouse_dragging(88, 19, 32, 32)) {
            int x = (mouse_x() - 88) >> 3;
            int y = (mouse_y() - 19) >> 3;
            state.color = (y << 2) | x;
        }

        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (mouse_clicked(100 + i * 6, 88, 5, 4)) {
                state.page = i;
            }
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32)) {
            auto innerX = mouse_x();
            auto innerY = mouse_y() - 96;
            innerX >>= 3;
            innerY >>= 3;
            auto newId = ((innerY << 4) | (innerX & 0xF)) & 0xFF;
            newId += (state.page << 6);
            state.id = newId;
        } else if (mouse_inside(0, 96, 128, 32)) {
            if (mouse_z() < 0) {
                state.page = std::clamp(state.page + 1, 0, 3);
            }
            if (mouse_z() > 0) {
                state.page = std::clamp(state.page - 1, 0, 3);
            }
        }

        // 右侧滑块
        for (auto i = 0; i < 3; i++) {
            if (mouse_clicked(98 + (i << 3), 59, 5, 5)) {
                state.view_size = (1 << i);
            }
            if (mouse_clicked(98 + (i << 3), 69, 5, 5)) {
                state.pencil_size = (((1 << i) >> 1) << 1) + 1;
            }
        }
    }

    void editor_update_script_edit() {
        auto autoScroll = true;

        if (!context()->inputs.empty()) {
            const auto text = context()->inputs.back();
            context()->inputs.pop();

            for (const auto &ch : text)
                if (ch & 0x80)
                    return;

            state.editor.insert(text);
        } else {
            if (keyboard_triggered(40) || keyboard_triggered(88)) {
                state.editor.insert('\n');
            } else if (keyboard_triggered(42)) {
                state.editor.erase_before();
            } else if (keyboard_triggered(82)) {
                state.editor.move(MoveType::PrevLine, !keyboard_shift());
            } else if (keyboard_triggered(81)) {
                state.editor.move(MoveType::NextLine, !keyboard_shift());
            } else if (keyboard_triggered(80)) {
                state.editor.move(MoveType::PrevColumn, !keyboard_shift());
            } else if (keyboard_triggered(79)) {
                state.editor.move(MoveType::NextColumn, !keyboard_shift());
            } else if (keyboard_pressed(4)) {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl()) {
                    state.editor.selectAll();
                }
            } else if (keyboard_pressed(29)) {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl()) {
                    state.editor.undo();
                }
            } else if (keyboard_pressed(28)) {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl()) {
                    state.editor.redo();
                }
            } else if (keyboard_pressed(43)) {
                if (!keyboard_alt() && !keyboard_shift() && !keyboard_ctrl()) {
                    state.editor.insert(" ");
                }
            } else {
                autoScroll = false;
            }
        }

        if (mouse_inside(8, 11, 120, 117)) {
            const auto innerX = (mouse_x() - 8) / 4;
            const auto innerY = (mouse_y() - 11) / 8;
            const auto pos = Coord(
                innerY + state.edit_x,
                innerX + state.edit_y);

            if (mouse_pressed(1)) {
                state.editor.setCursor(pos);
                state.editor.setAncher(pos);
            } else if (mouse_down(1)) {
                state.editor.setAncher(pos);
            }

            const auto lineCount = static_cast<int>(state.editor.lineCount());
            if (mouse_z() > 0) {
                state.edit_y = std::max(state.edit_y - 1, 0);
            }
            if (mouse_z() < 0 && lineCount > 13) {
                state.edit_y = std::min(state.edit_y + 1, lineCount - 14);
            }
        }

        if (autoScroll) {
            const auto cursor = state.editor.cursor();
            const auto deltaX = cursor.column - state.edit_x;
            const auto deltaY = cursor.line - state.edit_y;
            if (deltaY > 13) {
                state.edit_y = cursor.line - 13;
            } else if (deltaY < 0) {
                state.edit_y = cursor.line;
            }

            if (deltaX > 29) {
                state.edit_x = cursor.column - 29;
            } else if (deltaX < 0) {
                state.edit_x = cursor.column;
            }
        }
    }

    void editor_update_map_edit() {

        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (mouse_clicked(100 + i * 6, 88, 5, 4)) {
                state.page = i;
            }
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32)) {
            auto innerX = mouse_x();
            auto innerY = mouse_y() - 96;
            innerX >>= 3;
            innerY >>= 3;
            auto newId = ((innerY << 4) | (innerX & 0xF)) & 0xFF;
            newId += (state.page << 6);
            state.id = newId;
        } else if (mouse_inside(0, 96, 128, 32)) {
            if (mouse_z() < 0) {
                state.page = std::clamp(state.page + 1, 0, 3);
            }
            if (mouse_z() > 0) {
                state.page = std::clamp(state.page - 1, 0, 3);
            }
        }

        // 绘制地图
        if (mouse_dragging(0, 11, 128, 74)) {
            auto innerX = mouse_x() - state.map_x;
            auto innerY = mouse_y() - state.map_y;
            innerX /= 8 >> (state.map_zoom >> 1);
            innerY /= 8 >> (state.map_zoom >> 1);
            painter_map(innerX, innerY, state.id);
        } else if (mouse_dragging(0, 11, 128, 74, 2)) {
            state.map_x += mouse_dx();
            state.map_y += mouse_dy();
        } else if (mouse_inside(0, 11, 128, 74)) {
            const auto z = mouse_z();
            if (z != 0) {
                const auto next = std::clamp(z < 0 ? state.map_zoom << 1 : state.map_zoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != state.map_zoom) {
                    state.map_x = 64 + static_cast<int>((state.map_x - 64) * scale);
                    state.map_y = 48 + static_cast<int>((state.map_y - 48) * scale);
                    state.map_zoom = next;
                }
            }
        }
    }

    void editor_draw_tab() {
        painter_rect(0, 0, 128, 11, 13, true);
        painter_char(0x80, 1, 1, (state.tab == EditorPage::Script || mouse_inside(2, 2, 7, 7)) ? 7 : 1);
        painter_char(0x81, 10, 1, (state.tab == EditorPage::Sprite || mouse_inside(11, 2, 7, 7)) ? 7 : 1);
        painter_char(0x83, 19, 1, (state.tab == EditorPage::Map || mouse_inside(20, 2, 7, 7)) ? 7 : 1);
    }

    void editor_draw_sheet_edit() {
        const auto id = state.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;

        painter_rect(0, 11, 128, 84, 14, true);
        painter_rect(7, 18, 66, 66, 0);
        painter_rect(87, 18, 34, 34, 0);

        {
            // 绘制精灵内容
            auto pixelSize = 8 >> (state.view_size >> 1);
            auto range = 8 << (state.view_size >> 1);

            for (auto dy = 0; dy < range; dy++) {
                for (auto dx = 0; dx < range; dx++) {
                    auto color = state.edit_font ? painter_font(spX + dx, spY + dy, true) : painter_sprite(spX + dx, spY + dy);
                    painter_rect(8 + dx * pixelSize, 19 + dy * pixelSize, pixelSize, pixelSize, color, true);
                }
            }

            // 画板工具按钮
            painter_char(0x90, 7, 85, (state.tool == EditorTool::Pencil || mouse_inside(7, 85, 8, 8)) ? 3 : 1);
            painter_char(0x91, 15, 85, (state.tool == EditorTool::Straw || mouse_inside(15, 85, 8, 8)) ? 3 : 1);
            painter_char(0x92, 23, 85, (state.tool == EditorTool::Barrel || mouse_inside(23, 85, 8, 8)) ? 3 : 1);
            painter_char(0x93, 31, 85, mouse_inside(31, 85, 8, 8) ? 3 : 1);
            painter_char(0x94, 39, 85, mouse_inside(39, 85, 8, 8) ? 3 : 1);
            painter_char(0x95, 47, 85, mouse_inside(47, 85, 8, 8) ? 3 : 1);
            painter_char(0x96, 55, 85, mouse_inside(55, 85, 8, 8) ? 3 : 1);

            // 当前精灵序号
            const static char hex[16] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F'};

            painter_char(hex[idY], 64, 86, 3);
            painter_char(hex[idX], 68, 86, 3);
        }

        {
            // 精灵flag设置
            if (!state.edit_font) {
                auto flag = 0;
                for (auto y = idY; y < std::min(16, idY + state.view_size); y++) {
                    for (auto x = idX; x < std::min(16, idX + state.view_size); x++) {
                        flag |= painter_flags(((y << 4) | x) & 0xFF);
                    }
                }
                for (auto i = 0; i < 8; i++) {
                    painter_rect(85 + ((i << 2) + i), 80, 4, 4, 0, true);
                    if (flag & (1 << i)) {
                        painter_rect(86 + ((i << 2) + i), 81, 2, 2, 3, true);
                    }
                }
            }
        }

        {
            // 右侧调色板
            for (auto i = 0; i < 16; i++) {
                int x = i & 0b11;
                int y = i >> 2;
                painter_rect(88 + (x << 3), 19 + (y << 3), 8, 8, i, true);
            }

            auto select = state.color;
            int x = select & 0b11;
            int y = select >> 2;
            painter_rect(87 + (x << 3), 18 + (y << 3), 10, 10, 1);
        }

        {
            // 右侧滑块
            painter_rect(100, 61, 17, 1, 13);
            painter_rect(100, 71, 17, 1, 13);

            for (auto i = 0; i < 3; i++) {
                painter_rect(99 + (i << 3), 60, 3, 3, 0, true);
                if (state.view_size == (1 << i)) {
                    painter_pixel(100 + (i << 3), 61, 3);
                }
                painter_rect(99 + (i << 3), 70, 3, 3, 0, true);

                if (state.pencil_size == (((1 << i) >> 1) << 1) + 1) {
                    painter_pixel(100 + (i << 3), 71, 3);
                }
            }

            painter_rect(88, 68, 7, 7, 0, true);

            painter_rect(
                91 - (state.pencil_size >> 1),
                71 - (state.pencil_size >> 1),
                state.pencil_size,
                state.pencil_size,
                1, true);

            painter_char(0x98, 87, 57, 1);
        }

        {
            // 精灵分组
            for (auto i = 0; i < 4; i++) {
                painter_rect(99 + i * 6, 87, 7, 6, 13);
                painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            painter_rect(98 + state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + state.page * 6, 87, 7, 6, 3, true);
        }

        {
            // 下方精灵图预览及选择

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = state.edit_font ? painter_font(dx, (state.page << 5) + dy, true) : painter_sprite(dx, (state.page << 5) + dy);
                    painter_pixel(dx, 96 + dy, color);
                }
            }

            painter_clip(0, 96, 128, 32);
            // 选择的区域
            painter_rect(
                spX,
                96 + spY - (state.page << 5),
                state.view_size << 3,
                state.view_size << 3,
                1);
            painter_clip();
        }

        // 启用字体编辑
        painter_char(0x82, 84, 85, (state.edit_font || mouse_inside(85, 86, 7, 7)) ? 6 : 1);
    }

    void editor_draw_script_edit() {

        painter_rect(0, 11, 8, 117, 14, true);
        painter_rect(8, 11, 120, 117, 15, true);

        auto ancher = state.editor.ancher();
        auto cursor = state.editor.cursor();

        if (ancher > cursor)
            std::swap(ancher, cursor);

        painter_camera(8, 11);
        painter_clip(8, 11, 120, 117);

        if ((timer_ticks() >> 5) % 2)
            painter_rect(
                (cursor.column - state.edit_x) << 2,
                (cursor.line - state.edit_y) << 3,
                1, 7, 3, true);

        auto x = 0, y = 0;
        state.editor.eachLine(
            state.edit_y,
            std::min(state.edit_y + 15, static_cast<int>(state.editor.lineCount())),
            [&](Line &line, const int &ln) {
                for (
                    auto col = std::max(0, state.edit_x);
                    col < std::min(static_cast<int>(line.size()), state.edit_x + 31);
                    col++) {
                    const auto &ch = line[col];
                    const auto p = Coord(ln, col);

                    if (p >= ancher && p < cursor)
                        painter_rect(
                            (p.column - state.edit_x) << 2,
                            (p.line - state.edit_y) << 3,
                            4, 7, 3, true);

                    painter_char(ch.first, x, y, 1);
                    x += 4;
                }

                x = 0;
                y += 8;
            });

        painter_camera();
        painter_clip();
    }

    void editor_draw_map_edit() {
        const auto id = state.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;

        painter_rect(0, 11, 128, 84, 10, true);

        // 网格背景
        for (auto y = 11; y < 85; y++) {
            for (auto x = 0; x < 128; x++) {
                if ((x + y) & 1) {
                    painter_pixel(x, y, 8);
                } else {
                    painter_pixel(x, y, 0);
                }
            }
        }

        painter_rect(0, 85, 128, 10, 14, true);

        {
            for (auto i = 0; i < 4; i++) {
                painter_rect(99 + i * 6, 87, 7, 6, 13);
                painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            painter_rect(98 + state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + state.page * 6, 87, 7, 6, 3, true);
        }

        {
            // 下方精灵图预览及选择

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = painter_sprite(dx, (state.page << 5) + dy);
                    painter_pixel(dx, 96 + dy, color);
                }
            }

            painter_clip(0, 96, 128, 32);
            // 选择的区域
            painter_rect(
                spX,
                96 + spY - (state.page << 5),
                8,
                8,
                1);
            painter_clip();
        }

        {
            painter_clip(0, 11, 128, 74);

            auto chunkSize = 8 >> (state.map_zoom >> 1);
            auto step = state.map_zoom;

            painter_rect(state.map_x - 1, state.map_y - 1, 128 * chunkSize + 2, 128 * chunkSize + 2, 2);

            auto l = std::clamp(-state.map_x / chunkSize, 0, 128);
            auto t = std::clamp(-state.map_y / chunkSize, 0, 128);
            auto r = std::clamp((128 - state.map_x) / chunkSize + 1, 0, 128);
            auto b = std::clamp((128 - state.map_y) / chunkSize + 1, 0, 128);

            for (auto y = t; y < b; y++) {
                for (auto x = l; x < r; x++) {
                    auto id = painter_map(x, y);
                    auto spX = (id & 0xF) << 3;
                    auto spY = ((id >> 4) & 0xF) << 3;

                    for (auto inY = 0; inY < chunkSize; inY++) {
                        for (auto inX = 0; inX < chunkSize; inX++) {
                            auto color = id == 0 ? 0 : painter_sprite(spX + inX * step, spY + inY * step);
                            painter_pixel(
                                state.map_x + x * chunkSize + inX,
                                state.map_y + y * chunkSize + inY,
                                color);
                        }
                    }
                }
            }

            if (mouse_inside(0, 11, 128, 74)) {
                auto innerX = mouse_x() - state.map_x;
                auto innerY = mouse_y() - state.map_y;
                innerX /= chunkSize;
                innerY /= chunkSize;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128) {
                    painter_rect(state.map_x + innerX * chunkSize - 1, state.map_y + innerY * chunkSize - 1, chunkSize + 2, chunkSize + 2, 1);
                }
            }

            painter_clip();
        }
    }

    void editor_update_tab() {
        if (mouse_clicked(1, 1, 8, 8)) {
            signal_push(Signal::StartInput);
            state.tab = EditorPage::Script;
        }
        if (mouse_clicked(10, 1, 8, 8)) {
            signal_push(Signal::StopInput);
            state.tab = EditorPage::Sprite;
        }
        if (mouse_clicked(19, 1, 8, 8)) {
            signal_push(Signal::StopInput);
            state.tab = EditorPage::Map;
        }
    }

    
    void editor_update() {
        if (keyboard_pressed(41)) {
            signal_push(SIGNAL_SWAP_CONSOLE);
            return;
        }

        editor_update_tab();

        if (state.tab == EditorPage::Sprite) {
            editor_update_sheet_edit();
        }
        if (state.tab == EditorPage::Script) {
            editor_update_script_edit();
        }
        if (state.tab == EditorPage::Map) {
            editor_update_map_edit();
        }
    }

    void editor_draw() {
        painter_clear(0);

        editor_draw_tab();

        if (state.tab == EditorPage::Sprite) {
            editor_draw_sheet_edit();
        }
        if (state.tab == EditorPage::Script) {
            editor_draw_script_edit();
        }
        if (state.tab == EditorPage::Map) {
            editor_draw_map_edit();
        }
    }