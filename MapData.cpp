#include "MapData.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool MapData::AddLayerFromCSV(StageData& stage, const std::string& filename, const std::string& layerName) {
    std::ifstream ifs(filename);

    if (!ifs.is_open())
    {
        std::cerr << "CSV“Ç‚Ýž‚ÝŽ¸”s: "
            << filename << std::endl;
        return false;
    }

    Layer layer;
    layer.name = layerName;
    layer.visible = true;

    std::string line;

    int y = 0;
    int localWidth = 0;

    while (std::getline(ifs, line))
    {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cell;

        int x = 0;

        while (std::getline(ss, cell, ','))
        {
            layer.tiles.push_back(std::stoi(cell));
            x++;
        }

        localWidth = x;
        y++;
    }

    if (stage.width == 0)
    {
        stage.width = localWidth;
        stage.height = y;
    }

    stage.layers.push_back(std::move(layer));

    return true;
}

bool MapData::LoadStage(const std::string& stageFolder) {
    StageData stage;

    if (!AddLayerFromCSV(
        stage,
        stageFolder + "/map.csv",
        "Map"))
    {
        return false;
    }

    if (!AddLayerFromCSV(
        stage,
        stageFolder + "/obj.csv",
        "Object"))
    {
        return false;
    }

    stages.push_back(
        std::move(stage)
    );

    return true;
}