#pragma once

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "flightmodel.h"
#include "gfx/gfx.h"

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_opengl3.h"
#include "../lib/imgui/imgui_impl_sdl2.h"

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
  bool m_orbitcamera = false;

  int m_cameratype = 0;

  unsigned int m_frames = 0;
  float m_seconds = 0.0f;

  uint64_t last = 0, now = 0;

  // SDL
  SDL_Window* m_window;
  SDL_GLContext m_context;
  SDL_Joystick* m_sdljoystick;

  // graphics
  std::array<gfx::Camera*, 3> m_cameras;
  gfx::Object3D* m_scene;
  gfx::Renderer* m_renderer;
  gfx::Mesh* m_screen;


  gfx::Renderer* m_hud_renderer;
  gfx::Line2d* m_hud;
  gfx::RenderTarget* m_hud_target;

  gfx::OrbitController m_controller;

  gfx::Object3D* m_camera_attachment;
  gfx::Object3D* m_runway;

  // physics
  Airplane* m_airplane;
  phi::RigidBody* m_terrain;
  Clipmap* m_clipmap;

  gfx::Object3D* m_falcon;

  void init_airplane();
  void init_flightmodel();

  // setup/teardown
  void init();
  void destroy();

  void draw_imgui(float dt);

  void game_loop(float dt);

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
  int run();
};
