#include <vector>
// #include "SDL_FontCache.h"
#include "pixello.hpp"

struct window_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool is_running;
    int width;
    int height;
};

static log_func log_callback = nullptr;
static window_t window;

static inline void log(const std::string &msg) {
    if (log_callback == nullptr) {
        return;
    }

    log_callback(msg);
}

/**
 *  PUBLIC
 */
void set_logger(log_func log_c) {
    log_callback = log_c;
}


bool init() {
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        log("SDL could not initialize! SDL_Error: %s\n" + std::string(SDL_GetError()));
        return false;
    }

    return true;
}


bool create_window(const std::string &name, const int width, const int height) {
    window_t new_window;

    new_window.window = SDL_CreateWindow(name.c_str(),
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         width,
                                         height,
                                         SDL_WINDOW_SHOWN);

    if (new_window.window == nullptr) {
        log("Window could not be created! SDL_Error: %s\n" + std::string(SDL_GetError()));
        return false;
    }

    new_window.renderer = SDL_CreateRenderer(new_window.window, -1, SDL_RENDERER_ACCELERATED);

    if (new_window.renderer == nullptr) {
        log("Renderer could not be created! SDL_Error: %s\n" + std::string(SDL_GetError()));
        return false;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(new_window.renderer, &info);
    log("Renderer name: " + std::string(info.name));
    log("Texture formats: ");

    for (Uint32 i = 0; i < info.num_texture_formats; i++) {
        log(std::string(SDL_GetPixelFormatName(info.texture_formats[i])));
    }

    new_window.width = width;
    new_window.height = height;
    new_window.is_running = false;

    window = new_window;
    return true;
}


bool run(update_func update_function, const int scale) {
    if (scale < 1) {
        log("Scale should be at least 1");
        return false;
    }

    // FC_Font *font = FC_CreateFont();

    // if (FC_LoadFont(font, window.renderer,
    //                 "fonts/FreeSans.ttf",
    //                 50,
    //                 FC_MakeColor(1, 1, 1, 255),
    //                 TTF_STYLE_NORMAL) == 0) {

    //     log("Load Font  failed");
    //     return false;

    // }

    window.is_running = true;

    const unsigned int texWidth = window.width / scale;
    const unsigned int texHeight = window.height / scale;
    SDL_Texture *texture = SDL_CreateTexture(
                               window.renderer,
                               SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               texWidth,
                               texHeight);

    unsigned char *pixels = (unsigned char *) malloc(texWidth * texHeight * 4);

    SDL_Event event;

    while (window.is_running) {

        // FC_Draw(font, window.renderer, 100, 100, "This is %s.\n It works.", "example text");

        const Uint64 start = SDL_GetPerformanceCounter();

        // SDL_SetRenderDrawColor(window.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        // SDL_RenderClear(window.renderer);

        while (SDL_PollEvent(&event)) {
            if ((SDL_QUIT == event.type) ||
                    (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode)) {
                window.is_running = false;
                break;
            }
        }

        update_function(pixels, texWidth, texHeight, 4);
        // splat down some random pixels
        // for (unsigned int i = 0; i < 1000; i++) {
        //     const unsigned int x = rand() % texWidth;
        //     const unsigned int y = rand() % texHeight;

        //     const unsigned int offset = (texWidth * 4 * y) + x * 4;
        //     pixels[ offset + 0 ] = rand() % 256;        // b
        //     pixels[ offset + 1 ] = rand() % 256;        // g
        //     pixels[ offset + 2 ] = rand() % 256;        // r
        //     pixels[ offset + 3 ] = SDL_ALPHA_OPAQUE;    // a
        // }

        //unsigned char* lockedPixels;
        //int pitch;
        //SDL_LockTexture
        //    (
        //    texture,
        //    NULL,
        //    reinterpret_cast< void** >( &lockedPixels ),
        //    &pitch
        //    );
        //std::copy( pixels.begin(), pixels.end(), lockedPixels );
        //SDL_UnlockTexture( texture );


        SDL_UpdateTexture(texture,
                          NULL,
                          pixels,
                          texWidth * 4);

        SDL_RenderCopy(window.renderer, texture, NULL, NULL);
        SDL_RenderPresent(window.renderer);

        const Uint64 end = SDL_GetPerformanceCounter();
        const static Uint64 freq = SDL_GetPerformanceFrequency();
        const double seconds = (end - start) / static_cast< double >(freq);

        log("Frame time: " + std::to_string(seconds * 1000.0) + "ms");
    }

    // FC_FreeFont(font);

    return true;
}


void close() {
    //Destroy window
    SDL_DestroyWindow(window.window);
    //Quit SDL subsystems
    SDL_Quit();
}