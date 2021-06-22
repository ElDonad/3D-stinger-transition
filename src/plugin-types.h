//
// Created by ElDonad on 21/06/2021.
//

#ifndef OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H
#define OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H

#include <graphics/vec3.h>
#include "json.hpp"

namespace json = nlohmann;


namespace Stinger3D {

    enum EaseType {
        LINEAR,
        QUADRATIC,
        SINUSOIDAL,
        CONSTANT
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(EaseType, {
        { LINEAR, "linear" },
        { QUADRATIC, "quadratic" },
        { SINUSOIDAL, "sinuoidal" },
        { CONSTANT, "constant" }
    })

    float ease(float input, EaseType type, float amplitude = 1, float offset = 0);

    enum TransformationType {
        SCALE,
        ROTATION,
        TRANSLATION
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(TransformationType, {
        { SCALE, "scale" },
        { ROTATION, "rotation" },
        { TRANSLATION, "translation" }
    })

    class Transformation {
    public:
        float begin_frame;
        float end_frame;

        EaseType easing;
        TransformationType transformation;
        vec3 parameters;

        virtual vec3 getFrame(float frame);

    };


    void to_json(json::json &j, const Transformation &transform);

    void from_json(const json::json &j, Transformation &transform);

}

void to_json(json::json &j, const vec3 &transform);

void from_json(const json::json &j, vec3 &transform);

#endif //OBS_3DSTINGERTRANSITION_PLUGIN_TYPES_H
