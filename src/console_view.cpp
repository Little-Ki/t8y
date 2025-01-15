#include "console_view.h"
#include <fstream>
#include <numeric>

namespace t8y {

    enum class MetaType : uint8_t {
        Sprite = 0,
        Flag = 1,
        Font = 2,
        Map = 3,
        Script = 4,
    };

    struct CartMetaData {
        MetaType type;
        uint32_t size;
    };

    struct CartFileHeader {
        uint16_t magic;
        bool bigEndian;
        uint64_t hash;
        uint16_t count;
    };

    ConsoleView::ConsoleView(App *app) : IScene(app) {
    }

    void ConsoleView::update() {

        auto &keybord = app->keyboard;

        if (keybord.pressed(41)) {
            app->signal.join(SIGNAL_SWAP_EDIT);
            return;
        }

        if (!app->texts.empty()) {
            const auto text = app->texts.back();
            app->texts.pop();

            for (const auto &ch : text)
                if (!(ch & 0x80)) {
                    m_input.insert(std::next(m_input.begin(), m_cursor), ch);
                    m_cursor++;
                }
            sanitize();
            m_history.use = false;

        } else {
            if (keybord.pressed(40) || keybord.pressed(88)) {
                join(m_input, true);
                if (command())
                    if (m_history.items.empty() || m_history.items.back() != m_input) {
                        m_history.items.push_back(m_input);
                    }
                m_history.use = false;
                m_input.clear();
                sanitize();
            } else if (keybord.triggered(42)) {
                if (!m_input.empty() && m_cursor > 0) {
                    m_input.erase(std::next(m_input.begin(), m_cursor - 1));
                    m_cursor--;
                }
            } else if (keybord.triggered(80)) {
                m_cursor--;
            } else if (keybord.triggered(79)) {
                m_cursor++;
            } else if (keybord.triggered(82)) {
                if (!m_history.use) {
                    m_history.index = m_history.items.size() - 1;
                    m_history.use = true;
                } else if (m_history.index > 0) {
                    m_history.index = std::clamp(m_history.index - 1, 0U, m_history.items.size() - 1);
                }

                m_input = m_history.items[m_history.index];
                m_cursor = m_input.size();
            } else if (keybord.triggered(81)) {

                if (m_history.use) {
                    m_history.index = std::clamp(m_history.index + 1, 0U, m_history.items.size());
                    if (m_history.index == m_history.items.size()) {
                        m_input.clear();
                    } else {
                        m_input = m_history.items[m_history.index];
                        m_cursor = m_input.size();
                    }
                } else {
                    m_input.clear();
                }
            }

            sanitize();
        }
    }

    void ConsoleView::draw() {
        app->graphic.clear(0);

        auto linesHeight = measureLines();
        auto inputHeight = measureInput();
        auto x = 0, y = std::min(0, 112 - linesHeight);

        if (inputHeight > 8) {
            y -= (inputHeight - 8);
        }

        for (auto ln = 0U; ln < m_lines.size(); ln++) {
            auto &line = m_lines[ln];
            auto &content = line.content;
            auto &prefix = line.prefix;
            auto &color = line.color;

            if (prefix) {
                app->graphic.drawChar('>', x, y, color);
                x = 8;
            }

            for (const auto &ch : content) {
                app->graphic.drawChar(ch, x, y, color);
                x += 4;

                if (x > 120) {
                    x = prefix ? 8 : 0;
                    y += 8;
                }
            }

            x = 0;
            y += 8;
        }

        app->graphic.drawChar('>', 0, y);
        x = 8;

        for (auto i = 0U; i <= m_input.size(); i++) {
            if (m_cursor == i) {
                app->graphic.drawRect(x, y, 1, 7, 3);
            }
            if (i < m_input.size()) {
                app->graphic.drawChar(m_input[i], x, y);
                x += 4;
                if (x > 120) {
                    x = 8;
                    y += 8;
                }
            }
        }
    }

    void ConsoleView::enter() {
        app->signal.join(SIGNAL_START_INPUT);

        if (m_first_time) {
            m_first_time = false;
            clear();
        }
    }

    void ConsoleView::leave() {}

    void ConsoleView::join(const std::string &text, bool prefix, uint8_t color) {
        m_lines.push_back({"",
                           color,
                           prefix});
        for (auto it = text.begin(); it != text.end(); it++) {
            const auto ch = *it;

            if (ch == '\n') {
                m_lines.push_back({"", color, prefix});
            } else if (ch != '\r') {
                m_lines.back().content.push_back(ch);
            }
        }
    }

    bool ConsoleView::command() {
        if (m_input.empty())
            return false;

        auto payload = utils::split(m_input, ' ');

        for (auto &str : payload) {
            utils::ltrim(str);
            utils::rtrim(str);
        }

        if (payload[0].empty())
            return false;

        if (utils::equals(payload[0], "help")) {
            if (validate(payload, 1)) {
                join("Commands:", false, 6);
                join("");
                join("load <filename>");
                join("run");
                join("save <filename>");
                join("cls");
                join("");
                join("Press esc to editor view");
            }
        }

        if (utils::equals(payload[0], "cls")) {
            if (validate(payload, 1))
                clear();
        }

        if (utils::equals(payload[0], "load")) {
            if (validate(payload, 2))
                load(payload[1]);
        }

        if (utils::equals(payload[0], "save") && payload.size() > 1) {
            if (validate(payload, 2))
                save(payload[1]);
        }

        if (utils::equals(payload[0], "run")) {
            if (validate(payload, 1))
                app->signal.join(SIGNAL_SWAP_EXEC);
        }

        join("Invalid syntax", false, 3);

        return true;
    }

    void ConsoleView::sanitize() {
        m_cursor = std::clamp(m_cursor, 0U, m_input.size());
    }

    int ConsoleView::measureHeight(const Line &line) {
        const auto lineWidth = line.prefix ? 29 : 31;

        if (line.content.empty())
            return 8;

        return static_cast<int>(std::ceil(line.content.size() / static_cast<float>(lineWidth))) << 3;
    }

    int ConsoleView::measureLines() {
        auto height = 0;
        for (auto &line : m_lines) {
            height += measureHeight(line);
        }

        return height;
    }

    int ConsoleView::measureInput() {
        return static_cast<int>(std::ceil(m_input.size() / 29)) << 3;
    }

    void ConsoleView::load(const std::string &filename) {
        constexpr auto headerSize = sizeof(CartFileHeader);
        constexpr auto metaSize = sizeof(CartMetaData);

        std::ifstream in(filename, std::ios::in | std::ios::binary);
        in.unsetf(std::ios::skipws);

        if (!in.good()) {
            join("Faild to open.", false, 3);
            return;
        }

        in.seekg(0, in.end);
        auto length = in.tellg();
        in.seekg(0, in.beg);

        CartFileHeader header;

        if (length < headerSize) {
            in.close();
            join("File size not match.", false, 3);
            return;
        }

        in.read(reinterpret_cast<char *>(&header), headerSize);

        bool shouldSwap = (utils::is_big_endian() != header.bigEndian);

        if (shouldSwap) {
            utils::swapBytes(header.magic, header.hash, header.count);
        }

        if (header.magic != 't8') {
            in.close();
            join("File header not match.", false, 3);
            return;
        }

        if (length < header.count * metaSize) {
            in.close();
            join("File size not match.", false, 3);
            return;
        }

        std::vector<CartMetaData> metas;

        for (auto i = 0; i < header.count; i++) {
            CartMetaData meta;
            in.read(reinterpret_cast<char *>(&meta), metaSize);

            if (shouldSwap) {
                utils::swapBytes(meta.size, meta.type);
            }

            metas.push_back(meta);
        }

        auto dataSize = std::accumulate(metas.begin(), metas.end(), 0, [](int prev, CartMetaData it) {
            return prev + it.size;
        });

        if (length < headerSize + header.count * metaSize + dataSize) {
            in.close();
            join("File size not match.", false, 3);
            return;
        }

        for (const auto &meta : metas) {
            if (meta.type == MetaType::Font) {
                uint8_t buf[0x800]{0};
                in.read(reinterpret_cast<char *>(buf), std::min(0x800U, meta.size));
                std::copy(buf, buf + 0x800, app->context.fonts[1]);
            }

            if (meta.type == MetaType::Flag) {
                uint8_t buf[0x8]{0};
                in.read(reinterpret_cast<char *>(buf), std::min(0x8U, meta.size));
                std::copy(buf, buf + 0x8, app->context.flag);
            }

            if (meta.type == MetaType::Map) {
                uint8_t buf[0x2000]{0};
                in.read(reinterpret_cast<char *>(buf), std::min(0x2000U, meta.size));
                std::copy(buf, buf + 0x2000, app->context.map);
            }

            if (meta.type == MetaType::Sprite) {
                uint8_t buf[0x2000]{0};
                in.read(reinterpret_cast<char *>(buf), std::min(0x2000U, meta.size));
                std::copy(buf, buf + 0x2000, app->context.sprite);
            }

            if (meta.type == MetaType::Script) {
                std::vector<char> buf(meta.size, '\0');
                in.read(buf.data(), meta.size);
                app->context.script = buf.data();
            }
        }

        in.close();

        join("Cartridge loaded.", false, 6);
    }

    void ConsoleView::save(const std::string &filename) {
        constexpr auto headerSize = sizeof(CartFileHeader);
        constexpr auto metaSize = sizeof(CartMetaData);
        constexpr auto metaCount = 5;

        std::ofstream out(filename, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.good()) {
            join("Failed to save.", false, 3);
            return;
        }

        CartFileHeader header;
        CartMetaData meta;

        header.magic = 't8';
        header.hash = std::hash<std::string>{}(app->context.script);
        header.bigEndian = utils::is_big_endian();
        header.count = metaCount;

        out.write(reinterpret_cast<char *>(&header), headerSize);

        meta = {
            MetaType::Sprite,
            0x2000u};
        out.write(reinterpret_cast<char *>(&meta), metaSize);

        meta = {
            MetaType::Flag,
            0x8};
        out.write(reinterpret_cast<char *>(&meta), metaSize);

        meta = {
            MetaType::Font,
            0x800u};
        out.write(reinterpret_cast<char *>(&meta), metaSize);

        meta = {
            MetaType::Map,
            0x2000u};
        out.write(reinterpret_cast<char *>(&meta), metaSize);

        meta = {
            MetaType::Script,
            static_cast<uint32_t>(app->context.script.size())};
        out.write(reinterpret_cast<char *>(&meta), metaSize);

        out.write(reinterpret_cast<const char *>(app->context.sprite), 0x2000);
        out.write(reinterpret_cast<const char *>(app->context.flag), 0x8);
        out.write(reinterpret_cast<const char *>(app->context.fonts[1]), 0x800);
        out.write(reinterpret_cast<const char *>(app->context.map), 0x2000);
        out.write(app->context.script.data(), app->context.script.size());

        out.flush();
        out.close();

        join("Cartridge saved.", false, 6);
    }

    bool ConsoleView::validate(const std::vector<std::string> payload, uint32_t count) {
        if (payload.size() < count) {
            join("Too few argument", false, 3);
            return false;
        }
        if (payload.size() > count) {
            join("Too many argument", false, 3);
            return false;
        }

        return true;
    }

    void ConsoleView::clear() {
        m_lines.clear();

        join("T8Y FANTASTIC CONSOLE", false, 3);
        join("");
        join("Type help for help");
    }
}