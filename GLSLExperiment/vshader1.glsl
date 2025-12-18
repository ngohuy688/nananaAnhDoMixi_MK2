#version 400
in vec4 vPosition;
in vec3 vNormal;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 fNormal;
out vec3 fPosition;

void main()
{
    vec4 pos = View * Model * vPosition;
    fPosition = pos.xyz;
    fNormal = normalize((View * Model * vec4(vNormal, 0.0)).xyz);

    gl_Position = Projection * pos;
}//