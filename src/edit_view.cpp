#include "edit_view.h"

#include <iostream>

namespace t8y {
    EditView::EditView(App *app) : IScene(app) {
    }

    void EditView::update() {
        if (app->keyboard.pressed(41)) {
            app->signal.join(SIGNAL_SWAP_CONSOLE);
            return;
        }

        updateTab();
        if (m_ctx.tab == Tab::Sheet) {
            updateSheetEdit();
        }
        if (m_ctx.tab == Tab::Script) {
            updateScriptEdit();
        }
        if (m_ctx.tab == Tab::Map) {
            updateMapEdit();
        }
    }

    void EditView::draw() {
        app->graphic.clear(0);

        drawTab();
        if (m_ctx.tab == Tab::Sheet) {
            drawSheetEdit();
        }
        if (m_ctx.tab == Tab::Script) {
            drawScriptEdit();
        }
        if (m_ctx.tab == Tab::Map) {
            drawMapEdit();
        }
    }

    void EditView::enter() {
        for (auto i = 0; i < 16; i++)
            app->graphic.setPalt(i, i);
        app->graphic.clip();
        app->graphic.camera();

        m_editor.setText(app->context.script);

        if (m_ctx.tab == Tab::Script) {
            app->signal.join(SIGNAL_START_INPUT);
        }
    }

    void EditView::leave() {
        app->context.script = m_editor.getText();
    }

    void EditView::reset() {
    }

    void EditView::drawTab() {
        app->graphic.drawRect(0, 0, 128, 11, 13, true);
        app->graphic.drawChar(0x80, 1, 1, (m_ctx.tab == Tab::Script || app->mouse.inside(2, 2, 7, 7)) ? 7 : 1);
        app->graphic.drawChar(0x81, 10, 1, (m_ctx.tab == Tab::Sheet || app->mouse.inside(11, 2, 7, 7)) ? 7 : 1);
        app->graphic.drawChar(0x83, 19, 1, (m_ctx.tab == Tab::Map || app->mouse.inside(20, 2, 7, 7)) ? 7 : 1);
    }

    void EditView::drawSheetEdit() {
        const auto id = m_ctx.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;

        app->graphic.drawRect(0, 11, 128, 84, 14, true);
        app->graphic.drawRect(7, 18, 66, 66, 0);
        app->graphic.drawRect(87, 18, 34, 34, 0);

        {
            // 绘制精灵内容
            auto pixelSize = 8 >> (m_ctx.viewSize >> 1);
            auto range = 8 << (m_ctx.viewSize >> 1);

            for (auto dy = 0; dy < range; dy++) {
                for (auto dx = 0; dx < range; dx++) {
                    auto color = m_ctx.fontMode ? app->graphic.getFont(spX + dx, spY + dy, true) : app->graphic.getSprite(spX + dx, spY + dy);
                    app->graphic.drawRect(8 + dx * pixelSize, 19 + dy * pixelSize, pixelSize, pixelSize, color, true);
                }
            }

            // 画板工具按钮
            app->graphic.drawChar(0x90, 7, 85, (m_ctx.tool == Tool::Pencil || app->mouse.inside(7, 85, 8, 8)) ? 3 : 1);
            app->graphic.drawChar(0x91, 15, 85, (m_ctx.tool == Tool::Straw || app->mouse.inside(15, 85, 8, 8)) ? 3 : 1);
            app->graphic.drawChar(0x92, 23, 85, (m_ctx.tool == Tool::Barrel || app->mouse.inside(23, 85, 8, 8)) ? 3 : 1);
            app->graphic.drawChar(0x93, 31, 85, app->mouse.inside(31, 85, 8, 8) ? 3 : 1);
            app->graphic.drawChar(0x94, 39, 85, app->mouse.inside(39, 85, 8, 8) ? 3 : 1);
            app->graphic.drawChar(0x95, 47, 85, app->mouse.inside(47, 85, 8, 8) ? 3 : 1);
            app->graphic.drawChar(0x96, 55, 85, app->mouse.inside(55, 85, 8, 8) ? 3 : 1);

            // 当前精灵序号
            const static char hex[16] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F'};

            app->graphic.drawChar(hex[idY], 64, 86, 3);
            app->graphic.drawChar(hex[idX], 68, 86, 3);
        }

        {
            // 精灵flag设置
            if (!m_ctx.fontMode) {
                auto flag = 0;
                for (auto y = idY; y < std::min(16, idY + m_ctx.viewSize); y++) {
                    for (auto x = idX; x < std::min(16, idX + m_ctx.viewSize); x++) {
                        flag |= app->graphic.getFlag(((y << 4) | x) & 0xFF);
                    }
                }
                for (auto i = 0; i < 8; i++) {
                    app->graphic.drawRect(85 + ((i << 2) + i), 80, 4, 4, 0, true);
                    if (flag & (1 << i)) {
                        app->graphic.drawRect(86 + ((i << 2) + i), 81, 2, 2, 3, true);
                    }
                }
            }
        }

        {
            // 右侧调色板
            for (auto i = 0; i < 16; i++) {
                int x = i & 0b11;
                int y = i >> 2;
                app->graphic.drawRect(88 + (x << 3), 19 + (y << 3), 8, 8, i, true);
            }

            auto select = m_ctx.color;
            int x = select & 0b11;
            int y = select >> 2;
            app->graphic.drawRect(87 + (x << 3), 18 + (y << 3), 10, 10, 1);
        }

        {
            // 右侧滑块
            app->graphic.drawRect(100, 61, 17, 1, 13);
            app->graphic.drawRect(100, 71, 17, 1, 13);

            for (auto i = 0; i < 3; i++) {
                app->graphic.drawRect(99 + (i << 3), 60, 3, 3, 0, true);
                if (m_ctx.viewSize == (1 << i)) {
                    app->graphic.setPixel(100 + (i << 3), 61, 3);
                }
                app->graphic.drawRect(99 + (i << 3), 70, 3, 3, 0, true);

                if (m_ctx.pencilSize == (((1 << i) >> 1) << 1) + 1) {
                    app->graphic.setPixel(100 + (i << 3), 71, 3);
                }
            }

            app->graphic.drawRect(88, 68, 7, 7, 0, true);

            app->graphic.drawRect(
                91 - (m_ctx.pencilSize >> 1),
                71 - (m_ctx.pencilSize >> 1),
                m_ctx.pencilSize,
                m_ctx.pencilSize,
                1, true);

            app->graphic.drawChar(0x98, 87, 57, 1);
        }

        {
            // 精灵分组
            for (auto i = 0; i < 4; i++) {
                app->graphic.drawRect(99 + i * 6, 87, 7, 6, 13);
                app->graphic.drawRect(100 + i * 6, 88, 5, 4, app->mouse.inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            app->graphic.drawRect(98 + m_ctx.group * 6, 86, 9, 8, 13);
            app->graphic.drawRect(99 + m_ctx.group * 6, 87, 7, 6, 3, true);
        }

        {
            // 下方精灵图预览及选择

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = m_ctx.fontMode ? app->graphic.getFont(dx, (m_ctx.group << 5) + dy, true) : app->graphic.getSprite(dx, (m_ctx.group << 5) + dy);
                    app->graphic.setPixel(dx, 96 + dy, color);
                }
            }

            app->graphic.clip(0, 96, 128, 32);
            // 选择的区域
            app->graphic.drawRect(
                spX,
                96 + spY - (m_ctx.group << 5),
                m_ctx.viewSize << 3,
                m_ctx.viewSize << 3,
                1);
            app->graphic.clip();
        }

        // 启用字体编辑
        app->graphic.drawChar(0x82, 84, 85, (m_ctx.fontMode || app->mouse.inside(85, 86, 7, 7)) ? 6 : 1);
    }

    void EditView::drawScriptEdit() {

        app->graphic.drawRect(0, 11, 8, 117, 14, true);
        app->graphic.drawRect(8, 11, 120, 117, 15, true);

        auto ancher = m_editor.ancher();
        auto cursor = m_editor.cursor();

        if (ancher > cursor)
            std::swap(ancher, cursor);

        app->graphic.camera(8, 11);
        app->graphic.clip(8, 11, 120, 117);
        app->graphic.drawRect(
            (cursor.column - m_ctx.editX) << 2,
            (cursor.line - m_ctx.editY) << 3,
            1, 7, 3, true);

        auto x = 0, y = 0;
        m_editor.eachLine(
            m_ctx.editY,
            std::min(m_ctx.editY + 15, static_cast<int>(m_editor.lineCount())),
            [&](Line &line, const int &ln) {
                for (
                    auto col = std::max(0, m_ctx.editX);
                    col < std::min(static_cast<int>(line.size()), m_ctx.editX + 31);
                    col++) {
                    const auto &ch = line[col];
                    const auto p = Coord(ln, col);

                    if (p >= ancher && p < cursor)
                        app->graphic.drawRect(
                            (p.column - m_ctx.editX) << 2,
                            (p.line - m_ctx.editY) << 3,
                            4, 7, 3, true);

                    app->graphic.drawChar(ch.first, x, y, 1);
                    x += 4;
                }

                x = 0;
                y += 8;
            });

        app->graphic.camera();
        app->graphic.clip();
    }

    void EditView::drawMapEdit() {
        const auto id = m_ctx.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;

        app->graphic.drawRect(0, 11, 128, 84, 10, true);

        // 网格背景
        for (auto y = 11; y < 85; y++) {
            for (auto x = 0; x < 128; x++) {
                if ((x + y) & 1) {
                    app->graphic.setPixel(x, y, 8);
                } else {
                    app->graphic.setPixel(x, y, 0);
                }
            }
        }

        app->graphic.drawRect(0, 85, 128, 10, 14, true);

        {
            for (auto i = 0; i < 4; i++) {
                app->graphic.drawRect(99 + i * 6, 87, 7, 6, 13);
                app->graphic.drawRect(100 + i * 6, 88, 5, 4, app->mouse.inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            app->graphic.drawRect(98 + m_ctx.group * 6, 86, 9, 8, 13);
            app->graphic.drawRect(99 + m_ctx.group * 6, 87, 7, 6, 3, true);
        }

        {
            // 下方精灵图预览及选择

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = app->graphic.getSprite(dx, (m_ctx.group << 5) + dy);
                    app->graphic.setPixel(dx, 96 + dy, color);
                }
            }

            app->graphic.clip(0, 96, 128, 32);
            // 选择的区域
            app->graphic.drawRect(
                spX,
                96 + spY - (m_ctx.group << 5),
                8,
                8,
                1);
            app->graphic.clip();
        }

        {
            app->graphic.clip(0, 11, 128, 74);

            auto chunkSize = 8 >> (m_ctx.mapZoom >> 1);
            auto step = m_ctx.mapZoom;

            app->graphic.drawRect(m_ctx.mapX - 1, m_ctx.mapY - 1, 128 * chunkSize + 2, 128 * chunkSize + 2, 2);

            auto l = std::clamp(-m_ctx.mapX / chunkSize, 0, 128);
            auto t = std::clamp(-m_ctx.mapY / chunkSize, 0, 128);
            auto r = std::clamp((128 - m_ctx.mapX) / chunkSize + 1, 0, 128);
            auto b = std::clamp((128 - m_ctx.mapY) / chunkSize + 1, 0, 128);

            for (auto y = t; y < b; y++) {
                for (auto x = l; x < r; x++) {
                    auto id = app->graphic.getMap(x, y);
                    auto spX = (id & 0xF) << 3;
                    auto spY = ((id >> 4) & 0xF) << 3;

                    for (auto inY = 0; inY < chunkSize; inY++) {
                        for (auto inX = 0; inX < chunkSize; inX++) {
                            auto color = id == 0 ? 0 : app->graphic.getSprite(spX + inX * step, spY + inY * step);
                            app->graphic.setPixel(
                                m_ctx.mapX + x * chunkSize + inX,
                                m_ctx.mapY + y * chunkSize + inY,
                                color);
                        }
                    }
                }
            }

            if (app->mouse.inside(0, 11, 128, 74)) {
                auto innerX = app->mouse.x() - m_ctx.mapX;
                auto innerY = app->mouse.y() - m_ctx.mapY;
                innerX /= chunkSize;
                innerY /= chunkSize;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128) {
                    app->graphic.drawRect(m_ctx.mapX + innerX * chunkSize - 1, m_ctx.mapY + innerY * chunkSize - 1, chunkSize + 2, chunkSize + 2, 1);
                }
            }

            app->graphic.clip();
        }
    }

    void EditView::updateTab() {
        if (app->mouse.clicked(1, 1, 8, 8)) {
            app->signal.join(SIGNAL_START_INPUT);
            m_ctx.tab = Tab::Script;
        }
        if (app->mouse.clicked(10, 1, 8, 8)) {
            app->signal.join(SIGNAL_STOP_INPUT);
            m_ctx.tab = Tab::Sheet;
        }
        if (app->mouse.clicked(19, 1, 8, 8)) {
            app->signal.join(SIGNAL_STOP_INPUT);
            m_ctx.tab = Tab::Map;
        }
    }

    void EditView::updateSheetEdit() {
        const auto id = m_ctx.id;
        const auto idX = (id & 0xF);
        const auto idY = (id >> 4) & 0xF;
        const auto spX = idX << 3;
        const auto spY = idY << 3;
        const auto rangeSize = m_ctx.viewSize << 3;

        const auto gf = [&](int x, int y) { return app->graphic.getFont(x, y, true); };
        const auto sf = [&](int x, int y, uint8_t c) { app->graphic.setFont(x, y, c, true); };
        const auto gs = [&](int x, int y) { return app->graphic.getSprite(x, y); };
        const auto ss = [&](int x, int y, uint8_t c) { app->graphic.setSprite(x, y, c); };

        // 选择工具
        if (app->mouse.clicked(8, 86, 7, 7)) {
            m_ctx.tool = Tool::Pencil;
        }
        if (app->mouse.clicked(16, 86, 7, 7)) {
            m_ctx.tool = Tool::Straw;
        }
        if (app->mouse.clicked(24, 86, 7, 7)) {
            m_ctx.tool = Tool::Barrel;
        }

        if (m_ctx.fontMode) {
            if (app->mouse.clicked(32, 86, 7, 7)) {
                flipSheet(spX, spY, rangeSize, false, gf, sf);
            }
            if (app->mouse.clicked(40, 86, 7, 7)) {
                flipSheet(spX, spY, rangeSize, true, gf, sf);
            }
            if (app->mouse.clicked(48, 86, 7, 7)) {
                rotateSheet(spX, spY, rangeSize, gf, sf);
            }
            if (app->mouse.clicked(56, 86, 7, 7)) {
                eraseSheet(spX, spY, rangeSize, sf);
            }
        }

        if (!m_ctx.fontMode) {
            if (app->mouse.clicked(32, 86, 7, 7)) {
                flipSheet(spX, spY, rangeSize, false, gs, ss);
            }
            if (app->mouse.clicked(40, 86, 7, 7)) {
                flipSheet(spX, spY, rangeSize, true, gs, ss);
            }
            if (app->mouse.clicked(48, 86, 7, 7)) {
                rotateSheet(spX, spY, rangeSize, gs, ss);
            }
            if (app->mouse.clicked(56, 86, 7, 7)) {
                eraseSheet(spX, spY, rangeSize, ss);
            }
        }

        if (app->mouse.clicked(85, 86, 7, 7)) {
            m_ctx.fontMode = !m_ctx.fontMode;
        }

        {
            // 绘制区域
            auto innerX = app->mouse.x() - 8;
            auto innerY = app->mouse.y() - 19;

            innerX /= 8 >> (m_ctx.viewSize >> 1);
            innerY /= 8 >> (m_ctx.viewSize >> 1);

            if (m_ctx.tool == Tool::Straw) {
                if (app->mouse.clicked(8, 19, 64, 64)) {
                    m_ctx.color = m_ctx.fontMode ? gf(spX + innerX, spY + innerY) : gs(spX + innerX, spY + innerY);
                    m_ctx.tool = Tool::Pencil;
                }
            }

            if (m_ctx.tool == Tool::Barrel) {
                if (app->mouse.clicked(8, 19, 64, 64)) {
                    auto replace = m_ctx.fontMode ? gf(spX + innerX, spY + innerY) : gs(spX + innerX, spY + innerY);
                    if (m_ctx.fontMode) {
                        spraySheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            replace,
                            m_ctx.color,
                            gf,
                            sf);
                    } else {
                        spraySheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            replace,
                            m_ctx.color,
                            gs,
                            ss);
                    }
                }
            }
            if (m_ctx.tool == Tool::Pencil) {
                if (app->mouse.drag(8, 19, 64, 64)) {
                    if (m_ctx.fontMode) {
                        drawSheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            m_ctx.pencilSize, m_ctx.color, sf);
                    } else {
                        drawSheet(
                            spX, spY,
                            rangeSize,
                            spX + innerX, spY + innerY,
                            m_ctx.pencilSize, m_ctx.color, ss);
                    }
                }
            }
            if (app->mouse.inside(8, 19, 64, 64)) {
                if (app->mouse.z() < 0) {
                    m_ctx.viewSize = std::clamp(m_ctx.viewSize << 1, 1, 4);
                }
                if (app->mouse.z() > 0) {
                    m_ctx.viewSize = std::clamp(m_ctx.viewSize >> 1, 1, 4);
                }
            }
        }

        {
            // 设置精灵flag
            if (!m_ctx.fontMode) {
                auto old = 0;
                for (auto y = idY; y < std::min(16, idY + m_ctx.viewSize); y++) {
                    for (auto x = idX; x < std::min(16, idX + m_ctx.viewSize); x++) {
                        old |= app->graphic.getFlag(((y << 4) | x) & 0xFF);
                    }
                }

                for (auto i = 0; i < 8; i++) {
                    if (app->mouse.clicked(85 + ((i << 2) + i), 81, 4, 4)) {
                        for (auto y = idY; y < std::min(16, idY + m_ctx.viewSize); y++) {
                            for (auto x = idX; x < std::min(16, idX + m_ctx.viewSize); x++) {
                                auto flag = app->graphic.getFlag(((y << 4) | x) & 0xFF);
                                if (old & (1 << i)) {
                                    flag &= ~static_cast<uint8_t>(1 << i);
                                } else {
                                    flag |= static_cast<uint8_t>(1 << i);
                                }
                                app->graphic.setFlag(((y << 4) | x) & 0xFF, flag);
                            }
                        }
                    }
                }
            }
        }

        // 选择调色板
        if (app->mouse.drag(88, 19, 32, 32)) {
            int x = (app->mouse.x() - 88) >> 3;
            int y = (app->mouse.y() - 19) >> 3;
            m_ctx.color = (y << 2) | x;
        }

        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (app->mouse.clicked(100 + i * 6, 88, 5, 4)) {
                m_ctx.group = i;
            }
        }

        // 选择精灵
        if (app->mouse.drag(0, 96, 128, 32)) {
            auto innerX = app->mouse.x();
            auto innerY = app->mouse.y() - 96;
            innerX >>= 3;
            innerY >>= 3;
            auto newId = ((innerY << 4) | (innerX & 0xF)) & 0xFF;
            newId += (m_ctx.group << 6);
            m_ctx.id = newId;
        } else if (app->mouse.inside(0, 96, 128, 32)) {
            if (app->mouse.z() < 0) {
                m_ctx.group = std::clamp(m_ctx.group + 1, 0, 3);
            }
            if (app->mouse.z() > 0) {
                m_ctx.group = std::clamp(m_ctx.group - 1, 0, 3);
            }
        }

        // 右侧滑块
        for (auto i = 0; i < 3; i++) {
            if (app->mouse.clicked(98 + (i << 3), 59, 5, 5)) {
                m_ctx.viewSize = (1 << i);
            }
            if (app->mouse.clicked(98 + (i << 3), 69, 5, 5)) {
                m_ctx.pencilSize = (((1 << i) >> 1) << 1) + 1;
            }
        }
    }

    void EditView::updateScriptEdit() {
        auto &keybord = app->keyboard;
        auto &mouse = app->mouse;

        auto autoScroll = true;
        if (!app->texts.empty()) {
            const auto text = app->texts.back();
            app->texts.pop();

            for (const auto &ch : text)
                if (ch & 0x80)
                    return;

            m_editor.insert(text);
        } else {
            if (keybord.triggered(40) || keybord.triggered(88)) {
                m_editor.insert(std::string(1, '\n'));
            } else if (keybord.triggered(42)) {
                m_editor.backspace();
            } else if (keybord.triggered(82)) {
                m_editor.move(MoveType::PrevLine, !keybord.shift());
            } else if (keybord.triggered(81)) {
                m_editor.move(MoveType::NextLine, !keybord.shift());
            } else if (keybord.triggered(80)) {
                m_editor.move(MoveType::PrevColumn, !keybord.shift());
            } else if (keybord.triggered(79)) {
                m_editor.move(MoveType::NextColumn, !keybord.shift());
            } else if (keybord.pressed(4)) {
                if (!keybord.alt() && !keybord.shift() && keybord.ctrl()) {
                    m_editor.selectAll();
                }
            } else if (keybord.pressed(29)) {
                if (!keybord.alt() && !keybord.shift() && keybord.ctrl()) {
                    m_editor.undo();
                }
            } else if (keybord.pressed(28)) {
                if (!keybord.alt() && !keybord.shift() && keybord.ctrl()) {
                    m_editor.redo();
                }
            } else if (keybord.pressed(43)) {
                if (!keybord.alt() && !keybord.shift() && !keybord.ctrl()) {
                    m_editor.insert(" ");
                }
            } else {
                autoScroll = false;
            }
        }

        if (mouse.inside(8, 11, 120, 117)) {
            const auto innerX = (mouse.x() - 8) / 4;
            const auto innerY = (mouse.y() - 11) / 8;
            const auto pos = Coord(
                innerY + m_ctx.editY,
                innerX + m_ctx.editX);

            if (mouse.pressed(1)) {
                m_editor.setCursor(pos);
                m_editor.setAncher(pos);
            } else if (mouse.down(1)) {
                m_editor.setAncher(pos);
            }

            const auto lineCount = static_cast<int>(m_editor.lineCount());
            if (mouse.z() > 0) {
                m_ctx.editY = std::max(m_ctx.editY - 1, 0);
            }
            if (mouse.z() < 0 && lineCount > 13) {
                m_ctx.editY = std::min(m_ctx.editY + 1, lineCount - 14);
            }
        }

        if (autoScroll) {
            const auto cursor = m_editor.cursor();
            const auto deltaX = cursor.column - m_ctx.editX;
            const auto deltaY = cursor.line - m_ctx.editY;
            if (deltaY > 13) {
                m_ctx.editY = cursor.line - 13;
            } else if (deltaY < 0) {
                m_ctx.editY = cursor.line;
            }

            if (deltaX > 29) {
                m_ctx.editX = cursor.column - 29;
            } else if (deltaX < 0) {
                m_ctx.editX = cursor.column;
            }
        }
    }

    void EditView::updateMapEdit() {

        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (app->mouse.clicked(100 + i * 6, 88, 5, 4)) {
                m_ctx.group = i;
            }
        }

        // 选择精灵
        if (app->mouse.drag(0, 96, 128, 32)) {
            auto innerX = app->mouse.x();
            auto innerY = app->mouse.y() - 96;
            innerX >>= 3;
            innerY >>= 3;
            auto newId = ((innerY << 4) | (innerX & 0xF)) & 0xFF;
            newId += (m_ctx.group << 6);
            m_ctx.id = newId;
        } else if (app->mouse.inside(0, 96, 128, 32)) {
            if (app->mouse.z() < 0) {
                m_ctx.group = std::clamp(m_ctx.group + 1, 0, 3);
            }
            if (app->mouse.z() > 0) {
                m_ctx.group = std::clamp(m_ctx.group - 1, 0, 3);
            }
        }

        // 绘制地图
        if (app->mouse.drag(0, 11, 128, 74)) {
            auto innerX = app->mouse.x() - m_ctx.mapX;
            auto innerY = app->mouse.y() - m_ctx.mapY;
            innerX /= 8 >> (m_ctx.mapZoom >> 1);
            innerY /= 8 >> (m_ctx.mapZoom >> 1);
            app->graphic.setMap(innerX, innerY, m_ctx.id);
        } else if (app->mouse.drag(0, 11, 128, 74, 2)) {
            m_ctx.mapX += app->mouse.dx();
            m_ctx.mapY += app->mouse.dy();
        } else if (app->mouse.inside(0, 11, 128, 74)) {
            const auto z = app->mouse.z();
            if (z != 0) {
                const auto next = std::clamp(z < 0 ? m_ctx.mapZoom << 1 : m_ctx.mapZoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != m_ctx.mapZoom) {
                    m_ctx.mapX = 64 + static_cast<int>((m_ctx.mapX - 64) * scale);
                    m_ctx.mapY = 48 + static_cast<int>((m_ctx.mapY - 48) * scale);
                    m_ctx.mapZoom = next;
                }
            }
        }
    }

    void EditView::spraySheet(
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

    void EditView::drawSheet(
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

    void EditView::eraseSheet(int x, int y, int size, const std::function<void(int, int, uint8_t)> &setter) {
        for (auto _y = y; _y < y + size; _y++) {
            for (auto _x = x; _x < x + size; _x++) {
                setter(_x, _y, 0);
            }
        }
    }

    void EditView::rotateSheet(int x, int y, int size,
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

    void EditView::flipSheet(int x, int y, int size, bool vertical,
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
    
}