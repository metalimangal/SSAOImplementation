#version 330 core

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

uniform mat4 PVM;
uniform mat4 model;  // Model matrix to transform normals correctly

out vec3 frag_position; // Output to the fragment shader
out vec3 normal;       // Transformed normal

void main() {
    // Transform the vertex position to clip space
    gl_Position = PVM * vec4(vertex_position, 1.0);
    
    // Calculate the fragment position in world space
    frag_position = vec3(model * vec4(vertex_position, 1.0));
    
    // Transform the normal to world space
    normal = mat3(transpose(inverse(model))) * vertex_normal;
}
