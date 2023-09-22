#include "../include/mpv.hpp"
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

static bool exec_program_daemonized(const char **args) {
    /* 1 argument */
    if(args[0] == nullptr)
        return false;

    pid_t pid = vfork();
    if(pid == -1) {
        perror("Failed to vfork");
        return false;
    } else if(pid == 0) { /* child */
        setsid();
        signal(SIGHUP, SIG_IGN);

        // Daemonize child to make the parent the init process which will reap the zombie child
        pid_t second_child = vfork();
        if(second_child == 0) { // child
            execvp(args[0], (char* const*)args);
            perror("execvp");
            _exit(127);
        } else if(second_child != -1) {
            // TODO:
            _exit(0);
        }
    } else { /* parent */
        waitpid(pid, nullptr, 0); 
    }

    return true;
}

static void show_notification(const char *title, const char *msg, const char *urgency) {
    const char *args[] = { "notify-send", "-a", "vr-video-player", "-t", "10000", "-u", urgency, "--", title, msg, NULL };
    exec_program_daemonized(args);
}

static void* get_proc_address_mpv(void*, const char *name) {
    return SDL_GL_GetProcAddress(name);
}

static void on_mpv_events(void *ctx) {
    Mpv *mpv = (Mpv*)ctx;
    SDL_Event event;
    event.type = mpv->wakeup_on_mpv_events;
    SDL_PushEvent(&event);
}

static void on_mpv_render_update(void *ctx) {
    Mpv *mpv = (Mpv*)ctx;
    SDL_Event event;
    event.type = mpv->wakeup_on_mpv_render_update;
    SDL_PushEvent(&event);
}

Mpv::~Mpv() {
    destroy();
}

bool Mpv::create(bool use_system_mpv_config) {
    if(created)
        return false;

    mpv = mpv_create();
    if(!mpv) {
        fprintf(stderr, "Error: mpv_create failed\n");
        return false;
    }

    if(use_system_mpv_config) {
        mpv_set_option_string(mpv, "config", "yes");
        mpv_set_option_string(mpv, "load-scripts", "yes");
    }

    if(mpv_initialize(mpv) < 0) {
        fprintf(stderr, "Error: mpv_initialize failed\n");
        mpv_destroy(mpv);
        mpv = nullptr;
        return false;
    }

    //mpv_request_log_messages(mpv, "debug");

    mpv_opengl_init_params gl_init_params;
    memset(&gl_init_params, 0, sizeof(gl_init_params));
    gl_init_params.get_proc_address = get_proc_address_mpv;

    int advanced_control = 1;

    mpv_render_param params[] = {
        { MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params },
        { MPV_RENDER_PARAM_ADVANCED_CONTROL, &advanced_control },
        //{ MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, 0 }, // TODO: Manually sync after this
        { MPV_RENDER_PARAM_INVALID, 0 }
    };

    //mpv_set_option_string(mpv, "vd-lavc-dr", "yes");
    mpv_set_option_string(mpv, "vo", "libmpv");
    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "profile", "gpu-hq");
    mpv_set_option_string(mpv, "gpu-api", "opengl");

    if(mpv_render_context_create(&mpv_gl, mpv, params) < 0) {
        fprintf(stderr, "Error: mpv_render_context_create failed\n");
        mpv_destroy(mpv);
        mpv = nullptr;
        mpv_gl = nullptr;
        return false;
    }

    wakeup_on_mpv_render_update = SDL_RegisterEvents(1);
    wakeup_on_mpv_events = SDL_RegisterEvents(1);
    if(wakeup_on_mpv_render_update == (uint32_t)-1 || wakeup_on_mpv_events == (uint32_t)-1) {
        fprintf(stderr, "Error: SDL_RegisterEvents failed\n");
        // TODO: Remove registered events?
        wakeup_on_mpv_render_update = -1;
        wakeup_on_mpv_events = -1;
        mpv_render_context_free(mpv_gl);
        mpv_destroy(mpv);
        mpv = nullptr;
        mpv_gl = nullptr;
        return false;
    }

    mpv_set_wakeup_callback(mpv, on_mpv_events, this);
    mpv_render_context_set_update_callback(mpv_gl, on_mpv_render_update, this);

    created = true;
    return true;
}

bool Mpv::destroy() {
    if(!created)
        return true;

    if(mpv_gl)
        mpv_render_context_free(mpv_gl);
    if(mpv)
        mpv_destroy(mpv);

    created = false;
    return true;
}

bool Mpv::load_file(const char *path) {
    if(!created)
        return false;

    const char *cmd[] = { "loadfile", path, nullptr };
    mpv_command_async(mpv, 0, cmd);
    return true;
}

void Mpv::on_event(SDL_Event &event, bool *render_update, int64_t *width, int64_t *height, bool *quit, int *error) {
    if(render_update)
        *render_update = false;

    if(width)
        *width = 0;

    if(height)
        *height = 0;

    if(quit)
        *quit = false;

    if(error)
        *error = 0;

    if(!created)
        return;

    if(event.type == wakeup_on_mpv_render_update) {
        uint64_t flags = mpv_render_context_update(mpv_gl);
        if(flags & MPV_RENDER_UPDATE_FRAME) {
            if(render_update)
                *render_update = true;
        }
    }

    if(event.type == wakeup_on_mpv_events) {
        while(true) {
            mpv_event *mp_event = mpv_wait_event(mpv, 0);
            if(mp_event->event_id == MPV_EVENT_NONE)
                break;
            
            if(mp_event->event_id == MPV_EVENT_LOG_MESSAGE) {
                mpv_event_log_message *msg = (mpv_event_log_message*)mp_event->data;
                //printf("log: %s", msg->text);
            } else {
                //printf("mpv event: %s\n", mpv_event_name(mp_event->event_id));
            }

            if(mp_event->event_id == MPV_EVENT_END_FILE) {
                mpv_event_end_file *msg = (mpv_event_end_file*)mp_event->data;
                if(msg->reason == MPV_END_FILE_REASON_ERROR) {
                    show_notification("vr video player mpv video error", mpv_error_string(msg->error), "critical");
                    if(quit) {
                        *quit = true;
                        *error = -1;
                    }
                }
                if(msg->reason == MPV_END_FILE_REASON_EOF) {
                    show_notification("vr video player", "the video ended", "low");
                    if(quit)
                        *quit = true;
                }
            }

            if(mp_event->event_id == MPV_EVENT_VIDEO_RECONFIG) {
                int64_t new_width = 0;
                mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &new_width);

                int64_t new_height = 0;
                mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &new_height);
                
                if(width)
                    *width = new_width;

                if(height)
                    *height = new_height;
            }
        }
    }
}

void Mpv::seek(double seconds) {
    if(!created)
        return;

    char seconds_str[128];
    snprintf(seconds_str, sizeof(seconds_str), "%f", seconds);

    const char *cmd[] = { "seek", seconds_str, nullptr };
    mpv_command_async(mpv, 0, cmd);
}

void Mpv::toggle_pause() {
    if(!created)
        return;

    paused = !paused;
    int pause_value = paused ? 1 : 0;
    mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &pause_value);
}

void Mpv::draw(unsigned int framebuffer_id, int width, int height) {
    if(!created)
        return;

    mpv_opengl_fbo fbo;
    memset(&fbo, 0, sizeof(fbo));
    fbo.fbo = framebuffer_id;
    fbo.w = width;
    fbo.h = height;

    int flip_y = 0;
    int shit = 1;

    mpv_render_param params[] = {
        { MPV_RENDER_PARAM_OPENGL_FBO, &fbo },
        { MPV_RENDER_PARAM_FLIP_Y, &flip_y },
        //{ MPV_RENDER_PARAM_SKIP_RENDERING, &shit },
        { MPV_RENDER_PARAM_INVALID, 0 }
    };

    int res = mpv_render_context_render(mpv_gl, params);
    //fprintf(stderr, "draw mpv: %d\n", res);
}