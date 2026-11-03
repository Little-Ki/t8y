#pragma once
#include <string>
#include <vector>

namespace t8::utils {
    void str_ltrim(std::string &s);

    void str_rtrim(std::string &s);

    bool str_equals(const std::string &a, const std::string &b, bool ignore_case = false);

    std::vector<std::string> str_explode(const std::string &text, char token);    
}