#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "HandmadeMath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SOKOL_LOG_IMPL
#include "sokol_log.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_gfx.h"

// Sokol Gfx resources
sg_pipeline pip;
sg_bindings bind;

// Window and rendering parameters
const int window_width = 800;
const int window_height = 600;

SDL_Window* window = NULL;
SDL_GLContext context = NULL;

bool init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("Sokol Gfx Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL);

    if (!window)
    {
        printf("Failed to create SDL window: %s\n", SDL_GetError());
        return false;
    }

    context = SDL_GL_CreateContext(window);

    if (!context)
    {
        printf("Failed to create OpenGL context: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(1);

    return true;
}

typedef struct {
    float mvp[16];
} vs_params_t;

void frame(void)
{
    vs_params_t vs_params = {
        /*.mvp = { 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};*/

        .mvp = { 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1}
    };

    sg_pass_action pass_action = {
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f } }
    };

    sg_begin_default_pass(&pass_action, window_width, window_height);
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &SG_RANGE(vs_params));
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
}

sg_image img;
sg_image_data img_data;

int main()
{
    if (!init_sdl())
    {
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    int image_w, image_h, image_c;
    unsigned char* bitmap = stbi_load("./boomer.png", &image_w, &image_h,
                                                      &image_c, 0);

    // Sokol Gfx initialization

    sg_setup(&(sg_desc){
        .logger.func = slog_func
    });

    // Create a Sokol Gfx triangle
    float vertices[] = {
        // positions            // colors                //uvs
         0.0f,  0.5f, 0.5f,     1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
    };

    sg_buffer_desc vbuf_desc = {
        .size = sizeof(vertices),
        .data = vertices,
        .label = "vertex buffer"
    };

    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    sg_shader_desc shader_desc = {
        .vs.uniform_blocks[0] = {
            .size = sizeof(vs_params_t),
            .layout = SG_UNIFORMLAYOUT_NATIVE,  // this is the default and can be omitted
            .uniforms = {
                // order must be the same as in 'params_t':
                [0] = { .name = "mvp", .type = SG_UNIFORMTYPE_MAT4 },
            }
         },
        .vs.source =
            "#version 330\n"
            //"uniform vs_params {\n"
            //"  mat4 mvp;\n"
            //"};\n"
            "uniform mat4 mvp;\n"
            "in vec4 a_pos;\n"
            "in vec4 a_col;\n"
            "in vec2 a_uv;\n"
            "out vec4 col;\n"
            "out vec2 uv;\n"
            "void main() {\n"
            "  gl_Position = mvp * a_pos;\n"
            "  col = a_col;\n"
            "  uv = a_uv;\n"
            "}\n",
        .fs.images[0] = { .name="u_tex", .image_type=SG_IMAGETYPE_2D },
        .fs.source =
            "#version 330\n"
            "in vec4 col;\n"
            "in vec2 uv;\n"
            "uniform sampler2D u_tex;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = texture(u_tex, uv) * col;\n"
            "}\n"
    };

    sg_shader shader = sg_make_shader(&shader_desc);

    sg_pipeline_desc pip_desc = {
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT4,
                [2].format = SG_VERTEXFORMAT_FLOAT2
            }
        },
        .shader = shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .index_type = SG_INDEXTYPE_NONE,
        .label = "pipeline"
    };

    pip = sg_make_pipeline(&pip_desc);

    bind.fs_images[0] = sg_make_image(&(sg_image_desc){
        .width = image_w,
        .height = image_h,
        .data.subimage[0][0] = { .ptr = bitmap, .size = image_w * image_h * image_c }
    });

    bind.vertex_buffers[0] = vbuf;

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            done = true;
                            break;
                    }
                    break;
            }
        }

        frame();

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    sg_shutdown();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

