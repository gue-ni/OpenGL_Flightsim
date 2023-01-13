#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "gfx.h"
#include "phi.h"
#include "flightmodel.h"


using std::shared_ptr;
using std::make_shared;
using std::cout;
using std::endl;

constexpr auto SCREEN_WIDTH = 800;
constexpr auto SCREEN_HEIGHT = 600;

/*
std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	return os << v.x << ", " << v.y << ", " << v.z;
}
*/

void apply_to_object3d(const phi::RigidBody& rigid_body, gfx::Object3D& object3d)
{
    object3d.set_position(rigid_body.position);
    object3d.set_rotation_quaternion(rigid_body.rotation);
}

void solve_constraints(phi::RigidBody& rigid_body)
{
	if (rigid_body.position.y <= 0)
	{
		rigid_body.position.y = 0, rigid_body.velocity.y = 0;
	}
}

int main(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "OpenGL/SDL Sandbox", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, 
        SCREEN_HEIGHT, 
        SDL_WINDOW_OPENGL
    );

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;

    if (GLEW_OK != glewInit())
        return -1;

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // SDL options
    SDL_ShowCursor(SDL_FALSE);
    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    std::vector<float> cube_vertices_2 = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,




     
    };

    std::vector<float> triangle_vertices = {
        // positions          // normals           // texture coords
        0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   // bottom right
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    };

    std::vector<float> ico_vertices;
    std::vector<float> cube_vertices;
    std::vector<float> cessna_vertices;
    std::vector<float> cessna_prop_vertices;

    gfx::load_obj("assets/cube.obj", cube_vertices);
    gfx::load_obj("assets/icosphere.obj", ico_vertices);
    gfx::load_obj("assets/Cessna_172/Cessna_172.obj", cessna_vertices);
    gfx::load_obj("assets/Cessna_172/Cessna_172_prop.obj", cessna_prop_vertices);

    gfx::Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    auto blue   = make_shared<gfx::Phong>(gfx::rgb(0, 0, 255));
    auto grey    = make_shared<gfx::Phong>(glm::vec3(0.5f));
    auto green    = make_shared<gfx::Phong>(gfx::rgb(0, 255, 0));
    auto test_texture = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/uv-test.png"));
    auto container = make_shared<gfx::Phong>(make_shared<gfx::Texture>("assets/container.jpg"));
    auto cube_geo   = std::make_shared<gfx::Geometry>(cube_vertices_2, gfx::Geometry::POS_NORM_UV);
    auto ico_geo    = std::make_shared<gfx::Geometry>(ico_vertices, gfx::Geometry::POS_NORM_UV);
    auto triangle    = std::make_shared<gfx::Geometry>(triangle_vertices, gfx::Geometry::POS_NORM_UV);

    gfx::Object3D scene;

#if 1
    gfx::Skybox skybox({
        "assets/skybox/hd/right.jpg",
        "assets/skybox/hd/left.jpg",
        "assets/skybox/hd/top.jpg",
        "assets/skybox/hd/bottom.jpg",
        "assets/skybox/hd/front.jpg",
        "assets/skybox/hd/back.jpg",
    });
    scene.add(&skybox);
#endif
#if 1    
    gfx::Mesh ground(gfx::make_plane_geometry(50,50), test_texture);
    //ground.set_scale(glm::vec3(20, 0.5, 20));
    ground.set_position(glm::vec3(0, -1, 0));
    ground.set_scale(glm::vec3(10.0f));
    ground.receive_shadow = true;
    scene.add(&ground);
#endif
#if 1
    gfx::Light sun(gfx::Light::DIRECTIONAL, gfx::rgb(154, 219, 172));
    sun.set_position(glm::vec3(0.5f, 2.0f, 2.0f));
    sun.cast_shadow = true;
    scene.add(&sun);
#endif
#if 0 
    gfx::Mesh plane(gfx::make_plane_geometry(2, 2), test_texture);
    plane.set_position(glm::vec3(0, 2.0f, 0.0f));
    scene.add(&plane);
#endif
  
    auto position = glm::vec3(0.0f, 0.0f, 12.0f);
    auto velocity = glm::vec3(meter_per_second(150.0f), 0.0f, 0.0f);

    gfx::Object3D transform;
    transform.set_position(position);
    scene.add(&transform);

#if 0
    gfx::Mesh fuselage(cube_geo, container);
    fuselage.set_scale(glm::vec3(3.0f, 0.5f, 0.5f));
    transform.add(&fuselage);
#endif 
#if 0
    gfx::Mesh left_wing(cube_geo, container);
    left_wing.set_scale(glm::vec3(1.0f, 0.125f, 4.0f));
    left_wing.set_position(glm::vec3(0, 0.0, -2.0));
    transform.add(&left_wing);

    gfx::Mesh right_wing(cube_geo, container);
    right_wing.set_scale(glm::vec3(1.0f, 0.125f, 4.0f));
    right_wing.set_position(glm::vec3(0, 0.0, 2.0f));
    transform.add(&right_wing);

#endif 
#if 0
    gfx::Mesh elevator(cube_geo, container);
    elevator.set_scale(glm::vec3(1.0f, 0.125f, 2.0f));
    elevator.set_position(glm::vec3(-2.0f, 0.0f, 0.0f));
    transform.add(&elevator);
#endif 
#if 0
    gfx::Mesh rudder(cube_geo, container);
    rudder.set_scale(glm::vec3(1.0f, 1.0f, 0.125f));
    rudder.set_position(glm::vec3(-2.0f, 0.5f, 0.0f));
    transform.add(&rudder);
#endif 
#if 1
    gfx::Mesh cessna(std::make_shared<gfx::Geometry>(cessna_vertices, gfx::Geometry::POS_NORM_UV), grey);
    gfx::Mesh prop(std::make_shared<gfx::Geometry>(cessna_prop_vertices, gfx::Geometry::POS_NORM_UV), grey);
    transform.add(&cessna);
    transform.add(&prop);
#endif

    Aircraft aircraft(position, velocity);

    gfx::Camera camera(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 10000.0f);
    camera.set_position(glm::vec3(0, 1, 7));
    transform.add(&camera);

    gfx::Mesh test_cube(cube_geo, test_texture);
    test_cube.set_position(glm::vec3(20, 0, 0));
    test_cube.set_scale(glm::vec3(0.25f));
    transform.add(&test_cube);

    gfx::Mesh test_cube2(cube_geo, blue);
    test_cube2.set_position(glm::vec3(10, 0, 0));
    test_cube2.set_scale(glm::vec3(0.5f));
    transform.add(&test_cube2);


    gfx::OrbitController controller(15.0f);

    SDL_Event event;
    bool quit = false;
    uint64_t last = 0, now = SDL_GetPerformanceCounter();
    gfx::Seconds dt, timer = 0, log_timer = 0;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //glm::vec3 normal = Wing::calculate_wing_normal(1.0f);
    //std::cout << "wing_normal = " << normal << std::endl;


    float elevator_incidence = 0;
    float aileron_incidence = 0;


    
    while (!quit)
    {
        // delta time in seconds
        last = now;
        now = SDL_GetPerformanceCounter();
        dt = static_cast<gfx::Seconds>((now - last) / static_cast<gfx::Seconds>(SDL_GetPerformanceFrequency()));
        dt = min(dt, 0.02);

        if ((timer += dt) >= 1.0f)
        {
            printf("dt = %f, fps = %f\n", dt, 1 / dt);
            timer = 0.0f;
        }

        // user input
        while (SDL_PollEvent(&event) != 0)
        {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
                break;
            }
            case SDL_MOUSEMOTION: {
                controller.move_mouse(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                break;
            }
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: {
                    quit = true;
                }
                default:
                    break;
                }
                break;
            }
            }
        }
            
        aircraft.left_wing.lift_multiplier = 1.0f;
        aircraft.right_wing.lift_multiplier = 1.0f;
        aircraft.elevator.lift_multiplier = 1.0f;

        const uint8_t* key_states = SDL_GetKeyboardState(NULL);

        float elevator_torque = 15000.0f, aileron_torque = 10000.0f;

        if (key_states[SDL_SCANCODE_A])
        {
            //aircraft.rigid_body.add_relative_torque(phi::BACKWARD * aileron_torque);
            //aircraft.right_wing.lift_multiplier = 1.5;
            aileron_incidence += 0.01;
        }

        if (key_states[SDL_SCANCODE_D])
        {
            //aircraft.rigid_body.add_relative_torque(phi::FORWARD * aileron_torque);
            //aircraft.left_wing.lift_multiplier = 1.5;
            aileron_incidence -= 0.01;
        }

        if (key_states[SDL_SCANCODE_W])
        {
            elevator_incidence += 0.01;
            //aircraft.rigid_body.add_relative_torque(phi::LEFT * elevator_torque);
            //aircraft.elevator.lift_multiplier = 1.0f;
        }

        if (key_states[SDL_SCANCODE_S])
        {
            elevator_incidence -= 0.01;
            //aircraft.rigid_body.add_relative_torque(phi::RIGHT * elevator_torque);
            //aircraft.elevator.lift_multiplier = 2.0f;
        }

        if (key_states[SDL_SCANCODE_J])
        {
            aircraft.engine.throttle -= 0.1f;
            aircraft.engine.throttle = clamp(aircraft.engine.throttle, 0.0f, 1.0f);
        }

        if (key_states[SDL_SCANCODE_K])
        {
            aircraft.engine.throttle += 0.1f;
            aircraft.engine.throttle = clamp(aircraft.engine.throttle, 0.0f, 1.0f);

        }
        
        aircraft.elevator.normal = Wing::calculate_wing_normal(elevator_incidence);
        aircraft.left_aileron.normal = Wing::calculate_wing_normal(aileron_incidence);
        aircraft.right_aileron.normal = Wing::calculate_wing_normal(-aileron_incidence);
        aircraft.update(dt);

#if 0
        float factor = 0.0001f;
        std::cout << aircraft.left_wing.lift << ", " << aircraft.right_wing.lift << std::endl;
        left_wing.set_scale(glm::vec3(1, aircraft.left_wing.lift * factor, 2.0f));
        right_wing.set_scale(glm::vec3(1, aircraft.right_wing.lift * factor, 2.0f));
#endif

        solve_constraints(aircraft.rigid_body);
        apply_to_object3d(aircraft.rigid_body, transform);

        //controller.update(camera, aircraft.rigid_body.position, dt);
        controller.update(camera, camera.parent->get_position(), dt);

        prop.set_rotation(prop.get_rotation() + glm::vec3(0.01f, 0, 0));
        //test_cube.set_position(aircraft.left_wing. * 10.0f);

        renderer.render(camera, scene);

        // swap window
        SDL_GL_SwapWindow(window);
    }

    return 0;
}
