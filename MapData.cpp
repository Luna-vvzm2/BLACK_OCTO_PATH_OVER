
#include "MapData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <exception>

// CSVを2次元配列として読み込む
static bool ReadCSV(
    const std::string& filename,
    std::vector<std::vector<int>>& data)
{
    std::ifstream ifs(filename);

    if (!ifs.is_open())
    {
        std::cerr << "CSV読み込み失敗: "
            << filename << std::endl;
        return false;
    }

    std::string line;
    int lineNo = 0;

    while (std::getline(ifs, line))
    {
        lineNo++;

        if (line.empty()) continue;

        // UTF-8 BOMを除去
        if (lineNo == 1 &&
            line.size() >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF)
        {
            line.erase(0, 3);
        }

        std::stringstream ss(line);
        std::string cell;

        std::vector<int> row;
        int columnNo = 0;

        while (std::getline(ss, cell, ','))
        {
            columnNo++;

            // 前後の空白を除去
            size_t start = cell.find_first_not_of(" \t\r\n");

            if (start == std::string::npos)
            {
                row.push_back(0);
                continue;
            }

            size_t end = cell.find_last_not_of(" \t\r\n");
            cell = cell.substr(start, end - start + 1);

            try
            {
                size_t pos = 0;
                int value = std::stoi(cell, &pos);

                if (pos != cell.size())
                {
                    std::cerr << "不正なCSVデータ: "
                        << filename
                        << " 行:" << lineNo
                        << " 列:" << columnNo
                        << std::endl;
                    return false;
                }

                row.push_back(value);
            }
            catch (const std::exception& e)
            {
                std::cerr << "CSV変換エラー: "
                    << filename
                    << " 行:" << lineNo
                    << " 列:" << columnNo
                    << " 内容:" << cell
                    << " 詳細:" << e.what()
                    << std::endl;

                return false;
            }
        }

        if (!row.empty())
        {
            data.push_back(std::move(row));
        }
    }

    return !data.empty();
}


// CSVからレイヤーを追加
bool MapData::AddLayerFromCSV(
    StageData& stage,
    const std::string& filename,
    const std::string& layerName)
{
    std::vector<std::vector<int>> data;

    if (!ReadCSV(filename, data))
    {
        return false;
    }

    Layer layer;
    layer.name = layerName;
    layer.visible = true;

    int width = static_cast<int>(data[0].size());
    int height = static_cast<int>(data.size());

    for (const auto& row : data)
    {
        if (static_cast<int>(row.size()) != width)
        {
            std::cerr << "CSVの列数が一致しません: "
                << filename << std::endl;
            return false;
        }

        layer.tiles.insert(
            layer.tiles.end(),
            row.begin(),
            row.end()
        );
    }

    if (stage.width == 0)
    {
        stage.width = width;
        stage.height = height;
    }

    stage.layers.push_back(std::move(layer));

    return true;
}


// ステージ読み込み
bool MapData::LoadStage(const std::string& stageFolder)
{
    StageData stage;

    // レイヤー読み込み処理
    auto loadLayer = [&](const std::string& prefix,
        const std::string& layerName) -> bool
        {
            Layer layer;
            layer.name = layerName;
            layer.visible = true;

            int totalWidth = 0;
            int height = 0;

            // CSV番号01～03を順番に読み込む
            for (int partNo = 1; partNo <= 3; partNo++)
            {
                std::string filename =
                    stageFolder + "/" +
                    prefix + "_01_0" +
                    std::to_string(partNo) + ".csv";

                std::vector<std::vector<int>> data;

                if (!ReadCSV(filename, data))
                {
                    return false;
                }

                int currentWidth =
                    static_cast<int>(data[0].size());

                int currentHeight =
                    static_cast<int>(data.size());

                // 高さの一致を確認
                if (height != 0 && height != currentHeight)
                {
                    std::cerr << "CSVの高さが一致しません: "
                        << filename << std::endl;
                    return false;
                }

                height = currentHeight;

                // 各行の列数を確認
                for (const auto& row : data)
                {
                    if (static_cast<int>(row.size()) != currentWidth)
                    {
                        std::cerr << "CSVの列数が一致しません: "
                            << filename << std::endl;
                        return false;
                    }
                }

                // 横方向に連結
                if (totalWidth == 0)
                {
                    for (const auto& row : data)
                    {
                        layer.tiles.insert(
                            layer.tiles.end(),
                            row.begin(),
                            row.end()
                        );
                    }
                }
                else
                {
                    std::vector<int> combined;

                    for (int y = 0; y < height; y++)
                    {
                        // 既存データ
                        int oldStart = y * totalWidth;

                        combined.insert(
                            combined.end(),
                            layer.tiles.begin() + oldStart,
                            layer.tiles.begin() + oldStart + totalWidth
                        );

                        // 新しいCSVデータ
                        combined.insert(
                            combined.end(),
                            data[y].begin(),
                            data[y].end()
                        );
                    }

                    layer.tiles = std::move(combined);
                }

                totalWidth += currentWidth;
            }

            // Mapレイヤーのサイズを設定
            if (layerName == "Map")
            {
                stage.width = totalWidth;
                stage.height = height;
            }

            stage.layers.push_back(std::move(layer));

            return true;
        };

    // マップデータ読み込み
    if (!loadLayer("map", "Map"))
    {
        return false;
    }

    // オブジェクトデータ読み込み
    if (!loadLayer("obj", "Object"))
    {
        return false;
    }

    // ステージを追加
    stages.push_back(std::move(stage));

    return true;
}
