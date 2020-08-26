#pragma once
#include <SDL.h>
#include <SDL_render.h>
#include <string>

typedef void (* log_func)(const std::string &log);

class Pixello {
  private:
    log_func log_callback = nullptr;

    int w = 640;
    int h = 480;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    void log(const std::string &msg);

  public:
    void set_logger(log_func log_callback);
    bool init(const std::string &name, const int width, const int height);
    bool close();
};