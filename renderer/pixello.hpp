#pragma once
#include <SDL.h>
#include <SDL_render.h>
#include <string>

typedef void (* log_func)(const std::string &log);
typedef void (* update_func)(unsigned char *pixels,
                             const unsigned int w,
                             const unsigned int h,
                             const unsigned int c);


void set_logger(log_func log_callback);
bool init();
bool create_window(const std::string &name, const int width, const int height);
bool run(update_func update_function, const int scale);
void close();