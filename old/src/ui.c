#include "ui.h"

// Based on Casey Muratori's and Ryan Fleury's immediate mode ui implementation 
// more information : https://caseymuratori.com/blog_0001
//                  : https://www.youtube.com/watch?v=ijJMMFdYJHc

internal ui_id UIIDNull()
{
    ui_id id = { -1, -1 };
    return id;
}

internal ui_id UIIDInit(u32 primary, u32 secondary)
{
    ui_id id = { primary, secondary };
    return id;
}

internal b32 UIIDEqual(ui_id id1, ui_id id2)
{
    return (id1.primary == id2.primary &&
            id1.secondary == id2.secondary);
}

internal v4 UIGetNextAutoLayoutRect(ui *ui)
{
    v4 rect = {0};
    
    if(ui->auto_layout_stack_pos > 0)
    {
        u32 i = ui->auto_layout_stack_pos - 1;
        
        rect.x      = ui->auto_layout_stack[i].position.x;
        rect.y      = ui->auto_layout_stack[i].position.y;
        rect.width  = ui->auto_layout_stack[i].size.x;
        rect.height = ui->auto_layout_stack[i].size.y;
        
        if(ui->auto_layout_stack[i].is_column)
        {
            rect.y += ui->auto_layout_stack[i].progress;
            ui->auto_layout_stack[i].progress += rect.height;
        }
        else
        {
            rect.x += ui->auto_layout_stack[i].progress;
            ui->auto_layout_stack[i].progress += rect.width;
        }
    }
    else
    {
        // SoftAssert("Auto-layout attempted without proper auto-layout group." == 0);
        rect = v4(0, 0, 64, 64);
    }
    
    return rect;
}

internal v4 UIGetPannelAutoLayoutRect(ui *ui, v4 rel_rect)
{
    v4 rect = {0};
    
    if(ui->auto_layout_stack_pos > 0)
    {
        u32 i = ui->auto_layout_stack_pos - 1;
        
        rect.x      = ui->auto_layout_stack[i].position.x + rel_rect.x;
        rect.y      = ui->auto_layout_stack[i].position.y + rel_rect.y;
        rect.width  = rel_rect.width;
        rect.height = rel_rect.height;
    }
    else
    {
        // SoftAssert("Auto-layout attempted without proper auto-layout group." == 0);
    }
    
    return rect;
}

internal void UIBeginFrame(ui *ui, offscreen_buffer *buffer, input *input)
{
    ui->buffer = buffer;

    ui->mouse_x          = input->mouse_x;
    ui->mouse_y          = input->mouse_y;
    ui->left_mouse_down  = input->left_mouse_down;
    ui->right_mouse_down = input->right_mouse_down;

    ui->widget_count = 0;
}

internal void UIEndFrame(ui *ui)
{
    for(u32 i = 0; i < ui->widget_count; ++i)
    {
        ui_widget *widget = ui->widgets + i;
        
        widget->t_hot    += ((f32)(!!UIIDEqual(ui->hot, widget->id)) - widget->t_hot);
        widget->t_active += ((f32)(!!UIIDEqual(ui->active, widget->id)) - widget->t_active);
        
        switch(widget->type)
        {
            case UI_WIDGET_pannel: 
            {
                v4 titlebar_rect = v4(widget->rect.x, widget->rect.y, widget->rect.width, 25);
                DrawFilledRect(ui->buffer, widget->rect, v4(0.5f, 0.5f, 0.5f, 1.f));
                DrawFilledRect(ui->buffer, titlebar_rect, v4(1.f, 1.f, 1.f, 1.f));
            } break;

            case UI_WIDGET_button:
            {
                v4 color = {
                    0.6f + widget->t_hot * 0.4f - widget->t_active * 0.5f,
                    0.6f + widget->t_hot * 0.4f - widget->t_active * 0.5f,
                    0.6f + widget->t_hot * 0.4f - widget->t_active * 0.5f,
                    0.6f + widget->t_hot * 0.4f - widget->t_active * 0.5f,
                };
                
                DrawFilledRect(ui->buffer, widget->rect, color);
            } break;
            
            case UI_WIDGET_slider:
            {
                DrawFilledRect(ui->buffer, widget->rect, v4(0.6f, 0.6f, 0.6f, 0.6f));

                DrawFilledRect(ui->buffer, 
                               v4(widget->rect.x, widget->rect.y,
                                  widget->rect.width * widget->slider.value,
                                  widget->rect.height),
                               v4(0.8f, 0.8f, 0.8f, 0.8f));
            } break;
            
            default:
            {
                Assert("Wrong type, does not exisit");
            } break;
        }
    }
}

internal void UIPushColumn(ui *ui, v2 position, v2 size)
{
    Assert(ui->auto_layout_stack_pos < UI_MAX_AUTO_LAYOUT_GROUPS);
    u32 i = ui->auto_layout_stack_pos++;
    ui->auto_layout_stack[i].is_column = 1;
    ui->auto_layout_stack[i].position = position;
    ui->auto_layout_stack[i].size = size;
    ui->auto_layout_stack[i].progress = 0;
}

internal void UIPopColumn(ui *ui)
{
    if (ui->auto_layout_stack_pos > 0)
    {
        --ui->auto_layout_stack_pos;
    }
}

internal v4 UIPannelP(ui *ui, ui_id id, char *text, v4 rect)
{

    b32 cursor_is_over = (ui->mouse_x >= rect.x &&
                          ui->mouse_x <= rect.x + rect.width &&
                          ui->mouse_y >= rect.y &&
                          ui->mouse_y <= rect.y + 25);

    if (!UIIDEqual(ui->hot, id) && cursor_is_over)
    {
        ui->hot = id;
    }
    else if (UIIDEqual(ui->hot, id) && !cursor_is_over)
    {
        ui->hot = UIIDNull();
    }

    if (!UIIDEqual(ui->active, id))
    {
        if (UIIDEqual(ui->hot, id))
        {
            if (ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
    }
    
    if (UIIDEqual(ui->active, id))
    {
        local_persist b32 offset_set = 0;
        local_persist int offset_x = 0;
        local_persist int offset_y = 0;

        if (ui->left_mouse_down)
        {
            if (!offset_set)
            {
                offset_x = ui->mouse_x - rect.x;
                offset_y = ui->mouse_y - rect.y;
                offset_set = 1;
            }

            rect.x = ui->mouse_x - offset_x;
            rect.y = ui->mouse_y - offset_y;
        }
        else
        {
            offset_set = 0;
            ui->active = UIIDNull();
        }
    }
    
    ui_widget *widget = ui->widgets + ui->widget_count++;
    widget->type = UI_WIDGET_pannel;
    widget->id = id;
    widget->rect = rect;

    return rect;
}

internal void UIPannel(ui *ui, ui_id id, char *text, v4 rect)
{
    local_persist b32 rect_set = 0;
    local_persist v4 window_rect = {0};

    if (!rect_set)
    {
        window_rect = UIPannelP(ui, id, text, rect);
        rect_set = 1;
    }
    else 
    {
        window_rect = UIPannelP(ui, id, text, window_rect);
    }

    UIPushColumn(ui, v2(window_rect.x, window_rect.y + 25), 
                v2(window_rect.width, window_rect.height));
}

internal void UIEndPannel(ui * ui)
{
    UIPopColumn(ui);
}

internal b32 UIMenuP(ui *ui, ui_id id, char *text, v4 rect)
{
    Assert(ui->widget_count < UI_MAX_WIDGETS);
    
    b32 is_open = UIIDEqual(ui->open_menu, id);
    
    b32 cursor_is_over = (ui->mouse_x >= rect.x &&
                          ui->mouse_x <= rect.x + rect.width &&
                          ui->mouse_y >= rect.y &&
                          ui->mouse_y <= rect.y + rect.height);
    
    if (!UIIDEqual(ui->hot, id) && cursor_is_over)
    {
        ui->hot = id;

        // If another menu is open cursor_is_over open_menu will change
        if (!UIIDEqual(ui->open_menu, UIIDNull()))
        {
            ui->open_menu = id;
        }
                
    }
    else if (UIIDEqual(ui->hot, id) && !cursor_is_over)
    {
        ui->hot = UIIDNull();
    }
    
    if (UIIDEqual(ui->active, id))
    {
        if(!ui->left_mouse_down)
        {
            is_open = (is_open == 0) ? 1 : 0;
            ui->active = UIIDNull();

            if (!is_open)
            {
                ui->open_menu = UIIDNull();
            }
        }
    }
    else if (UIIDEqual(ui->hot, id))
    {
        if (ui->left_mouse_down)
        {
            ui->active = id;
        }
    }
    else if (UIIDEqual(ui->hot, UIIDNull()))
    {
        if (ui->left_mouse_down && is_open)
        {
            is_open = 0;
            ui->open_menu = UIIDNull();
        }
    }
    
    ui_widget *widget = ui->widgets + ui->widget_count++;
    widget->type = UI_WIDGET_button;
    widget->id = id;
    widget->rect = rect;

    if (is_open)
    {
        ui->open_menu = id;
    }
    
    return is_open;
}

internal b32 UIButtonMenu(ui *ui, ui_id id, char *text, 
                          v4 button_rect, v4 menu_rect)
{
    b32 is_open = UIMenuP(ui, id, text, button_rect);
    UIPushColumn(ui, v2(menu_rect.x, menu_rect.y), 
                 v2(menu_rect.width, menu_rect.height));

    return is_open;
}

internal void UIEndMenu(ui *ui)
{
    UIPopColumn(ui);
}
    
internal b32 UIButtonP(ui *ui, ui_id id, char *text, v4 rect)
{
    Assert(ui->widget_count < UI_MAX_WIDGETS);
    
    b32 is_triggered = 0;
    
    b32 cursor_is_over = (ui->mouse_x >= rect.x &&
                          ui->mouse_x <= rect.x + rect.width &&
                          ui->mouse_y >= rect.y &&
                          ui->mouse_y <= rect.y + rect.height);
    
    if (!UIIDEqual(ui->hot, id) && cursor_is_over)
    {
        ui->hot = id;
    }
    else if (UIIDEqual(ui->hot, id) && !cursor_is_over)
    {
        ui->hot = UIIDNull();
    }
    
    if (UIIDEqual(ui->active, id))
    {
        if(!ui->left_mouse_down)
        {
            is_triggered = UIIDEqual(ui->hot, id);
            ui->active = UIIDNull();
            ui->open_menu = UIIDNull();
        }
    }
    else
    {
        if (UIIDEqual(ui->hot, id))
        {
            if (ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
    }
    
    ui_widget *widget = ui->widgets + ui->widget_count++;
    widget->type = UI_WIDGET_button;
    widget->id = id;
    widget->rect = rect;
    
    return is_triggered;
}

internal b32 UIButton(ui *ui, ui_id id, char *text)
{
    v4 rect = UIGetNextAutoLayoutRect(ui);
    return UIButtonP(ui, id, text, rect);
}

internal b32 UIPannelButton(ui *ui, ui_id id, char *text, v4 rect)
{
    v4 window_rect = UIGetPannelAutoLayoutRect(ui, rect);
    return UIButtonP(ui, id, text, window_rect);
}

internal f32 UISliderP(ui *ui, ui_id id, char *text, f32 value, v4 rect)
{
    Assert(ui->widget_count < UI_MAX_WIDGETS);
   
    b32 cursor_is_over = (ui->mouse_x >= rect.x &&
                          ui->mouse_x <= rect.x + rect.width &&
                          ui->mouse_y >= rect.y &&
                          ui->mouse_y <= rect.y + rect.height);
    
    if (!UIIDEqual(ui->hot, id) && cursor_is_over)
    {
        ui->hot = id;
    }
    else if (UIIDEqual(ui->hot, id) && !cursor_is_over)
    {
        ui->hot = UIIDNull();
    }
    
    if (!UIIDEqual(ui->active, id))
    {
        if (UIIDEqual(ui->hot, id))
        {
            if (ui->left_mouse_down)
            {
                ui->active = id;
            }
        }
    }
    
    if (UIIDEqual(ui->active, id))
    {
        if (ui->left_mouse_down)
        {
            value = (ui->mouse_x - rect.x) / rect.width;
        }
        else
        {
            ui->active = UIIDNull();
            ui->open_menu = UIIDNull();
        }
    }
    
    if (value < 0.f)
    {
        value = 0.f;
    }
    else if (value > 1.f)
    {
        value = 1.f;
    }
    
    ui_widget *widget = ui->widgets + ui->widget_count++;
    widget->type = UI_WIDGET_slider;
    widget->id = id;
    widget->rect = rect;
    widget->slider.value = value;
    
    return value;
}

internal f32 UISlider(ui *ui, ui_id id, char *text, f32 value)
{
    v4 rect = UIGetNextAutoLayoutRect(ui);
    return UISliderP(ui, id, text, value, rect);
}
