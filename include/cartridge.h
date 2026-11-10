#pragma once
#include <string>

namespace t8::core
{
    struct AppContext;
}

namespace t8::cartridge
{
    using namespace t8::core;

    bool load(const std::string &name, AppContext *ctx);

    bool save(const std::string &name, const AppContext *ctx);
}