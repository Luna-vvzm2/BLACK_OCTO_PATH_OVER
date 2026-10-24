#pragma once
#include <DxLib.h>

class NewspaperUI {
public:
    void Draw() const;

private:
    static constexpr int kScreenW = 1920;
    static constexpr int kScreenH = 1080;
    static constexpr int kLeftW = kScreenW / 3;   
};