#include "MapData.h"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    const auto root = std::filesystem::current_path();
    assert(!std::filesystem::exists(root / "assets/events/event_106.csv"));
    {
        std::ifstream event(root / "assets/events/event_103.csv");
        std::string line;
        int clearCommands = 0;
        while (std::getline(event, line))
            if (line.rfind("CLEAR,", 0) == 0) ++clearCommands;
        assert(clearCommands == 1);
    }
    MapData maps;
    const int expectedWidths[] = { 250, 210, 27 };
    for (int stage = 1; stage <= 3; ++stage)
    {
        assert(maps.LoadStage("assets/maps/stage" + std::to_string(stage), stage));
        const auto& data = maps.stages.back();
        assert(data.width == expectedWidths[stage - 1] && data.height == 15);
        assert(data.layers.size() == 2);
        for (const auto& layer : data.layers)
            assert(layer.tiles.size() == static_cast<size_t>(data.width * data.height));
        for (int id : data.layers[1].tiles)
        {
            assert(id != 106);
            if (id < 100 || id >= 200) continue;
            const auto event = root / ("assets/events/event_" + std::to_string(id) + ".csv");
            assert(std::filesystem::exists(event));
            assert(std::filesystem::file_size(event) > 0);
        }
    }
    const auto scratch = std::filesystem::temp_directory_path() /
        ("shinobi-map-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(scratch);
    const auto write = [&](const std::string& file, const std::string& text)
    {
        std::ofstream out(scratch / file, std::ios::binary);
        out << text;
    };
    for (int part = 1; part <= 3; ++part)
    {
        const std::string suffix = "_09_0" + std::to_string(part) + ".csv";
        write("map" + suffix, part == 1 ? "\xEF\xBB\xBF 1,2\n3,4\n" : "5,6\n7,8\n");
        write("obj" + suffix, "-1,-1\n-1,-1\n");
    }
    MapData split;
    assert(split.LoadStage(scratch.string(), 9));
    const auto& combined = split.stages.back();
    assert(combined.width == 6 && combined.height == 2);
    assert((combined.layers[0].tiles == std::vector<int>{1, 2, 5, 6, 5, 6, 3, 4, 7, 8, 7, 8}));

    // A newer complete map must take priority over older split assets.
    write("map.csv", "10,11\n12,13\n");
    write("obj.csv", "-1,-1\n-1,-1\n");
    MapData complete;
    assert(complete.LoadStage(scratch.string(), 9));
    assert((complete.stages[0].layers[0].tiles == std::vector<int>{10, 11, 12, 13}));
    write("obj.csv", "-1\n-1\n");
    MapData mismatched;
    assert(!mismatched.LoadStage(scratch.string(), 9));
    assert(mismatched.stages.empty());
    std::filesystem::remove(scratch / "map.csv");
    std::filesystem::remove(scratch / "obj.csv");
    write("obj_09_03.csv", "-1\n-1\n");
    MapData splitMismatch;
    assert(!splitMismatch.LoadStage(scratch.string(), 9));
    assert(splitMismatch.stages.empty());
    for (int part = 1; part <= 3; ++part)
        for (const auto* prefix : {"map", "obj"})
            std::filesystem::remove(scratch / (std::string(prefix) + "_09_0" + std::to_string(part) + ".csv"));
    std::filesystem::remove(scratch);
    std::cout << "Map loading, layout and event integration tests passed.\n";
}
