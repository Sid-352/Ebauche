#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

out vec4 finalColor;

void main()
{
    vec3 viewDir = normalize(viewPos - fragPosition);
    float rim = max(dot(normalize(fragNormal), viewDir), 0.0); 
    float glowMultiplier = 0.6 + (rim * 0.8);
    finalColor = vec4(colDiffuse.rgb * glowMultiplier, colDiffuse.a);
}
