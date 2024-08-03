//
// Created by ElDonad on 21/06/2021.
//

#ifndef OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H
#define OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H

#include <graphics/vec3.h>
#include <graphics/quat.h>
#include "json.hpp"

#include <variant>

namespace json = nlohmann;

namespace Stinger3D
{

    enum EaseType
    {
        LINEAR,
        QUADRATIC,
        SINUSOIDAL,
        CONSTANT
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EaseType, {{LINEAR, "linear"},
                                            {QUADRATIC, "quadratic"},
                                            {SINUSOIDAL, "sinusoidal"},
                                            {CONSTANT, "constant"}})

    float ease(float input, EaseType type, float amplitude = 1, float offset = 0, float start = 0);

    enum TransformationType
    {
        SCALE,
        ROTATION,
        TRANSLATION
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransformationType, {{SCALE, "scale"},
                                                      {ROTATION, "rotation"},
                                                      {TRANSLATION, "translation"}})

    class Transformation
    {
    public:
        float begin_frame;
        float end_frame;

        EaseType easing;
        TransformationType transformation;
        std::variant<vec3, quat> parameters;

        virtual std::variant<vec3, quat> getFrame(float frame);
    };

    void to_json(json::json &j, const Transformation &transform);

    void from_json(const json::json &j, Transformation &transform);

    class Frame
    {
    public:
        vec3 position;
        quat rotation;
        vec3 scale;
    };

    void to_json(json::json &j, const Frame &frame);

    void from_json(const json::json &j, Frame &frame);

    class DataHolder
    {
    public:
        virtual void render_frame(float time) = 0;
        virtual ~DataHolder() = default;
    };

    class TransformData;

    class InterpolationData;

    void to_json(json::json &j, const TransformData &data);

    void from_json(const json::json &j, TransformData &data);

    void to_json(json::json &j, const InterpolationData &data);

    void from_json(const json::json &j, InterpolationData &data);

    class TransformData : public DataHolder
    {
    public:
        std::vector<Transformation> transforms;

        virtual void render_frame(float time) override;

        friend void to_json(json::json &j, const TransformData &data);

        friend void from_json(const json::json &j, TransformData &data);
    };

    class InterpolationData : public DataHolder
    {
    public:
        virtual void render_frame(float time) override;

        friend void to_json(json::json &j, const InterpolationData &data);

        friend void from_json(const json::json &j, InterpolationData &data);

    protected:
        std::vector<Frame> raw_frames;
        float resolution;
    };

    enum DataType
    {
        TRANSFORM,
        INTERPOLATION,
        INVALID
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(DataType, {{TRANSFORM, "transform"},
                                            {INTERPOLATION, "interpolation"},
                                            {INVALID, nullptr}})

    std::string toString(DataType &);

    DataType toDataType(std::string &);

    class Transition
    {
    public:
        std::unique_ptr<DataHolder> transforms;
        float swap_time;
        DataType data_type;
    };

    void to_json(json::json &j, const Transition &);

    void from_json(const json::json &j, Transition &);

}

void to_json(json::json &j, const vec3 &transform);

void from_json(const json::json &j, vec3 &transform);

void to_json(json::json &j, const quat &transform);

void from_json(const json::json &j, quat &transform);

#endif // OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H
