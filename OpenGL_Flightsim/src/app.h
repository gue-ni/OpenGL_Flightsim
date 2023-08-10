#pragma once

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "gfx/gfx.h"
#include "flightmodel.h"

struct GameObject {
  gfx::Mesh* mesh;
  Airplane* rigid_body;
};

class App
{
 private:
  int m_width, m_height;

  bool m_quit = false;
  bool m_paused = false;
  bool m_orbitcamera = true;

  unsigned int m_frames = 0;
  float m_seconds = 0.0f;

  uint64_t last = 0, now = 0;

  // SDL
  SDL_Window* m_window;
  SDL_GLContext m_context;
  SDL_Joystick* m_sdljoystick;

  // graphics
  gfx::Camera* m_camera;
  gfx::Object3D* m_scene;
  gfx::Renderer2* m_renderer;

  gfx::OrbitController m_controller;

  gfx::Object3D* m_camera_attachment;

  // physics
  Airplane* m_airplane;
  phi::RigidBody *m_terrain;

  gfx::Mesh* m_falcon;

  // setup/teardown
  void init_app();
  void destroy_app();

  // user input
  void poll_events();
  void event_keydown(SDL_Keycode sim);
  void event_mousewheel(float value);
  void event_mousemotion(float xrel, float yrel);
  void event_joyaxis(uint8_t axis, int16_t value);

  float delta_time();

 public:
  App(int w, int h, const std::string& name = "App");
  ~App();
  void execute();
};
