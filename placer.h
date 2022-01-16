#pragma once

#include <filesystem>

namespace placer
{
    void next( std::filesystem::path root );
    void summary( std::filesystem::path root );
    void print_placement( std::filesystem::path );
}