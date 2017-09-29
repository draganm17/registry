#pragma once

#include <locale>
#include <string>
#include <string_view>


namespace registry {
namespace details {

    template <typename Source>
    std::wstring to_native(const Source& str, const std::locale& loc = std::locale(""));

    template <typename InputIt>
    std::wstring to_native(InputIt first, InputIt last, const std::locale& loc = std::locale(""));

}} // namespace registry::details