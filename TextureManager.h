#pragma once
#include <unordered_map>
#include <string>

class TextureManager
{
public:

    static void ReleaseTextures(
        std::unordered_map<std::string, int>& cache
    );
};