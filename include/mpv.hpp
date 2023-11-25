#pragma once

#include <stdint.h>
#include <SDL.h>

typedef struct mpv_handle mpv_handle;
typedef struct mpv_render_context mpv_render_context;

class Mpv {
public:
    Mpv() = default;
    ~Mpv();

    bool create(bool use_system_mpv_config, const char *profile = "gpu-hq");
    bool destroy();

    bool load_file(const char *path);
    // |width| and |ħeight| are set to 0 unless there is an event to reconfigure video size
    void on_event(SDL_Event &event, bool *render_update, int64_t *width, int64_t *height, bool *quit, int *error);
    void seek(double seconds);
    void toggle_pause();
    void draw(unsigned int framebuffer_id, int width, int height);

    bool created = false;
    uint32_t wakeup_on_mpv_render_update = -1;
    uint32_t wakeup_on_mpv_events = -1;

    mpv_handle *mpv = nullptr;
    mpv_render_context *mpv_gl = nullptr;
    bool paused = false;
};