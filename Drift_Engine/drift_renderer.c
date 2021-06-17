#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h" 

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "drift_renderer.h"

// Utilites

// TODO: Create shader from file
internal shader CreateShader(const char *vertex_shader_source,
                             const char *fragment_shader_source)
{
    b32 success;
    char info_log[512];

    // Vertex  Shader
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
        DriftLogWarning("%s", info_log);
    }

    // Fragment Shader
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
        DriftLogWarning("%s", info_log);
    }

    // Link Shaders
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        DriftLogWarning("%s", info_log);
    }

    // Delete Shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return (shader)shader_program;
}

internal void UseShader(shader shader)
{
    glUseProgram(shader);
}

internal void DetachShader()
{
    glUseProgram(0);
}

internal void UploadMatrix4f(shader shader, matrix4f matrix, char *name)
{
    int location = glGetUniformLocation(shader, name); 
    glUniformMatrix4fv(location, 1, GL_FALSE, &matrix.elements[0][0]);
}

internal void Upload3f(shader shader, v3 vec, char *name)
{
    int location = glGetUniformLocation(shader, name);
    glUniform3f(location, vec.x, vec.y, vec.z);
}

internal void Upload4f(shader shader, v4 vec, char *name)
{
    int location = glGetUniformLocation(shader, name);
    glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

internal void Upload1i(shader shader, u32 val, char *name)
{
    int location = glGetUniformLocation(shader, name);
    glUniform1i(location, val);
}

// Textures
    

internal texture CreateTexture(char *filename)
{
    texture tex;
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // TODO: Add flags for texture GL options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(1);
    u8 *data = stbi_load(filename, &tex.width, &tex.height, &tex.channels, 0);

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        DriftLogWarning("stbi_load could not load texture");
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

internal texture CreateTextureFromData(u8 *data, int texture_width,
                                       int texture_height)
{
    texture tex;
    tex.width = texture_width;
    tex.height = texture_height;
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // TODO: Add flags for texture GL options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (data)
    {
        // TODO: Handle Access Violation
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        DriftLogWarning("stbi_load could not load texture");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

// Font

internal void GenTexFromFontBM(font *font, u8 *bitmap)
{
    glGenTextures(1, &font->texture_atlas);
    glBindTexture(GL_TEXTURE_2D, font->texture_atlas);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, font->texture_size, font->texture_size,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, bitmap);
    glGenerateMipmap(GL_TEXTURE_2D);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void CloseFont(font *font)
{
    if (font->info)
    {
        free(font->info);
    }

    if (font->characters)
    {
        free(font->characters);
    }

    font->texture_atlas = 0;
}

internal void InitFontPacked(font *font, u8 *font_memory)
{
    u8 *bitmap;
    font->texture_size = 512;
    int tex_size;
    for(;;)
    {
        tex_size = font->texture_size;
        bitmap = malloc(tex_size * tex_size);

        stbtt_pack_context pack_context;
		stbtt_PackBegin(&pack_context, bitmap, tex_size, tex_size, 0, 1, 0);
		stbtt_PackSetOversampling(&pack_context, 1, 1);

        if (!stbtt_PackFontRange(&pack_context, font_memory, 0,
                                 font->size, 32, 95, font->characters))
        {
            // Texture size not big enough
			free(bitmap);
			stbtt_PackEnd(&pack_context);
			font->texture_size *= 2;
		}
        else
        {
			stbtt_PackEnd(&pack_context);
			break;
		}
    }

    u8 *reverse_bitmap = (u8 *)malloc(tex_size * tex_size);
    for (int i = tex_size - 1; i >= 0; --i)
    {
        memcpy(&reverse_bitmap[(tex_size - 1 - i) * tex_size],
               &bitmap[i * tex_size], tex_size);
    }
    
    GenTexFromFontBM(font, reverse_bitmap);
    free(bitmap);
    free(reverse_bitmap);
}

internal void InitFontBaked(font *font, u8 *font_memory)
{
    // Get best texture size
    u8 *bitmap;
    font->texture_size = 512;
    int tex_size;
    for(;;)
    {
        tex_size = font->texture_size;
        bitmap = malloc(tex_size * tex_size);

        if (!stbtt_BakeFontBitmap((u8 *)font_memory, 0, font->size, bitmap,
                                  tex_size, tex_size, 32, 95, font->char_data))
        {
            // Texture size not big enough
			free(bitmap);
			font->texture_size *= 2;
		}
        else
        {
			break;
		}
    }

    u8 *reverse_bitmap = (u8 *)malloc(tex_size * tex_size);
    for (int i = tex_size - 1; i >= 0; --i)
    {
        memcpy(&reverse_bitmap[(tex_size - 1 - i) * tex_size],
               &bitmap[i * tex_size], tex_size);
    }

    GenTexFromFontBM(font, reverse_bitmap);
    free(bitmap);
    free(reverse_bitmap);
}

internal b32 InitFont(font *font, char *file, int font_size)
{
    read_file_result font_read_result = platform->ReadFile(file);
	font->info = malloc(sizeof(stbtt_fontinfo));
	font->characters = malloc(sizeof(stbtt_packedchar) * 96);
    font->size = font_size;

    if (!stbtt_InitFont(font->info, (u8 *)font_read_result.memory, 0))
    {
        CloseFont(font);
        font = NULL;
        return 0;
    }

    InitFontPacked(font, (u8 *)font_read_result.memory);

    stbtt_GetFontVMetrics(font->info, &font->ascent,
                          &font->descent, &font->line_gap);

    font->scale = stbtt_ScaleForPixelHeight(font->info, font->size);
    font->baseline = (int)(font->ascent * font->scale);

    platform->FreeFileMemory(font_read_result.memory);
    return 1;
}

internal f32 GetTextWidth(font *font, char *text)
{
    f32 width = 0;

    for (int i = 0; text[i]; ++i)
    {
        if (text[i] >= 32 && text[i] < 128)
        {
            stbtt_packedchar *info = &font->characters[text[i] - 32];
            width += info->xadvance;
        }
    }

    return width;
}

// Core

internal void ClearScreen(v4 color)
{
    glClearColor(color.r, color.g, color.b, color.a); 
    glClear(GL_COLOR_BUFFER_BIT);
}

internal void InitRenderer(renderer *renderer)
{
    if (!gladLoadGL()) 
    {
        DriftLogError("Glad did not load");
    }

    // TODO: Add assest path to platform
    if (!InitFont(&renderer->d_font, "W:\\drift_engine\\assets\\arial.ttf", 18))
    {
        DriftLogWarning("Error with initializg font");
    }

    // Enable opacity
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    u32 indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    // Primitives
    renderer->shader = CreateShader(default_vertex_shader,
                                    default_fragment_shader);

    float vertices[] = {
        // position    // color
         0.5f,  0.5f,   1.f, 1.f, 1.f, 1.f, // top right
         0.5f, -0.5f,   1.f, 1.f, 1.f, 1.f, // bottom right
        -0.5f, -0.5f,   1.f, 1.f, 1.f, 1.f, // bottom left
        -0.5f,  0.5f,   1.f, 1.f, 1.f, 1.f  // top left
    };

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &renderer->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                 indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    // Texture and Text
    renderer->texture_shader = CreateShader(texture_vertex_shader,
                                            texture_fragment_shader);

    renderer->text_shader = CreateShader(text_vertex_shader,
                                         text_fragment_shader);

    float texture_vertices[] = {
        // position     // texture coords
         0.5f,  0.5f,   1.f, 1.f, // top right
         0.5f, -0.5f,   1.f, 0.f, // bottom right
        -0.5f, -0.5f,   0.f, 0.f, // bottom left
        -0.5f,  0.5f,   0.f, 1.f  // top left 
    };

    glGenVertexArrays(1, &renderer->texture_vao);
    glBindVertexArray(renderer->texture_vao);

    glGenBuffers(1, &renderer->texture_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->texture_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertices),
                 texture_vertices, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &renderer->texture_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->texture_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                 indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 
}

internal void BeginRenderer(renderer *renderer,
                            int window_width, int window_height)
{
    renderer->window_width = window_width;
    renderer->window_height = window_height;

    glViewport(0, 0, window_width, window_height);

    renderer->projection_matrix =
        OrthographicMatrix(0.f, (f32)window_width, (f32)window_height, 0.f, 0.f, 100.f);

    renderer->render_list_count = 0;
}

internal void SubmitRenderer(renderer *renderer)
{
    for (u32 i = 0; i < renderer->render_list_count; ++i)
    {
        render_object obj = renderer->render_list[i];
        switch(obj.type)
        {
            case RENDER_TYPE_line:
            {
                UploadMatrix4f(renderer->shader,
                               renderer->projection_matrix,
                               "projection");

                glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0,
                                12 * sizeof(float), obj.vertices);

                UseShader(renderer->shader);
                glBindVertexArray(renderer->vao);
                glDrawArrays(GL_LINES, 0, 2);
            } break;

            case RENDER_TYPE_triangle:
            {
                UploadMatrix4f(renderer->shader,
                               renderer->projection_matrix,
                               "projection");

                glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0,
                                18 * sizeof(float), obj.vertices);

                UseShader(renderer->shader);
                glBindVertexArray(renderer->vao);
                glDrawArrays(GL_TRIANGLES, 0, 3);
            } break;

            case RENDER_TYPE_rect:
            {
                UploadMatrix4f(renderer->shader,
                               renderer->projection_matrix,
                               "projection");

                glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0,
                                24 * sizeof(float), obj.vertices);

                UseShader(renderer->shader);
                glBindVertexArray(renderer->vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            } break;

            case RENDER_TYPE_texture:
            {
                UploadMatrix4f(renderer->texture_shader,
                               renderer->projection_matrix,
                               "projection");

                // Get the data
                glBindBuffer(GL_ARRAY_BUFFER, renderer->texture_vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0,
                                16 * sizeof(float), obj.vertices);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                // Specify Texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.texture->id);

                Upload1i(renderer->texture_shader, 0, "tex");

                // Draw texture
                UseShader(renderer->texture_shader);
                glBindVertexArray(renderer->texture_vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            } break;

            case RENDER_TYPE_text:
            {
                UploadMatrix4f(renderer->text_shader,
                               renderer->projection_matrix,
                               "projection");

                Upload3f(renderer->text_shader, obj.text.color, "color");

                // Specify Texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.text.texture_atlas);

                Upload1i(renderer->text_shader, 0, "text");

                UseShader(renderer->text_shader);
                glBindVertexArray(renderer->texture_vao);

                for (int i = 0; i < obj.text.char_count; ++i)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, renderer->texture_vbo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(float),
                                    &obj.text.vertices[i * 16]);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    // Draw character
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            } break;

            default: break;
        }
    }

    glBindVertexArray(0);
}

internal void RenderLine(renderer *renderer, v2 p1, v2 p2, v4 color)
{
    render_object obj = {0};
    {
        obj.type = RENDER_TYPE_line;

        // Point 1
        obj.vertices[0] = p1.x;
        obj.vertices[1] = p1.y;

        obj.vertices[2] = color.r;
        obj.vertices[3] = color.g;
        obj.vertices[4] = color.b;
        obj.vertices[5] = color.a;

        // Point 2
        obj.vertices[6] = p2.x;
        obj.vertices[7] = p2.y;

        obj.vertices[8] = color.r;
        obj.vertices[9] = color.g;
        obj.vertices[10] = color.b;
        obj.vertices[11] = color.a;
    }

    renderer->render_list[renderer->render_list_count++] = obj;
}

internal void RenderTriangle(renderer *renderer, v2 p1,
                             v2 p2, v2 p3, v4 color)
{
    render_object obj = {0};
    {
        obj.type = RENDER_TYPE_triangle;

        // Bottom Left
        obj.vertices[0] = p1.x;
        obj.vertices[1] = p1.y;

        obj.vertices[2] = color.r;
        obj.vertices[3] = color.g;
        obj.vertices[4] = color.b;
        obj.vertices[5] = color.a;

        // Bottom Right
        obj.vertices[6] = p2.x;
        obj.vertices[7] = p2.y;

        obj.vertices[8] = color.r;
        obj.vertices[9] = color.g;
        obj.vertices[10] = color.b;
        obj.vertices[11] = color.a;
        
        // Top Right
        obj.vertices[12] = p3.x;
        obj.vertices[13] = p3.y;

        obj.vertices[14] = color.r;
        obj.vertices[15] = color.g;
        obj.vertices[16] = color.b;
        obj.vertices[17] = color.a;
    }

    renderer->render_list[renderer->render_list_count++] = obj;
}

internal void RenderRect(renderer *renderer, v2 position, v2 size, v4 color)
{
    render_object obj = {0};
    {
        obj.type = RENDER_TYPE_rect;

        v2 tl = position;
        v2 br = v2(position.x + size.x, position.y + size.y);

        // Top Right
        obj.vertices[0] = br.x;
        obj.vertices[1] = tl.y;

        obj.vertices[2] = color.r;
        obj.vertices[3] = color.g;
        obj.vertices[4] = color.b;
        obj.vertices[5] = color.a;

        // Bottom Right
        obj.vertices[6] = br.x;
        obj.vertices[7] = br.y;

        obj.vertices[8] = color.r;
        obj.vertices[9] = color.g;
        obj.vertices[10] = color.b;
        obj.vertices[11] = color.a;

        // Bottom Left
        obj.vertices[12] = tl.x;
        obj.vertices[13] = br.y;

        obj.vertices[14] = color.r;
        obj.vertices[15] = color.g;
        obj.vertices[16] = color.b;
        obj.vertices[17] = color.a;

        // Top Left
        obj.vertices[18] = tl.x;
        obj.vertices[19] = tl.y;

        obj.vertices[20] = color.r;
        obj.vertices[21] = color.g;
        obj.vertices[22] = color.b;
        obj.vertices[23] = color.a;
    }

    renderer->render_list[renderer->render_list_count++] = obj;
}

internal void RenderTexture(renderer *renderer, v2 position,
                            v2 size, texture *tex)
{
    render_object obj = {0};
    {
        obj.type = RENDER_TYPE_texture;
        obj.texture = tex;

        v2 tl = position;
        v2 br = v2(position.x + size.x, position.y + size.y);

        // Top Right
        obj.vertices[0] = br.x;
        obj.vertices[1] = tl.y;

        obj.vertices[2] = 1.f;
        obj.vertices[3] = 1.f;

        // Bottom Right
        obj.vertices[4] = br.x;
        obj.vertices[5] = br.y;

        obj.vertices[6] = 1.f;
        obj.vertices[7] = 0.f;

        // Bottom Left
        obj.vertices[8] = tl.x;
        obj.vertices[9] = br.y;

        obj.vertices[10] = 0.f;
        obj.vertices[11] = 0.f;

        // Top Left
        obj.vertices[12] = tl.x;
        obj.vertices[13] = tl.y;

        obj.vertices[14] = 0.f;
        obj.vertices[15] = 1.f;
    }

    renderer->render_list[renderer->render_list_count++] = obj;
}

internal void RenderTextureAtlas(renderer *renderer, v2 position, v2 size,
                                 texture *tex, v2 tex_coord_tl, v2 tex_coord_br)
{
    render_object obj = {0};
    {
        obj.type = RENDER_TYPE_texture;
        obj.texture = tex;

        v2 tl = position;
        v2 br = v2(position.x + size.x, position.y + size.y);

        // Top Right
        obj.vertices[0] = br.x;
        obj.vertices[1] = tl.y;

        obj.vertices[2] = tex_coord_br.x;
        obj.vertices[3] = tex_coord_tl.y;

        // Bottom Right
        obj.vertices[4] = br.x;
        obj.vertices[5] = br.y;

        obj.vertices[6] = tex_coord_br.x;
        obj.vertices[7] = tex_coord_br.y;

        // Bottom Left
        obj.vertices[8] = tl.x;
        obj.vertices[9] = br.y;

        obj.vertices[10] = tex_coord_tl.x;
        obj.vertices[11] = tex_coord_br.y;

        // Top Left
        obj.vertices[12] = tl.x;
        obj.vertices[13] = tl.y;

        obj.vertices[14] = tex_coord_tl.x;
        obj.vertices[15] = tex_coord_tl.y;
    }

    renderer->render_list[renderer->render_list_count++] = obj;
}

internal void RenderText(renderer *renderer, font *font,
                         char *text, v2 position, v3 color)
{
    text_info t = {0};
    render_object obj = {0};
    {
        t.color = color;
        t.char_count = 0;
        t.texture_atlas = font->texture_atlas;
        obj.type = RENDER_TYPE_text;
    }

    int current_xoffset = 0;
    for (int i = 0; text[i]; ++i)
    {
        int char_index = text[i] - 32;
        stbtt_packedchar *info = &font->characters[char_index];
        //stbtt_bakedchar *info = &font->char_data[char_index];

        f32 x_pos = position.x + current_xoffset;
        f32 y_pos = position.y;

        stbtt_aligned_quad q;
        stbtt_GetPackedQuad(font->characters, font->texture_size,
                            font->texture_size, char_index, &x_pos, &y_pos, &q, 1);

        // Top Right
        t.vertices[i * 16 + 0] = q.x1;
        t.vertices[i * 16 + 1] = q.y1;

        t.vertices[i * 16 + 2] = q.s1;
        t.vertices[i * 16 + 3] = q.t1;

        // Bottom Right
        t.vertices[i * 16 + 4] = q.x1;
        t.vertices[i * 16 + 5] = q.y0;

        t.vertices[i * 16 + 6] = q.s1;
        t.vertices[i * 16 + 7] = q.t0;

        // Bottom Left
        t.vertices[i * 16 + 8] = q.x0;
        t.vertices[i * 16 + 9] = q.y0;

        t.vertices[i * 16 + 10] = q.s0;
        t.vertices[i * 16 + 11] = q.t0;

        // Top Left
        t.vertices[i * 16 + 12] = q.x0;
        t.vertices[i * 16 + 13] = q.y1;

        t.vertices[i * 16 + 14] = q.s0;
        t.vertices[i * 16 + 15] = q.t1;

        ++t.char_count;
        current_xoffset += info->xadvance;
    }

    obj.text = t;
    renderer->render_list[renderer->render_list_count++] = obj;
}
