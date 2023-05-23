#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
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

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

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

void frame(void)
{
    sg_pass_action pass_action = {
        .colors[0] = { .load_action=SG_LOADACTION_CLEAR, .clear_value={0.0f, 0.0f, 0.0f, 1.0f } }
    };

    sg_begin_default_pass(&pass_action, window_width, window_height);
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
}

int main()
{
    if (!init_sdl())
    {
        return -1;
    }

    // Sokol Gfx initialization
    sg_desc desc = {
        .pipeline_pool_size = 1,
        .buffer_pool_size = 2,
        .image_pool_size = 1,
        .shader_pool_size = 1,
        .pass_pool_size = 1
    };
    sg_setup(&desc);

    // Create a Sokol Gfx triangle
    float vertices[] = {
        // positions            // colors
         0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
    };

    sg_buffer_desc vbuf_desc = {
        .size = sizeof(vertices),
        .data = vertices,
        .label = "vertex buffer"
    };
    sg_buffer vbuf = sg_make_buffer(&vbuf_desc);

    sg_shader_desc shader_desc = {
        /*.vs.uniform_blocks[0] = {
            .size = sizeof(sg_default_uniform_block_t),
            .uniforms = {
                [0] = {.name="mvp", .type=SG_UNIFORMTYPE_MAT4}
            }
        },*/
        /*.fs.images[0] = {
            .name = "tex",
            .type = SG_IMAGETYPE_2D,
        },*/
        .vs.source =
            "#version 330\n"
            //"uniform mat4 mvp;\n"
            "in vec4 position;\n"
            "in vec4 color0;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  gl_Position = position;\n"
            "  color = color0;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n"
    };
    sg_shader shader = sg_make_shader(&shader_desc);

    sg_pipeline_desc pip_desc = {
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
                [1].format = SG_VERTEXFORMAT_FLOAT4
            }
        },
        .shader = shader,
        .primitive_type = SG_PRIMITIVETYPE_TRIANGLES,
        .index_type = SG_INDEXTYPE_NONE,
        .label = "pipeline"
    };
    pip = sg_make_pipeline(&pip_desc);

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

