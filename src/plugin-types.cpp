//
// Created by ElDonad on 21/06/2021.
//

#include "plugin-types.h"

#include <iostream>

namespace Stinger3D {

    float ease(float input, EaseType type, float amplitude, float offset, float start) {
        float t2 = (input - offset) * (amplitude - start);
        float t2_inv = (1 - (input - offset)) * (amplitude - start);
        float val;
        switch (type) {
            case LINEAR:
                return t2 + start;
            case SINUSOIDAL:
                val = -(amplitude - start) * (cos(M_PI * (input - offset)) - 1.0) / 2.0 + start;
                return val;
            case QUADRATIC:
                return t2 < 0.5 ? 2.0 * t2 * t2 : 1.0 - powf(-2.0 * t2 + 2.0, 2.0) / 2.0;
            case CONSTANT:
                return amplitude;
        }
    }

    std::variant<vec3, quat> Transformation::getFrame(float frame) {
        if (frame < begin_frame || frame > end_frame) {
            if (transformation == ROTATION)
                return std::variant<vec3, quat>(quat{0, 0, 0, 0});
            else if (transformation == SCALE)
                return std::variant<vec3, quat>(vec3{1, 1, 1});
            else
                return std::variant<vec3, quat>(vec3{0, 0, 0});
        } else {
            float advancement = (frame - begin_frame) / (end_frame - begin_frame);
            if (transformation == TRANSLATION) {
                return std::variant<vec3, quat>(vec3{
                        ease(advancement, easing, std::get<vec3>(parameters).x),
                        ease(advancement, easing, std::get<vec3>(parameters).y),
                        ease(advancement, easing, std::get<vec3>(parameters).z),
                });
            } else if (transformation == SCALE) {
                return std::variant<vec3, quat>(vec3{
                        ease(advancement, easing, std::get<vec3>(parameters).x, 0, 1),
                        ease(advancement, easing, std::get<vec3>(parameters).y, 0, 1),
                        ease(advancement, easing, std::get<vec3>(parameters).z, 0, 1),
                });
            } else {
                return std::variant<vec3, quat>(quat{
                        std::get<quat>(parameters).x,
                        std::get<quat>(parameters).y,
                        std::get<quat>(parameters).z,
                        ease(advancement, easing, std::get<quat>(parameters).w)
                });
            }

        }
    }

    void to_json(json::json &j, const Transformation &transform) {
        j["begin_frame"] = transform.begin_frame;
        j["end_frame"] = transform.end_frame;
        j["easing"] = transform.easing;
        j["transformation"] = transform.transformation;
        if (transform.transformation == ROTATION) {
            j["params"] = std::get<quat>(transform.parameters);
        } else {
            j["params"] = std::get<vec3>(transform.parameters);
        }
    }

    void from_json(const json::json &j, Transformation &transform) {
        j.at("begin_frame").get_to(transform.begin_frame);
        j.at("end_frame").get_to(transform.end_frame);
        j.at("easing").get_to(transform.easing);
        j.at("transformation").get_to(transform.transformation);
        if (transform.transformation == ROTATION) {
            transform.parameters = j.at("params").get<quat>();
        } else {
            transform.parameters = j.at("params").get<vec3>();
        }
    }

    void to_json(json::json &j, const Transition &transition) {
        j["swap_time"] = transition.swap_time;
        j["transforms"] = transition.transforms;
    }

    void from_json(const json::json &j, Transition &transition) {
        j.at("swap_time").get_to(transition.swap_time);
        j.at("transforms").get_to(transition.transforms);
    }

}

void to_json(json::json &j, const vec3 &transform) {
    j["x"] = transform.x;
    j["y"] = transform.y;
    j["z"] = transform.z;
}

void from_json(const json::json &j, vec3 &transform) {
    j.at("x").get_to(transform.x);
    j.at("y").get_to(transform.y);
    j.at("z").get_to(transform.z);
}

void to_json(json::json &j, const quat &transform) {
    j["x"] = transform.x;
    j["y"] = transform.y;
    j["z"] = transform.z;
    j["w"] = transform.w;
}

void from_json(const json::json &j, quat &transform) {
    j.at("x").get_to(transform.x);
    j.at("y").get_to(transform.y);
    j.at("z").get_to(transform.z);
    j.at("w").get_to(transform.w);
}
