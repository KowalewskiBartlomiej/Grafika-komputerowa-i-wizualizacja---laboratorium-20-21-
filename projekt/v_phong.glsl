#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Zmienne interpolowane z okreœlonym miejscem na karcie graficznej
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

//Zmienne interpolowane
out vec3 pixel_position;
out vec3 pixel_normal;
out vec2 itexcoord;

void main(void) {
    
    pixel_position = vec3(M*vec4(Pos, 1.0)); //pozycja piksela w przestrzeni modelu
    pixel_normal = Normal; //wektor normalny danego wierzcho³ka
    itexcoord = TexCoords;

    gl_Position=P*V*M*vec4(Pos, 1);
}
