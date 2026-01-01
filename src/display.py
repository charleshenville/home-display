import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLUT import *
import math
import ast

from perlin import SimplexNoise as noise

from datetime import datetime
import time

str_2_matrix_map = {}

font_size = 32

noise_obj = noise()

def init_opengl(width, height):
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0, width, 0, height, -1, 1)
    glMatrixMode(GL_MODELVIEW)

def draw_circle(x, y, radius, segments=100, color=(1.0, 1.0, 1.0)):
    glColor3f(*color)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(x, y)  # Center of the circle
    for i in range(segments + 1):  # +1 to close the circle
        theta = 2.0 * math.pi * i / segments
        dx = radius * math.cos(theta)
        dy = radius * math.sin(theta)
        glVertex2f(x + dx, y + dy)
    glEnd()

def render_text(width, height, time_str, time_param, dot_size=5, color=(1.0, 1.0, 1.0)):
    
    matrix = str_2_matrix_map.get(time_str, [])
    if not matrix:
        return  # Exit if matrix is empty or not found

    # Calculate the width and height of the matrix
    text_width = len(matrix[0]) * dot_size
    text_height = len(matrix) * dot_size

    # Centering the text on the screen
    x_offset = (width - text_width) / 2
    y_offset = (height + text_height) / 2

    # animation parameters
    speed = 0.03
    density = 0.03
    variance = 3

    for y, row in enumerate(matrix):
        for x, pixel in enumerate(row):
            if pixel:
                rval = noise_obj.noise3(x * density, y * density, time_param*speed)
                draw_circle(x_offset + x * dot_size, y_offset - y * dot_size, (dot_size / 2) + rval * variance, 100, color)
    # x_offset += len(matrix[0]) * dot_size + dot_size  # Adjust x_offset for the next character

def main():
    global str_2_matrix_map

    pygame.init()

    screen_info = pygame.display.Info()
    width, height = screen_info.current_w, screen_info.current_h

    screen = pygame.display.set_mode((width, height), DOUBLEBUF | OPENGL | pygame.FULLSCREEN)
    init_opengl(width, height)

    with open("./generated_matricies/gen.txt", 'r') as file:
        str_2_matrix_map = ast.literal_eval(file.read())
    
    dot_size = 8
    color = (1.0, 1.0, 1.0)

    last_time = ""
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return
        
        now = datetime.now()
        time_string = now.strftime('%H:%M')

        # if time_string != last_time:
        tp = time.perf_counter()
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        render_text(width, height, time_string, tp, dot_size, color)
        pygame.display.flip()
        pygame.time.wait(10)
        last_time = time_string

if __name__ == '__main__':
    main()
