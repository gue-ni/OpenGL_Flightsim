#pragma once

#include "phi.h"
#include "gfx.h"

#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>

constexpr float log_intervall = 1.0f;

struct ValueTuple
{
    float alpha;
    float cl;
    float cd;
};

struct MinMax
{
    float min, max;
};

// NACA 0012 AIRFOILS (n0012-il) Xfoil prediction polar at RE=1,000,000 Ncrit=9
std::vector<ValueTuple> NACA_0012 = {
    {-18.500f, -1.2258f, 0.10236f},
    {-18.250f, -1.2456f, 0.09505f},
    {-18.000f, -1.2659f, 0.08782f},
    {-17.750f, -1.2852f, 0.08088f},
    {-17.500f, -1.3031f, 0.07429f},
    {-17.250f, -1.3193f, 0.06814f},
    {-17.000f, -1.3322f, 0.06256f},
    {-16.750f, -1.3427f, 0.05745f},
    {-16.500f, -1.3519f, 0.05263f},
    {-16.250f, -1.3687f, 0.04704f},
    {-16.000f, -1.3806f, 0.04234f},
    {-15.750f, -1.3868f, 0.03865f},
    {-15.500f, -1.3879f, 0.03577f},
    {-15.250f, -1.3854f, 0.03348f},
    {-15.000f, -1.3802f, 0.03160f},
    {-14.750f, -1.3739f, 0.02997f},
    {-14.500f, -1.3669f, 0.02849f},
    {-14.250f, -1.3590f, 0.02718f},
    {-14.000f, -1.3502f, 0.02602f},
    {-13.750f, -1.3399f, 0.02502f},
    {-13.500f, -1.3281f, 0.02416f},
    {-13.250f, -1.3149f, 0.02343f},
    {-13.000f, -1.3132f, 0.02207f},
    {-12.750f, -1.3004f, 0.02117f},
    {-12.500f, -1.2834f, 0.02049f},
    {-12.250f, -1.2649f, 0.01988f},
    {-12.000f, -1.2455f, 0.01931f},
    {-11.750f, -1.2250f, 0.01881f},
    {-11.500f, -1.2032f, 0.01839f},
    {-11.250f, -1.1872f, 0.01745f},
    {-11.000f, -1.1666f, 0.01690f},
    {-10.750f, -1.1449f, 0.01645f},
    {-10.500f, -1.1228f, 0.01602f},
    {-10.250f, -1.1000f, 0.01566f},
    {-10.000f, -1.0807f, 0.01499f},
    {-9.750f, -1.0591f, 0.01456f},
    {-9.500f, -1.0368f, 0.01423f},
    {-9.250f, -1.0143f, 0.01395f},
    {-9.000f, -0.9948f, 0.01341f},
    {-8.750f, -0.9735f, 0.01307f},
    {-8.500f, -0.9517f, 0.01279f},
    {-8.250f, -0.9313f, 0.01240f},
    {-8.000f, -0.9104f, 0.01209f},
    {-7.750f, -0.8888f, 0.01187f},
    {-7.500f, -0.8687f, 0.01149f},
    {-7.250f, -0.8475f, 0.01124f},
    {-7.000f, -0.8268f, 0.01095f},
    {-6.750f, -0.7975f, 0.01069f},
    {-6.500f, -0.7634f, 0.01034f},
    {-6.250f, -0.7289f, 0.01003f},
    {-6.000f, -0.6939f, 0.00973f},
    {-5.750f, -0.6601f, 0.00942f},
    {-5.500f, -0.6268f, 0.00911f},
    {-5.000f, -0.5571f, 0.00847f},
    {-4.750f, -0.5213f, 0.00816f},
    {-4.500f, -0.4903f, 0.00786f},
    {-4.250f, -0.4588f, 0.00756f},
    {-4.000f, -0.4276f, 0.00728f},
    {-3.750f, -0.4004f, 0.00705f},
    {-3.500f, -0.3730f, 0.00681f},
    {-3.250f, -0.3462f, 0.00659f},
    {-3.000f, -0.3201f, 0.00640f},
    {-2.750f, -0.2938f, 0.00623f},
    {-2.500f, -0.2675f, 0.00606f},
    {-2.250f, -0.2410f, 0.00593f},
    {-2.000f, -0.2144f, 0.00581f},
    {-1.750f, -0.1878f, 0.00570f},
    {-1.500f, -0.1611f, 0.00562f},
    {-1.250f, -0.1343f, 0.00556f},
    {-1.000f, -0.1075f, 0.00549f},
    {-0.750f, -0.0806f, 0.00546f},
    {-0.500f, -0.0538f, 0.00542f},
    {-0.250f, -0.0268f, 0.00541f},
    {0.000f, 0.0000f, 0.00540f},
    {0.250f, 0.0269f, 0.00541f},
    {0.500f, 0.0538f, 0.00542f},
    {0.750f, 0.0806f, 0.00546f},
    {1.000f, 0.1075f, 0.00549f},
    {1.250f, 0.1344f, 0.00555f},
    {1.500f, 0.1611f, 0.00562f},
    {1.750f, 0.1878f, 0.00570f},
    {2.000f, 0.2144f, 0.00581f},
    {2.250f, 0.2410f, 0.00593f},
    {2.500f, 0.2675f, 0.00606f},
    {2.750f, 0.2938f, 0.00623f},
    {3.000f, 0.3201f, 0.00640f},
    {3.250f, 0.3462f, 0.00659f},
    {3.500f, 0.3730f, 0.00681f},
    {3.750f, 0.4004f, 0.00705f},
    {4.000f, 0.4276f, 0.00728f},
    {4.250f, 0.4588f, 0.00756f},
    {4.500f, 0.4903f, 0.00786f},
    {4.750f, 0.5213f, 0.00816f},
    {5.000f, 0.5572f, 0.00847f},
    {5.500f, 0.6268f, 0.00911f},
    {5.750f, 0.6601f, 0.00942f},
    {6.000f, 0.6940f, 0.00972f},
    {6.250f, 0.7290f, 0.01003f},
    {6.500f, 0.7635f, 0.01034f},
    {6.750f, 0.7976f, 0.01069f},
    {7.000f, 0.8267f, 0.01095f},
    {7.250f, 0.8474f, 0.01124f},
    {7.500f, 0.8686f, 0.01149f},
    {7.750f, 0.8887f, 0.01186f},
    {8.000f, 0.9103f, 0.01208f},
    {8.250f, 0.9312f, 0.01240f},
    {8.500f, 0.9516f, 0.01279f},
    {8.750f, 0.9734f, 0.01307f},
    {9.000f, 0.9948f, 0.01341f},
    {9.250f, 1.0143f, 0.01395f},
    {9.500f, 1.0368f, 0.01423f},
    {9.750f, 1.0591f, 0.01456f},
    {10.000f, 1.0808f, 0.01499f},
    {10.250f, 1.1001f, 0.01566f},
    {10.500f, 1.1229f, 0.01602f},
    {10.750f, 1.1450f, 0.01645f},
    {11.000f, 1.1668f, 0.01690f},
    {11.250f, 1.1874f, 0.01745f},
    {11.500f, 1.2033f, 0.01840f},
    {11.750f, 1.2251f, 0.01881f},
    {12.000f, 1.2457f, 0.01931f},
    {12.250f, 1.2651f, 0.01988f},
    {12.500f, 1.2837f, 0.02049f},
    {12.750f, 1.3008f, 0.02117f},
    {13.000f, 1.3137f, 0.02207f},
    {13.250f, 1.3157f, 0.02342f},
    {13.500f, 1.3290f, 0.02415f},
    {13.750f, 1.3406f, 0.02502f},
    {14.000f, 1.3509f, 0.02602f},
    {14.250f, 1.3600f, 0.02717f},
    {14.500f, 1.3679f, 0.02848f},
    {14.750f, 1.3750f, 0.02995f},
    {15.000f, 1.3814f, 0.03158f},
    {15.250f, 1.3864f, 0.03348f},
    {15.500f, 1.3892f, 0.03575f},
    {15.750f, 1.3881f, 0.03863f},
    {16.000f, 1.3818f, 0.04235f},
    {16.250f, 1.3702f, 0.04702f},
    {16.500f, 1.3536f, 0.05261f},
    {16.750f, 1.3451f, 0.05734f},
    {17.000f, 1.3352f, 0.06237f},
    {17.250f, 1.3218f, 0.06802f},
    {17.500f, 1.3059f, 0.07416f},
    {17.750f, 1.2880f, 0.08075f},
    {18.000f, 1.2685f, 0.08773f},
    {18.250f, 1.2485f, 0.09493f},
    {18.500f, 1.2284f, 0.10229f}};

// NACA 2412 (naca2412-il) Xfoil prediction polar at RE=1,000,000 Ncrit=9
std::vector<ValueTuple> NACA_2412 = {
    {-17.500f, -1.1118f, 0.08608f},
    {-17.250f, -1.1738f, 0.07238f},
    {-17.000f, -1.2296f, 0.05928f},
    {-16.750f, -1.2629f, 0.04931f},
    {-16.500f, -1.2790f, 0.04253f},
    {-16.250f, -1.2852f, 0.03792f},
    {-16.000f, -1.2869f, 0.03455f},
    {-15.750f, -1.2853f, 0.03207f},
    {-15.500f, -1.2815f, 0.03016f},
    {-15.250f, -1.2755f, 0.02867f},
    {-15.000f, -1.2674f, 0.02752f},
    {-14.750f, -1.2667f, 0.02608f},
    {-14.500f, -1.2585f, 0.02491f},
    {-14.250f, -1.2429f, 0.02417f},
    {-14.000f, -1.2251f, 0.02358f},
    {-13.750f, -1.2068f, 0.02304f},
    {-13.500f, -1.1881f, 0.02254f},
    {-13.250f, -1.1690f, 0.02212f},
    {-13.000f, -1.1561f, 0.02124f},
    {-12.750f, -1.1417f, 0.02051f},
    {-12.500f, -1.1239f, 0.02009f},
    {-12.250f, -1.1058f, 0.01971f},
    {-12.000f, -1.0881f, 0.01933f},
    {-11.750f, -1.0704f, 0.01896f},
    {-11.500f, -1.0519f, 0.01868f},
    {-11.250f, -1.0327f, 0.01847f},
    {-11.000f, -1.0222f, 0.01737f},
    {-10.750f, -1.0046f, 0.01696f},
    {-10.500f, -0.9766f, 0.01657f},
    {-10.250f, -0.9448f, 0.01617f},
    {-10.000f, -0.9125f, 0.01582f},
    {-9.750f, -0.8793f, 0.01556f},
    {-9.500f, -0.8484f, 0.01474f},
    {-9.250f, -0.8172f, 0.01411f},
    {-9.000f, -0.7863f, 0.01373f},
    {-8.750f, -0.7542f, 0.01336f},
    {-8.500f, -0.7211f, 0.01301f},
    {-8.250f, -0.6870f, 0.01271f},
    {-8.000f, -0.6520f, 0.01227f},
    {-7.750f, -0.6166f, 0.01161f},
    {-7.500f, -0.5850f, 0.01122f},
    {-7.250f, -0.5532f, 0.01089f},
    {-7.000f, -0.5207f, 0.01059f},
    {-6.750f, -0.4884f, 0.01033f},
    {-6.500f, -0.4627f, 0.00998f},
    {-6.250f, -0.4342f, 0.00964f},
    {-6.000f, -0.4073f, 0.00939f},
    {-5.750f, -0.3804f, 0.00916f},
    {-5.500f, -0.3538f, 0.00887f},
    {-5.250f, -0.3274f, 0.00860f},
    {-5.000f, -0.3006f, 0.00836f},
    {-4.750f, -0.2737f, 0.00816f},
    {-4.500f, -0.2465f, 0.00798f},
    {-4.000f, -0.1918f, 0.00768f},
    {-3.750f, -0.1645f, 0.00752f},
    {-3.500f, -0.1372f, 0.00737f},
    {-3.250f, -0.1100f, 0.00719f},
    {-3.000f, -0.0825f, 0.00704f},
    {-2.750f, -0.0552f, 0.00689f},
    {-2.500f, -0.0277f, 0.00678f},
    {-2.250f, -0.0003f, 0.00666f},
    {-2.000f, 0.0272f, 0.00653f},
    {-1.750f, 0.0546f, 0.00640f},
    {-1.500f, 0.0819f, 0.00628f},
    {-1.250f, 0.1092f, 0.00616f},
    {-1.000f, 0.1362f, 0.00602f},
    {-0.750f, 0.1632f, 0.00589f},
    {-0.500f, 0.1903f, 0.00580f},
    {-0.250f, 0.2173f, 0.00573f},
    {0.000f, 0.2442f, 0.00568f},
    {0.250f, 0.2709f, 0.00563f},
    {0.500f, 0.2968f, 0.00556f},
    {0.750f, 0.3217f, 0.00548f},
    {1.000f, 0.3469f, 0.00547f},
    {1.250f, 0.3722f, 0.00552f},
    {1.500f, 0.3979f, 0.00559f},
    {1.750f, 0.4250f, 0.00569f},
    {2.000f, 0.4549f, 0.00581f},
    {2.750f, 0.5582f, 0.00624f},
    {3.000f, 0.5945f, 0.00639f},
    {3.250f, 0.6318f, 0.00654f},
    {3.500f, 0.6686f, 0.00674f},
    {3.750f, 0.6918f, 0.00692f},
    {4.000f, 0.7153f, 0.00711f},
    {4.250f, 0.7389f, 0.00730f},
    {4.500f, 0.7624f, 0.00752f},
    {4.750f, 0.7858f, 0.00776f},
    {5.000f, 0.8089f, 0.00804f},
    {5.250f, 0.8319f, 0.00836f},
    {5.500f, 0.8552f, 0.00869f},
    {5.750f, 0.8784f, 0.00906f},
    {6.000f, 0.9016f, 0.00945f},
    {6.250f, 0.9251f, 0.00983f},
    {6.500f, 0.9483f, 0.01025f},
    {6.750f, 0.9710f, 0.01073f},
    {7.000f, 0.9944f, 0.01114f},
    {7.250f, 1.0179f, 0.01153f},
    {7.500f, 1.0414f, 0.01194f},
    {7.750f, 1.0644f, 0.01238f},
    {8.000f, 1.0885f, 0.01270f},
    {8.250f, 1.1111f, 0.01317f},
    {8.500f, 1.1353f, 0.01347f},
    {8.750f, 1.1585f, 0.01385f},
    {9.000f, 1.1801f, 0.01435f},
    {9.250f, 1.2032f, 0.01471f},
    {9.500f, 1.2262f, 0.01506f},
    {9.750f, 1.2485f, 0.01545f},
    {10.000f, 1.2696f, 0.01591f},
    {10.250f, 1.2881f, 0.01655f},
    {10.500f, 1.3090f, 0.01697f},
    {10.750f, 1.3299f, 0.01737f},
    {11.000f, 1.3500f, 0.01780f},
    {11.250f, 1.3684f, 0.01825f},
    {11.500f, 1.3833f, 0.01884f},
    {11.750f, 1.3931f, 0.01974f},
    {12.000f, 1.4114f, 0.02015f},
    {12.250f, 1.4284f, 0.02066f},
    {12.500f, 1.4446f, 0.02122f},
    {12.750f, 1.4595f, 0.02187f},
    {13.000f, 1.4699f, 0.02284f},
    {13.250f, 1.4815f, 0.02375f},
    {13.500f, 1.4967f, 0.02445f},
    {13.750f, 1.5106f, 0.02526f},
    {14.000f, 1.5228f, 0.02622f},
    {14.250f, 1.5311f, 0.02751f},
    {14.500f, 1.5386f, 0.02893f},
    {14.750f, 1.5499f, 0.03008f},
    {15.000f, 1.5597f, 0.03141f},
    {15.250f, 1.5675f, 0.03297f},
    {15.500f, 1.5712f, 0.03497f},
    {15.750f, 1.5723f, 0.03733f},
    {16.000f, 1.5775f, 0.03935f},
    {16.250f, 1.5806f, 0.04166f},
    {16.500f, 1.5820f, 0.04423f},
    {16.750f, 1.5815f, 0.04711f},
    {17.000f, 1.5784f, 0.05040f},
    {17.250f, 1.5716f, 0.05428f},
    {17.500f, 1.5603f, 0.05893f},
    {17.750f, 1.5483f, 0.06380f},
    {18.000f, 1.5415f, 0.06805f},
    {18.250f, 1.5328f, 0.07268f},
    {18.500f, 1.5214f, 0.07778f},
    {18.750f, 1.5083f, 0.08321f},
    {19.000f, 1.4942f, 0.08893f},
    {19.250f, 1.4781f, 0.09506f},
};

float max(float a, float b)
{
    return a > b ? a : b;
}

float min(float a, float b)
{
    return a < b ? a : b;
}

float sign(float a)
{
    return a >= 0.0f ? 1.0f : -1.0f;
}

float clamp(float v, float lo, float hi)
{
    return min(max(v, lo), hi);
}

struct Curve
{
    std::vector<ValueTuple> data;

    Curve(const std::vector<ValueTuple> &curve_data) : data(curve_data)
    {
        for (int i = 0; i < data.size() - 1; i++)
            assert(data[i].alpha < data[i + 1].alpha);
    }

    static float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }

    void sample(float alpha, float *cl, float *cd) const
    {
        // assert(data[0].alpha <= alpha && alpha <= data[data.size() - 1].alpha);

        int first = 0;
        int last = data.size() - 1;

        if (alpha < data[first].alpha)
        {
            *cl = data[first].cl;
            *cd = data[first].cd;
            return;
        }

        if (data[last].alpha < alpha)
        {
            *cl = data[last].cl;
            *cd = data[last].cd;
            return;
        }

        for (int i = 0; i < data.size() - 1; i++)
        {
            if (data[i].alpha <= alpha && alpha <= data[i + 1].alpha)
            {
                auto t0 = alpha - data[i].alpha;
                auto t1 = data[i + 1].alpha - data[i].alpha;
                auto f = t0 / t1;
                *cl = lerp(data[i].cl, data[i + 1].cl, f);
                *cd = lerp(data[i].cd, data[i + 1].cd, f);
                break;
            }
        }
        return;
    }
};

struct Wing
{
    const std::string name;
    const float area;
    const glm::vec3 center_of_gravity; // center of gravity relative to rigidbody cg
    glm::vec3 normal;
    const Curve curve;

    float lift_multiplier = 1.0f;
    float drag_multiplier = 1.0f;
    float lift_coefficient = 0.0f;
    float drag_coefficient = 0.0f;
    float lift = 0.0f;
    float drag = 0.0f; 
    float angle_of_attack = 0.0f;
    float incidence = 0.0f;

    float log_timer = 1.0f;

    Wing(const std::string &wing_name, const glm::vec3 &cg_offset, float wing_area, const Curve &aerodynamics, const glm::vec3 &wing_normal)
        : name(wing_name),
          center_of_gravity(cg_offset),
          area(wing_area),
          curve(aerodynamics),
          normal(wing_normal)
    {
    }

    Wing(const std::string &wing_name, const glm::vec3 &cg_offset, float wing_area, const Curve &aerodynamics, float wing_incidence)
        : name(wing_name),
          center_of_gravity(cg_offset),
          area(wing_area),
          curve(aerodynamics),
          incidence(wing_incidence),
          normal(calculate_normal(wing_incidence))
    {
    }

    static glm::vec3 calculate_normal(float incidence_angle_degrees)
    {
        float theta = glm::radians(incidence_angle_degrees + 90.0f);
        float x = cos(theta);
        float y = sin(theta);
        return glm::normalize(glm::vec3(x, y, 0.0f));
    }

    void apply_forces(phi::RigidBody &rigid_body, float dt, bool log)
    {
        // pretty sure something is wrong here
        const auto velocity = rigid_body.inverse_transform_direction(rigid_body.velocity);

        const auto local_velocity = rigid_body.inverse_transform_direction(rigid_body.velocity) 
            + glm::cross(rigid_body.angular_velocity, center_of_gravity);

        const auto speed = glm::length(local_velocity);

        if (speed <= 0.0f)
            return;

        assert(speed > 0.0f);

        auto drag_direction = glm::normalize(-local_velocity);
        auto lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

        angle_of_attack = glm::degrees(asin(glm::dot(drag_direction, normal)));

        curve.sample(angle_of_attack, &lift_coefficient, &drag_coefficient);

        lift = lift_coefficient * lift_multiplier * speed * speed * area;
        drag = drag_coefficient * drag_multiplier * speed * speed * area;

        // ugly hack, necessary probably because the inertia tensor is wrong
        auto force = (lift * lift_direction);
        auto torque = glm::cross(center_of_gravity, force);
        float torque_mulitplier = 0.01;
        rigid_body.add_relative_force(force);
        rigid_body.add_relative_torque(torque * torque_mulitplier);

        rigid_body.add_force_at_point(drag * drag_direction, center_of_gravity);

        if (log && ((log_timer += dt) > log_intervall))
        {
            log_timer = 0;
#if 1
            std::cout << "######### [ " << name << " ] #######" << std::endl;
            std::cout << "lv = " << local_velocity << std::endl;
            //std::cout << "t = " << torque << std::endl;
            std::cout << "f = " << force << std::endl;
            std::cout << "lift_direction = " << lift_direction << std::endl;
            // std::cout << "aoa = " << angle_of_attack << std::endl;
            // std::cout << "lift = " << lift << std::endl;
            // std::cout << "lift_multiplier = " << lift_multiplier << ",  lift = " << lift_coefficient * lift_multiplier * tmp2 << std::endl;
            // std::cout << "lift_coefficient = " <<  lift_coefficient  << std::endl;
            // std::cout << "tmp2 = " <<  tmp2  << std::endl;
            // std::cout << "drag = " << drag << std::endl;
            // std::cout << "force = " << force << std::endl;
            // std::cout << "torque = " << torque << std::endl;
            // std::cout << "lift_dir = " << lift_direction << std::endl;
            // std::cout << "drag_dir = " << drag_direction << std::endl;
            std::cout << "####################################" << std::endl;
#endif
        }
    }
};

struct ControlSurface : public Wing
{
};

inline float knots(float meter_per_second)
{
    return 0.0f;
}

inline float meter_per_second(float kilometer_per_hour)
{
    return kilometer_per_hour / 3.6f;
}

inline float kilometer_per_hour(float meters_per_second)
{
    return meters_per_second * 3.6f;
}

struct Engine
{
    float thrust = 200000.0f; // newtons
    float throttle = 1.0f;    // [0, 1]

    void apply_forces(phi::RigidBody &rigid_body)
    {
        rigid_body.add_relative_force(glm::vec3(thrust * throttle, 0.0f, 0.0f));
        // TODO: implement torque from propeller
    }
};

struct Aircraft
{
    phi::RigidBody rigid_body;

    //std::vector<Wing> elements;

    Wing left_wing;
    Wing right_wing;
    Wing right_aileron;
    Wing left_aileron;
    Wing rudder;
    Wing elevator;

    Engine engine;

    float log_timer = 1.0f;

    Aircraft(const glm::vec3 &position, const glm::vec3 &velocity)
        : rigid_body(16000.0f), // mass in kg
          left_wing("left_wing", glm::vec3(-0.5f, 0.0, -2.73f), 6.96f * 3.5f, Curve(NACA_2412), phi::UP),
          left_aileron("left_aileron", glm::vec3(0.0f, 0.0f, -1.0f), 3.8f * 1.26f, Curve(NACA_0012), phi::UP),
          right_wing("right_wing", glm::vec3(-0.5f, 0.0, +2.73f), 6.96f * 3.5f, Curve(NACA_2412), phi::UP),
          right_aileron("right_aileron", glm::vec3(0.0f, 0.0f, 1.0f), 3.8f * 1.26f, Curve(NACA_0012), phi::UP),
          elevator("elevator", glm::vec3(-6.64f, -0.12f, 0.0f), 6.54f * 2.7f, Curve(NACA_0012), phi::UP),
          rudder("rudder", glm::vec3(-6.64f, 0.0f, 0.0f), 5.31f * 3.1f, Curve(NACA_0012), phi::RIGHT)
    {
        rigid_body.position = position;
        rigid_body.velocity = velocity;
    }

    void update(float dt)
    {
#if 1
        left_wing.apply_forces(rigid_body, dt, true);
        right_wing.apply_forces(rigid_body, dt, true);
        elevator.apply_forces(rigid_body, dt, false);
        rudder.apply_forces(rigid_body, dt, false);
        left_aileron.apply_forces(rigid_body, dt, false);
        right_aileron.apply_forces(rigid_body, dt, false);
        engine.apply_forces(rigid_body);
#endif


        //float cl, cd;
        //right_wing.curve.sample(-0.5f, &cl, &cd);
        // std::cout << cl << ", " << cd << std::endl;


        if ((log_timer += dt) > log_intervall)
        {
#if 1
            log_timer = 0;
            std::cout << "########## airplane ##############" << std::endl;
            // std::cout << "height: " << rigid_body.position.y << std::endl;
            // std::cout << "vertical speed: " << kilometer_per_hour(rigid_body.velocity.y) << std::endl;
            printf("speed: %.2f\n", glm::length(rigid_body.velocity));
            printf("throttle: %.2f\n", engine.throttle);
            printf("wing_area: lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.area, right_wing.area, elevator.area);
            printf("lift: lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.lift, right_wing.lift, elevator.lift);
            printf("drag: lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.drag, right_wing.drag, elevator.drag);
            printf("aoa:  lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.angle_of_attack, right_wing.angle_of_attack, elevator.angle_of_attack);
            printf("cl:   lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.lift_coefficient, right_wing.lift_coefficient, elevator.lift_coefficient);
            printf("cd:   lw = %.2f, rw = %.2f, el = %.2f\n", left_wing.drag_coefficient, right_wing.drag_coefficient, elevator.drag_coefficient);
            std::cout << "angular_velocity = " << rigid_body.angular_velocity << std::endl;
            std::cout << "torque = " << rigid_body.get_torque() << std::endl;
            std::cout << "force = " << rigid_body.get_force() << std::endl;
            std::cout << "##################################" << std::endl;
#endif
        }

        /*
        float max_speed = 170.0f;
        if (glm::length(rigid_body.velocity) > max_speed)
        {
            rigid_body.velocity = glm::normalize(rigid_body.velocity) * max_speed;
        }
        */

        rigid_body.update(dt);
    }
};
