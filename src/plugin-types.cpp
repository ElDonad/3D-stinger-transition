//
// Created by ElDonad on 21/06/2021.
//

#include "plugin-types.h"

#include <iostream>
#include <obs-module.h>
#include <graphics/axisang.h>

namespace Stinger3D
{

    const vec3 camera_position = vec3{-1, -1, -1};

    float ease(float input, EaseType type, float amplitude, float offset, float start)
    {
        float t2 = (input - offset) * (amplitude - start);
        float t2_inv = (1 - (input - offset)) * (amplitude - start);
        float val;
        switch (type)
        {
        case LINEAR:
            return t2 + start;
        case SINUSOIDAL:
        {
            val = -(amplitude - start) * (cosf(M_PI * (input - offset)) - 1.0f) / 2.0f + start;
            return val;
        }
        case QUADRATIC:
            return t2 < 0.5f ? 2.0f * t2 * t2 : 1.0f - powf(-2.0f * t2 + 2.0f, 2.0f) / 2.0f;
        case CONSTANT:
            return amplitude;
        default:
            return 0;
        }
    }

    std::variant<vec3, quat> Transformation::getFrame(float frame)
    {
        if (frame < begin_frame || frame > end_frame)
        {
            if (transformation == ROTATION)
                return std::variant<vec3, quat>(quat{0, 0, 0, 0});
            else if (transformation == SCALE)
                return std::variant<vec3, quat>(vec3{1, 1, 1});
            else
                return std::variant<vec3, quat>(vec3{0, 0, 0});
        }
        else
        {
            float advancement = (frame - begin_frame) / (end_frame - begin_frame);
            if (transformation == TRANSLATION)
            {
                return std::variant<vec3, quat>(vec3{
                    ease(advancement, easing, std::get<vec3>(parameters).x),
                    ease(advancement, easing, std::get<vec3>(parameters).y),
                    ease(advancement, easing, std::get<vec3>(parameters).z),
                });
            }
            else if (transformation == SCALE)
            {
                return std::variant<vec3, quat>(vec3{
                    ease(advancement, easing, std::get<vec3>(parameters).x, 0, 1),
                    ease(advancement, easing, std::get<vec3>(parameters).y, 0, 1),
                    ease(advancement, easing, std::get<vec3>(parameters).z, 0, 1),
                });
            }
            else
            {
                return std::variant<vec3, quat>(quat{
                    std::get<quat>(parameters).x,
                    std::get<quat>(parameters).y,
                    std::get<quat>(parameters).z,
                    ease(advancement, easing, std::get<quat>(parameters).w)});
            }
        }
    }

    void to_json(json::json &j, const Frame &frame)
    {
        j["position"] = frame.position;
        j["rotation"] = frame.rotation;
        j["scale"] = frame.scale;
    }

    void from_json(const json::json &j, Frame &frame)
    {
        j.at("position").get_to(frame.position);
        j.at("rotation").get_to(frame.rotation);
        j.at("scale").get_to(frame.scale);
    }

    void to_json(json::json &j, const TransformData &data)
    {
        j["transforms"] = data.transforms;
    }

    void from_json(const json::json &j, TransformData &data)
    {
        j.at("transforms").get_to(data.transforms);
    }

    void to_json(json::json &j, const InterpolationData &data)
    {
        j["frames"] = data.raw_frames;
        j["resolution"] = data.resolution;
    }

    void from_json(const json::json &j, InterpolationData &data)
    {
        j.at("resolution").get_to(data.resolution);
        j.at("frames").get_to(data.raw_frames);
    }

    float interpolate(float lower, float upper, float offset)
    {
        return lower + (upper - lower) * offset;
    };

    void InterpolationData::render_frame(float time)
    {
        int lowerFramei = (int)floor(raw_frames.size() * time);
        int upperFramei = (int)ceil(raw_frames.size() * time);
        float currentFramei = ((float)raw_frames.size()) * time;
        float coi = currentFramei - (float)lowerFramei;

        auto &lf = raw_frames[lowerFramei];
        auto &uf = raw_frames[upperFramei];
        auto pos = vec3{
            interpolate(lf.position.x, uf.position.x, coi),
            interpolate(lf.position.y, uf.position.y, coi),
            interpolate(lf.position.z, uf.position.z, coi)};
        auto rot = quat{
            interpolate(lf.rotation.x, uf.rotation.x, coi),
            interpolate(lf.rotation.y, uf.rotation.y, coi),
            interpolate(lf.rotation.z, uf.rotation.z, coi),
            interpolate(lf.rotation.w, uf.rotation.w, coi),
        };

        auto scale = vec3{
            interpolate(lf.scale.x, uf.scale.x, coi),
            interpolate(lf.scale.y, uf.scale.y, coi),
            interpolate(lf.scale.z, uf.scale.z, coi)};

        gs_matrix_translate(&camera_position);
        gs_matrix_translate(&pos);

        gs_matrix_translate3f(1, 1, 0);
        gs_matrix_rotquat(&rot);
        gs_matrix_translate3f(-1, -1, 0);

        gs_matrix_scale(&scale);
    }

    void TransformData::render_frame(float time)
    {
        for (auto transform : transforms)
        {
            switch (transform.transformation)
            {
            case Stinger3D::TRANSLATION:
            {
                auto machin = std::get<vec3>(transform.getFrame(time));
                gs_matrix_translate(&machin);
                break;
            }
            case Stinger3D::ROTATION:
            {
                auto qu = std::get<quat>(transform.getFrame(time));
                gs_matrix_rotaa4f(qu.x, qu.y, qu.z, qu.w);
                break;
            }
            case Stinger3D::SCALE:
            {
                auto val = std::get<vec3>(transform.getFrame(time));
                gs_matrix_scale(&val);
                break;
            }
            }
        }
    }

    void to_json(json::json &j, const Transformation &transform)
    {
        j["begin_frame"] = transform.begin_frame;
        j["end_frame"] = transform.end_frame;
        j["easing"] = transform.easing;
        j["transformation"] = transform.transformation;
        if (transform.transformation == ROTATION)
        {
            j["params"] = std::get<quat>(transform.parameters);
        }
        else
        {
            j["params"] = std::get<vec3>(transform.parameters);
        }
    }

    void from_json(const json::json &j, Transformation &transform)
    {
        j.at("begin_frame").get_to(transform.begin_frame);
        j.at("end_frame").get_to(transform.end_frame);
        j.at("easing").get_to(transform.easing);
        j.at("transformation").get_to(transform.transformation);
        if (transform.transformation == ROTATION)
        {
            transform.parameters = j.at("params").get<quat>();
        }
        else
        {
            transform.parameters = j.at("params").get<vec3>();
        }
    }

    void to_json(json::json &j, const Transition &transition)
    {
        j["swap_time"] = transition.swap_time;
        j["data_type"] = transition.data_type;
        switch (transition.data_type)
        {
        case TRANSFORM:
            j["data"] = *static_cast<TransformData *>(transition.transforms.get());
        case INTERPOLATION:
            j["data"] = *static_cast<InterpolationData *>(transition.transforms.get());
        }
    }

    void from_json(const json::json &j, Transition &transition)
    {
        j.at("swap_time").get_to(transition.swap_time);

        j.at("data_type").get_to(transition.data_type);
        switch (transition.data_type)
        {
        case INTERPOLATION:
            transition.transforms = std::make_unique<InterpolationData>(
                std::move(j.at("data").get<InterpolationData>()));
            break;
        case TRANSFORM:
            transition.transforms = std::make_unique<TransformData>(
                std::move(j.at("data").get<TransformData>()));
            break;
        }
    }

}

void to_json(json::json &j, const vec3 &transform)
{
    j["x"] = transform.x;
    j["y"] = transform.y;
    j["z"] = transform.z;
}

void from_json(const json::json &j, vec3 &transform)
{
    j.at("x").get_to(transform.x);
    j.at("y").get_to(transform.y);
    j.at("z").get_to(transform.z);
}

void to_json(json::json &j, const quat &transform)
{
    j["x"] = transform.x;
    j["y"] = transform.y;
    j["z"] = transform.z;
    j["w"] = transform.w;
}

void from_json(const json::json &j, quat &transform)
{
    j.at("x").get_to(transform.x);
    j.at("y").get_to(transform.y);
    j.at("z").get_to(transform.z);
    j.at("w").get_to(transform.w);
}
