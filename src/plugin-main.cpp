/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/
#include <vector>

#include <obs-module.h>

#include "plugin-macros.generated.h"

#include <random>
#include <fstream>
#include <graphics/image-file.h>
#include <util/dstr.h>

#include "plugin-types.h"

using namespace Stinger3D;

class stinger_3D_transition {
public:
    stinger_3D_transition(obs_data_t *settings, obs_source_t *source);

    static void registerInputSource();

private:
    gs_effect_t *effect;
    obs_source_t *source;
    obs_source_t *media_source;
    gs_eparam_t *aparam;
    gs_eparam_t *bparam;
    gs_eparam_t *tparam;

    Transition transition;

    void render(gs_effect_t *effect);

    bool
    render_audio(uint64_t *ts_out, obs_source_audio_mix *audio, uint32_t mixers, size_t channels, size_t sample_rate);

    void transition_callback(gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx, uint32_t cy);

    void update(obs_data_t *settings);

    void startTransition();

    void endTransition();

    obs_properties_t *getProperties();

    static void getDefaults(obs_data_t *);

    float delay;

    std::string videoPath;
};

obs_source_info SOURCE_2;

static float ease(float t, float offset = 0.0f, float amplitude = 4.0f) {
    float t2 = (t - offset) * amplitude;
    return -(cos(M_PI * t2) - 1.0) / 2.0; // sinus
    //return t2 < 0.5 ? 2.0 * t2 * t2 : 1.0 - powf(-2.0 * t2 + 2.0, 2.0) / 2.0; // quadratic
    //return t2; // linear
}

#define DISTANCE 1.0

void stinger_3D_transition::transition_callback(gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx, uint32_t cy) {
    if (t < delay) {
        t = 0;
    } else {
        t -= delay;
    }
    gs_projection_push();
    gs_perspective(90, 1280.0 / 720.0, 1.0f / 2097152.0f, 2097152.0f);
    gs_matrix_push();
    gs_matrix_scale3f(1.0 * 1280.0 / 720.0, 1.0, 1.0);

    transition.transforms->render_frame(t);

    //center image, camera at one unit of distance from the center
    //gs_matrix_translate3f(-1, -1, -1.0);

    const bool previous = gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(true);

    gs_effect_set_texture(aparam, t < transition.swap_time ? a : b);
    gs_effect_set_texture(bparam, b);
    gs_effect_set_float(tparam, t);

    while (gs_effect_loop(effect, "Transition"))
        gs_draw_sprite(NULL, 0, 2, 2);


    gs_enable_framebuffer_srgb(previous);
    gs_matrix_pop();
    gs_projection_pop();
}

void stinger_3D_transition::render(gs_effect_t *effect) {
    obs_transition_video_render(this->source, [](void *data, gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx,
                                                 uint32_t cy) {
        static_cast<stinger_3D_transition *>(data)->transition_callback(a, b, t, cx, cy);
    });
    gs_matrix_push();
    //gs_matrix_scale3f(1.0f, 1.0f, 1.0f);
    obs_source_video_render(media_source);
    gs_matrix_pop();
}

void stinger_3D_transition::update(obs_data_t *settings) {
    blog(LOG_INFO, "update transition");

    //descriptor parsing
    std::string descriptor_filename(obs_data_get_string(settings, "descriptor"));
    std::fstream file;
    file.open(descriptor_filename, std::fstream::in);

    std::string descriptor_string(std::istreambuf_iterator<char>{file}, {});

    try {
        json::json descriptor_json = json::json::parse(descriptor_string);
        transition = descriptor_json.get<Transition>();
    }

    catch (json::basic_json<>::exception err) {
        blog(LOG_ERROR, "Could not parse json parameters : %s", err.what());
    }


    delay = (float) obs_data_get_int(settings, "delay") / 1000.0;
    //Media initialization

    auto newPath = std::string(obs_data_get_string(settings, "Video file"));

    if (newPath != videoPath) {
        blog(LOG_INFO, "Refreshing the source...");
        obs_source_release(media_source);
        obs_data_t *media_settings = obs_data_create();
        obs_data_set_string(media_settings, "local_file", newPath.c_str());
        obs_data_set_bool(media_settings, "hw_decode", false);

        media_source = obs_source_create_private("ffmpeg_source", "transition video", media_settings);

        obs_data_release(media_settings);

        videoPath = newPath;
    }


}

void stinger_3D_transition::registerInputSource() {
    obs_source_info *source_2 = &SOURCE_2;
    source_2->id = "3d_transition";
    source_2->type = OBS_SOURCE_TYPE_TRANSITION;
    source_2->get_name = [](void *) { return obs_module_text("TransitionName"); };
    source_2->create = [](obs_data_t *settings, obs_source_t *source) {
        return static_cast<void *>(new stinger_3D_transition(settings, source));
    };
    source_2->get_properties = [](void *data) { return static_cast<stinger_3D_transition *>(data)->getProperties(); };
    source_2->video_render = [](void *data, gs_effect_t *effect) {
        return static_cast<stinger_3D_transition *>(data)->render(effect);
    };
    source_2->audio_render = [](void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio, uint32_t mixers,
                                size_t channels, size_t sample_rate) {
        return static_cast<stinger_3D_transition *>(data)->render_audio(ts_out, audio, mixers, channels, sample_rate);
    };
    source_2->update = [](void *data, obs_data_t *settings) {
        static_cast<stinger_3D_transition *>(data)->update(settings);
    };
    source_2->destroy = [](void *data) { delete static_cast<stinger_3D_transition *>(data); };
    source_2->get_defaults = [](obs_data_t *data) {
        stinger_3D_transition::getDefaults(data);
    };
    source_2->transition_start = [](void *data) { static_cast<stinger_3D_transition *>(data)->startTransition(); };
    source_2->transition_stop = [](void *data) { static_cast<stinger_3D_transition *>(data)->endTransition(); };

    obs_register_source(source_2);

}

bool
stinger_3D_transition::render_audio(uint64_t *ts_out, obs_source_audio_mix *audio, uint32_t mixers, size_t channels,
                                    size_t sample_rate) {
    return obs_transition_audio_render(source, ts_out, audio, mixers, channels, sample_rate,
                                       [](void *, float t) { return 1 - t; },
                                       [](void *, float t) { return t; });
}

stinger_3D_transition::stinger_3D_transition(obs_data_t *settings, obs_source_t *source) {
    blog(LOG_INFO, "----- Creating Stream Transition...");
    gs_effect_t *effect;

    this->source = source;
    char *file = obs_module_file("transition.effect");
    if (file == NULL) {
        blog(LOG_ERROR, "file not found");
        return;
    }

    obs_enter_graphics();
    char **error_string = (char **) bmalloc(sizeof(char) * 2048);
    effect = gs_effect_create_from_file(file, error_string);
    obs_leave_graphics();

    this->effect = effect;
    this->aparam = gs_effect_get_param_by_name(effect, "tex_a");
    this->bparam = gs_effect_get_param_by_name(effect, "tex_b");
    this->tparam = gs_effect_get_param_by_name(effect, "t");

    bfree(file);
    blog(LOG_INFO, "Stream transition created !");

    obs_source_update(this->source, settings);
    this->media_source = nullptr;

    bfree(error_string);

}

void stinger_3D_transition::startTransition() {
    proc_handler_t *ph = obs_source_get_proc_handler(media_source);
    calldata_t cd = {0};
    proc_handler_call(ph, "restart", &cd);
    blog(LOG_INFO, "Video started !");
    obs_source_add_active_child(source, media_source);
}

void stinger_3D_transition::endTransition() {
    obs_source_remove_active_child(source, media_source);
}

obs_properties_t *stinger_3D_transition::getProperties() {
    auto properties = obs_properties_create();
    obs_properties_add_int(properties, "delay", "Delay between audio and animation (ms)", 0, 1000, 1);
    obs_properties_add_path(properties, "Video file", "Path to the video", OBS_PATH_FILE, NULL, NULL);
    obs_properties_add_path(properties, "descriptor", "JSON transform description", OBS_PATH_FILE, NULL, NULL);

    return properties;
}

void stinger_3D_transition::getDefaults(obs_data_t *data) {
    obs_data_set_default_int(data, "delay", 20);
}

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("3D stinger transition", "en-US")

OBS_MODULE_AUTHOR("ElDonad")

bool obs_module_load() {
    stinger_3D_transition::registerInputSource();
    return true;
}