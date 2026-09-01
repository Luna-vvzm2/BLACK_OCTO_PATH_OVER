#pragma once
#include <string>
#include <vector>

struct Layer {
    std::string name;
    bool visible = false;
    std::vector<int> tiles;
};

struct StageData {
    int width = 0;
    int height = 0;

    std::vector<Layer>layers;
};

class MapData {
public:
    std::vector<StageData> stages;
    int tileSize = 32;

    // CSV Ç©ÇÁì«Ç›çûÇ›
    bool AddLayerFromCSV(StageData& stage, const std::string& filename, const std::string& layerName);
    bool LoadStage(const std::string& stageFolder);
};
