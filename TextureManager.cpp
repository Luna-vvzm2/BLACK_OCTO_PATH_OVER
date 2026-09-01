#include "TextureManager.h"
#include <DxLib.h>
#include <iostream>

void TextureManager::ReleaseTextures(std::unordered_map<std::string, int>& cache) {
        std::cout
            << "cache size="
            << cache.size()
            << std::endl;

        for (auto& tex : cache)
        {
            std::cout
                << tex.first
                << " : "
                << tex.second
                << std::endl;

            if (tex.second != -1)
            {
                DeleteGraph(tex.second);
            }
        }

        cache.clear();
}