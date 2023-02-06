#pragma once

#include "phi.h"
#include "gfx.h"

#include <cmath>
#include <vector>
#include <tuple>
#include <algorithm>

struct ValueTuple
{
    float alpha, cl, cd;
};

// NACA 0012 (n0012-il) Xfoil prediction polar at RE=1,000,000 Ncrit=9
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

struct Aerodynamics
{
    float min, max;
    std::vector<ValueTuple> data;

    Aerodynamics(const std::vector<ValueTuple> &curve_data) : data(curve_data)
    {
        min = curve_data[0].alpha;
        max = curve_data[curve_data.size() - 1].alpha;
    }

    std::tuple<float, float> sample(float alpha) const
    {
        int index = static_cast<int>(phi::utils::scale(alpha, min, max, 0, data.size()));
        index = phi::utils::clamp(index, 0, static_cast<int>(data.size() - 1U));
        if (!(0 <= index && index < data.size()))
        {
            printf("alpha = %f, index = %d, size = %d\n", alpha, index, (int)data.size());
            assert(false);
        }
        return { data[index].cl, data[index].cd };
    }
};

Aerodynamics naca0012(NACA_0012);
Aerodynamics naca2412(NACA_2412);

struct Engine
{   
    float throttle = 0.5f;    
    const float thrust = 200000.0f; 
    
    Engine(float engine_thrust) : thrust(engine_thrust) {}

    void apply_forces(phi::RigidBody &rigid_body)
    {
        rigid_body.add_relative_force({ thrust * throttle, 0.0f, 0.0f });
        // TODO: implement torque from propeller
    }
};

struct Wing
{
    const float area;
    const std::string name;
    const glm::vec3 position; 
    const Aerodynamics *aerodynamics;

    glm::vec3 normal;
    float lift_multiplier = 1.0f;
    float drag_multiplier = 1.0f;
    
    Wing(
        const std::string &wing_name, 
        const glm::vec3 &offset, 
        float wing_area, 
        const Aerodynamics *aero, 
        const glm::vec3 &wing_normal
    )
        : name(wing_name),
          position(offset),
          area(wing_area),
          aerodynamics(aero),
          normal(wing_normal)
    {}

    static glm::vec3 calculate_normal(float incidence_angle_degrees)
    {
        float theta = glm::radians(incidence_angle_degrees + 90.0f);
        float x = cos(theta), y = sin(theta);
        return glm::normalize(glm::vec3(x, y, 0.0f));
    }

    void set_incidence(float incidence)
    {
        normal = calculate_normal(incidence);
    }

    void apply_forces(phi::RigidBody &rigid_body)
    {
        auto local_velocity = rigid_body.get_point_velocity(position);
        auto speed = glm::length(local_velocity);

        if (speed <= 0.0f || area <= 0.0f)
            return;

        auto drag_direction = glm::normalize(-local_velocity);
        auto lift_direction = glm::normalize(glm::cross(glm::cross(drag_direction, normal), drag_direction));

        auto angle_of_attack = glm::degrees(std::asin(glm::dot(drag_direction, normal)));

        auto [lift_coefficient, drag_coefficient] = aerodynamics->sample(angle_of_attack);

        float tmp = phi::sq(speed) * phi::rho * area * 0.5f;
        auto lift = lift_direction * lift_coefficient * lift_multiplier * tmp;
        auto drag = drag_direction * drag_coefficient * drag_multiplier * tmp;

        rigid_body.add_force_at_point(lift + drag, position);
    }
};

glm::mat3 inertia = phi::inertia::tensor({100000.0f, 50000.0f, 500000.0f}); 
//glm::mat3 inertia = phi::inertia::tensor({200000.0f, 50000.0f, 500000.0f}); 

struct Aircraft
{
    Engine engine;
    std::vector<Wing> elements;
    phi::RigidBody rigid_body;
    glm::vec3 joystick{}; // roll, yaw, pitch

    const float aileron_torque = 1500000.0f, elevator_torque = 1000000.0f, yaw_torque = 1000.0f;
    const glm::vec3 control_torque = { aileron_torque, yaw_torque, elevator_torque };

    float log_timer = 1.0f;

    Aircraft(const glm::vec3 &position, const glm::vec3 &velocity)
        : rigid_body({ .mass = 16000.0f, .inertia = inertia }),
        elements({
          Wing("left_wing",     glm::vec3(-0.5f,   0.0f, -2.73f),      24.36f, &naca2412, phi::UP),
          Wing("left_aileron",  glm::vec3( 0.0f,   0.0f, -2.0f),        8.79f, &naca0012, phi::UP),
          Wing("right_aileron", glm::vec3( 0.0f,   0.0f,  2.0f),        8.79f, &naca0012, phi::UP),
          Wing("right_wing",    glm::vec3(-0.5f,   0.0f,  2.73f),      24.36f, &naca2412, phi::UP),
          Wing("elevator",      glm::vec3(-6.64f, -0.12f, 0.0f),       17.66f, &naca0012, phi::UP),
          Wing("rudder",        glm::vec3(-6.64f,  0.0f,  0.0f),       16.46f, &naca0012, phi::RIGHT),
          Wing("fuselage_v",    glm::vec3( 0.0f,   0.0f,  0.0f),       25.00f, &naca0012, phi::RIGHT),
          Wing("fuselage_h",    glm::vec3( 0.0f,   0.0f,  0.0f),       25.00f, &naca0012, phi::UP),
        }),
        engine(100000.0f)
    {
        rigid_body.position = position;
        rigid_body.velocity = velocity;
        elements[6].lift_multiplier = elements[7].lift_multiplier = 0.0f;
        elements[6].drag_multiplier = elements[7].drag_multiplier = 10.0f;
        // std::cout << "aircraft inertia: " << rigid_body.inertia << std::endl;
    }

    void update(phi::Seconds dt)
    {
        Wing& lw = elements[0];
        Wing& la = elements[1];
        Wing& ra = elements[2];
        Wing& rw = elements[3];
        Wing& el = elements[4];
        Wing& ru = elements[5];

#if 1
        float roll = joystick.x;
        float yaw = joystick.y;
        float pitch = joystick.z;
        float max_elevator_deflection = 5.0f, max_aileron_deflection = 15.0f;
        float aileron_deflection = roll * max_aileron_deflection;
        la.set_incidence(+aileron_deflection);
        ra.set_incidence(-aileron_deflection);
        el.set_incidence(-pitch * max_elevator_deflection);
#else
        float control_authority = phi::utils::clamp(glm::length(rigid_body.velocity) / 150.0f, 0.0f, 1.0f);
        rigid_body.add_relative_torque((joystick * control_torque) * control_authority);
#endif

        for (Wing& wing : elements)
        {
            wing.apply_forces(rigid_body);
        }    

        engine.apply_forces(rigid_body);

        if ((log_timer += dt) > 0.5f)
        {
            log_timer = 0;
#if 1
            printf(
                "%.2f km/h, thrtl: %.2f, alt: %.2f m\n", 
                phi::units::kilometer_per_hour(glm::length(rigid_body.velocity)),
                engine.throttle,
                rigid_body.position.y
            );
#endif
        }

        rigid_body.update(dt);
    }
};
