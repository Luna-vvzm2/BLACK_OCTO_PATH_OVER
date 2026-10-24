#include <emscripten.h>
#include <emscripten/html5.h>
#include <algorithm>
#include <cstring>

namespace {
constexpr double width = 960.0, height = 540.0, size = 48.0;
double x = 456.0, y = 246.0, previous = 0.0;
bool left = false, right = false, up = false, down = false;

EM_JS(void, draw, (double px, double py), {
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d');
    ctx.fillStyle = '#101827';
    ctx.fillRect(0, 0, 960, 540);
    ctx.strokeStyle = '#243249';
    for (let i = 0; i <= 960; i += 48) {
        ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, 540); ctx.stroke();
    }
    for (let i = 0; i <= 540; i += 48) {
        ctx.beginPath(); ctx.moveTo(0, i); ctx.lineTo(960, i); ctx.stroke();
    }
    ctx.fillStyle = '#60dfcc';
    ctx.fillRect(px, py, 48, 48);
    document.getElementById('position').textContent =
        'X: ' + px.toFixed(0) + ' / Y: ' + py.toFixed(0);
});

EM_BOOL key(int type, const EmscriptenKeyboardEvent* event, void*) {
    const bool pressed = type == EMSCRIPTEN_EVENT_KEYDOWN;
    if (!std::strcmp(event->code, "ArrowLeft") || !std::strcmp(event->code, "KeyA")) left = pressed;
    else if (!std::strcmp(event->code, "ArrowRight") || !std::strcmp(event->code, "KeyD")) right = pressed;
    else if (!std::strcmp(event->code, "ArrowUp") || !std::strcmp(event->code, "KeyW")) up = pressed;
    else if (!std::strcmp(event->code, "ArrowDown") || !std::strcmp(event->code, "KeyS")) down = pressed;
    else return EM_FALSE;
    return EM_TRUE;
}
EM_BOOL blur(int, const EmscriptenFocusEvent*, void*) {
    left = right = up = down = false;
    previous = 0.0;
    return EM_FALSE;
}
void frame() {
    const double now = emscripten_get_now();
    const double dt = previous == 0.0 ? 0.0 : std::clamp((now - previous) / 1000.0, 0.0, 0.05);
    previous = now;
    double dx = static_cast<int>(right) - static_cast<int>(left);
    double dy = static_cast<int>(down) - static_cast<int>(up);
    if (dx != 0.0 && dy != 0.0) { dx *= 0.70710678118; dy *= 0.70710678118; }
    x = std::clamp(x + dx * 300.0 * dt, 0.0, width - size);
    y = std::clamp(y + dy * 300.0 * dt, 0.0, height - size);
    draw(x, y);
}
}
extern "C" EMSCRIPTEN_KEEPALIVE void reset_position() {
    x = 456.0; y = 246.0;
    left = right = up = down = false;
}
int main() {
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, key);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, key);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, blur);
    emscripten_set_main_loop(frame, 0, 1);
}
