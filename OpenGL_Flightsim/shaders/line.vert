#version 330 core
layout (location = 0) in vec2 a_Pos;

void main() {
	gl_Position = vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
}

