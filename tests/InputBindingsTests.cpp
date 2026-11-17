#include "Input.h"
#include "SaveManager.h"
#include "SaveData.h"
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

// Input devices are idle in these binding and save regression tests.
void Key::Init() {}
void Key::Update() {}
void Joypad::Init(int) {}
void Joypad::Update() {}

int main()
{
    Input input;
    assert(input.GetKeyBinding(Action::OCTO) == Key::Q);
    assert(input.GetKeyBinding(Action::TIGER) == Key::E);
    assert(input.GetKeyBinding(Action::SHADOW_HEAL) == Key::Z);

    std::array<std::vector<int>, SaveData::ActionCount> original;
    for (size_t i = 0; i < original.size(); ++i)
        original[i] = input.GetKeyBindings(static_cast<Action>(i));

    // Loading Q/E for map zoom must preserve the jutsu bindings.
    for (size_t i = 0; i < original.size(); ++i)
        input.SetKeyBindings(static_cast<Action>(i), original[i]);
    assert(input.GetKeyBinding(Action::OCTO) == Key::Q);
    assert(input.GetKeyBinding(Action::TIGER) == Key::E);

    // Rebinding still prevents conflicts between gameplay actions.
    assert(input.SetKeyBinding(Action::HEAL, Key::Q));
    assert(input.GetKeyBindings(Action::OCTO).empty());
    assert(input.GetKeyBinding(Action::ZOOM_IN) == Key::Q);
    std::array<std::vector<int>, SaveData::ActionCount> beforeEdit;
    for (size_t i = 0; i < beforeEdit.size(); ++i)
        beforeEdit[i] = input.GetKeyBindings(static_cast<Action>(i));
    input.ResetKeyBindings();
    for (size_t i = 0; i < beforeEdit.size(); ++i)
        input.SetKeyBindings(static_cast<Action>(i), beforeEdit[i]);
    for (size_t i = 0; i < beforeEdit.size(); ++i)
        assert(input.GetKeyBindings(static_cast<Action>(i)) == beforeEdit[i]);
    assert(!input.SetKeyBinding(Action::HEAL, Key::ESCAPE));
    assert(!input.SetKeyBinding(Action::HEAL, Key::ENTER));

    const auto previous = std::filesystem::current_path();
    const auto scratch = std::filesystem::temp_directory_path() /
        ("shinobi-input-tests-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(scratch / "save");
    std::filesystem::current_path(scratch);
    SaveData saved;
    saved.currentStage = 2;
    saved.playTime = 123.0f;
    saved.newsOwned[3] = true;
    for (size_t i = 0; i < original.size(); ++i)
        saved.keyBindings[i] = original[i];
    saved.keyBindings[static_cast<size_t>(Action::OCTO)].clear();
    saved.keyBindings[static_cast<size_t>(Action::TIGER)].clear();
    saved.keyBindings[static_cast<size_t>(Action::SHADOW_HEAL)].clear();
    assert(SaveManager::Save(saved));
    SaveData loaded;
    assert(SaveManager::Load(loaded));
    input.ResetKeyBindings();
    for (size_t i = 0; i < original.size(); ++i)
        if (loaded.hasKeyBinding[i])
            input.SetKeyBindings(static_cast<Action>(i), loaded.keyBindings[i]);
    assert(input.GetKeyBinding(Action::OCTO) == Key::Q);
    assert(input.GetKeyBinding(Action::TIGER) == Key::E);
    assert(input.GetKeyBinding(Action::SHADOW_HEAL) == Key::Z);
    assert(loaded.currentStage == 2 && loaded.playTime == 123.0f);
    assert(loaded.newsOwned[3]);
    std::filesystem::current_path(previous);
    std::filesystem::remove(scratch / "save/save.json");
    std::filesystem::remove(scratch / "save");
    std::filesystem::remove(scratch);
    std::cout << "Input binding and save regression tests passed.\n";
}
