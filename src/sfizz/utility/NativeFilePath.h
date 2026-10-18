// SPDX-License-Identifier: BSD-2-Clause
#pragma once

#include <ghc/fs_std.hpp>
#include <system_error>

namespace sfz {

inline fs::path nativeFilePath(const fs::path& path)
{
#if defined(_WIN32)
    if (path.empty())
        return path;
    std::error_code error;
    auto absolute = fs::absolute(path, error);
    if (error)
        return path;
    absolute = absolute.lexically_normal();
    absolute.make_preferred();
    const auto& spelling = absolute.native();
    if (spelling.compare(0, 4, L"\\\\?\\") == 0)
        return absolute;
    if (spelling.compare(0, 2, L"\\\\") == 0)
        return fs::path(L"\\\\?\\UNC\\" + spelling.substr(2));
    return fs::path(L"\\\\?\\" + spelling);
#else
    return path;
#endif
}

} // namespace sfz
