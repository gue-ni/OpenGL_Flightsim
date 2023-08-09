#pragma once

class App
{
 private:
     bool m_close = false; 
     bool m_paused = false; 

 public:
  App() {}
  ~App() {}

  void handle_user_input() {}

  float delta_time()
  {
      return 0.0f;
  }

  void game_loop() {
      while(!m_close)
      {
      }
  }
};
