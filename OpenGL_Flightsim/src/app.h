#pragma once

#define SDL_MAIN_HANDLED
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include "gfx/gfx.h"

class App
{
 private:
  int m_width, m_height;

  bool m_quit = false;
  bool m_paused = false;

  unsigned int m_frames = 0;
  float m_seconds = 0.0f;

  uint64_t last = 0, now = 0;

  // SDL
  SDL_Window* m_window;
  SDL_GLContext m_context;

  // graphics
  gfx::Camera* m_camera;
  gfx::Object3D* m_scene;
  gfx::Renderer2 *m_renderer;

  void init_app();
  void destroy_app();
  void poll_events();
  void handle_keydown(SDL_Keycode sim);
  float delta_time();

 public:
  App(int w, int h, const std::string& name = "App");
  ~App();
  void execute();
};
