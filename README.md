# Pixelate
A basic pixel editor

![](pixelate.GIF)

## Technology Stack Used: 
- Made from scratch, written in C
- SDL2 will be used for the linux port and stb_truetypes.h will be used for displaying text

## Features:
- **Canvas** - users are able to open a canvas, move it around, and draw and erase on it
- **Serialization** - basic canvas state binary serialization (NOTE: there is still problem with loading)
- **Layers** - basic layer system implemented with linked list

## Currently Working On:
- **GUI** - A immediate GUI system
- **OpenGL Rendering** - Currently rendering through DIBStretchBits, but will move to modren OpenGL renderering 
