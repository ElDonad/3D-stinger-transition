//
// Created by ElDonad on 21/06/2021.
//

#include "plugin-types.h"

#include <iostream>

namespace Stinger3D {

    float ease(float input, EaseType type, float amplitude, float offset) {
        float t2 = (input - offset) * amplitude;
        switch (type) {
            case LINEAR:
                return t2;
            case SINUSOIDAL:
                return -(cos(M_PI * t2) - 1.0) / 2.0;
            case QUADRATIC:
                return t2 < 0.5 ? 2.0 * t2 * t2 : 1.0 - powf(-2.0 * t2 + 2.0, 2.0) / 2.0;
            case CONSTANT:
                return 1;
        }
    }

    vec3 Transformation::getFrame(float frame) {
        if (frame < begin_frame || frame > end_frame) {
            return vec3{0, 0, 0};
        } else {
            float advancement = (frame - begin_frame) / (end_frame - begin_frame);
            return vec3{
                    ease(advancement, easing, parameters.x),
                    ease(advancement, easing, parameters.y),
                    ease(advancement, easing, parameters.z),
            };
        }

    }

    void to_json(json::json &j, const Transformation &transform) {
        j["begin_frame"] = transform.begin_frame;
        j["end_frame"] = transform.end_frame;
        j["easing"] = transform.easing;
        j["transformation"] = transform.transformation;
        j["params"] = transform.parameters;
    }

    void from_json(const json::json &j, Transformation &transform) {
        j.at("begin_frame").get_to(transform.begin_frame);
        j.at("end_frame").get_to(transform.end_frame);
        j.at("easing").get_to(transform.easing);
        j.at("transformation").get_to(transform.transformation);
        j.at("params").get_to(transform.parameters);
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
