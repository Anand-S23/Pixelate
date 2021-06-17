#ifndef DRIFT_RENDERER_H
#define DRIFT_RENDERER_H

global const char *default_vertex_shader = "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec4 color;\n"

    "uniform mat4 projection;\n"

    "out vec4 f_color;\n"

    "void main()\n"
    "{\n"
        "gl_Position = projection * vec4(pos.x, pos.y, 0.f, 1.f);\n"
        "f_color = color;\n"
   "}\0";

global const char *default_fragment_shader = "#version 330 core\n"
    "in vec4 f_color;\n"
    "out vec4 frag_color;\n"

    "void main() {\n"
        "frag_color = f_color;\n"
    "}\n\0";

global const char *texture_vertex_shader = "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 texture_coord;\n"

    "uniform mat4 projection;\n"

    "out vec2 tex_coord;\n"

    "void main()\n"
    "{\n"
        "gl_Position = projection * vec4(pos.x, pos.y, 0.f, 1.f);\n"
        "tex_coord = vec2(texture_coord.x, texture_coord.y);\n"
   "}\0";

global const char *texture_fragment_shader = "#version 330 core\n"
    "in vec2 tex_coord;\n"
    "out vec4 frag_color;\n"

    "uniform sampler2D tex;\n"

    "void main() {\n"
        "frag_color = texture(tex, tex_coord);\n"
    "}\n\0";

global const char *text_vertex_shader = "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 texture_coord;\n"

    "uniform mat4 projection;\n"

    "out vec2 tex_coord;\n"

    "void main()\n"
    "{\n"
        "gl_Position = projection * vec4(pos.x, pos.y, 0.f, 1.f);\n"
        "tex_coord = vec2(texture_coord.x, 1.0 - texture_coord.y);\n"
   "}\0";

global const char *text_fragment_shader = "#version 330 core\n"
    "in vec2 tex_coord;\n"
    "out vec4 frag_color;\n"

    "uniform sampler2D text;\n"
    "uniform vec3 color;\n"

    "void main() {\n"

        "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, tex_coord).a);\n"
        "frag_color = vec4(color, 1.0) * sampled;\n"
    "}\n\0";

typedef unsigned int shader;

#define MAX_RENDER_OBJECTS 1000
#define MAX_TEXT_CHARACTERS 60

typedef struct font
{
	stbtt_fontinfo *info;
	stbtt_packedchar *characters;
    stbtt_bakedchar char_data[96];
    int type;
    u32 texture_atlas;
	int texture_size;
	f32 size;
	f32 scale;
	int ascent;
	int descent;
    int line_gap;
	int baseline;
} font;

typedef struct texture_buffer
{
    void *memory;
    int width;
    int height;
    int pitch;
} texture_buffer;

typedef struct texture
{
    u32 id;
    int width;
    int height;
    int channels;
} texture;

typedef enum render_type
{
    RENDER_TYPE_line, 
    RENDER_TYPE_triangle,
    RENDER_TYPE_rect,
    RENDER_TYPE_texture,
    RENDER_TYPE_text,
} render_type;

typedef struct text_info
{
    v3 color;
    u32 texture_atlas;
    u32 char_count;
    f32 vertices[MAX_TEXT_CHARACTERS * 16];
} text_info;

// TODO: More generic
typedef struct render_object
{
    render_type type;
    float vertices[24];
    texture *texture;
    text_info text;
} render_object;

typedef struct renderer
{
    int window_width;
    int window_height;
    matrix4f projection_matrix;
    
    shader shader;
    shader texture_shader;
    shader text_shader;

    u32 vao;
    u32 vbo;
    u32 ebo;

    u32 texture_vao;
    u32 texture_vbo;
    u32 texture_ebo;

    font d_font;

    render_object render_list[MAX_RENDER_OBJECTS];
    u32 render_list_count;
} renderer;

#endif
