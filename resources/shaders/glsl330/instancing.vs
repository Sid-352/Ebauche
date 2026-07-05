#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

in mat4 instanceTransform;

uniform mat4 matView;
uniform mat4 matProjection;

out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    mat4 mvpi = matProjection * matView * instanceTransform;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvpi * vec4(vertexPosition, 1.0);
}
