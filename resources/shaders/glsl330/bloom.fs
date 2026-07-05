#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;

out vec4 finalColor;

void main()
{
    vec2 tex_offset = 1.0 / resolution; 
    vec3 result = vec3(0.0);
    float totalWeight = 0.0;

    float spread = 4.0;
    for(int x = -4; x <= 4; ++x) 
    {
        for(int y = -4; y <= 4; ++y) 
        {
            float w = 1.0 / (1.0 + float(x*x + y*y));
            result += texture(texture0, fragTexCoord + vec2(tex_offset.x * float(x) * spread, tex_offset.y * float(y) * spread)).rgb * w;
            totalWeight += w;
        }
    }
    result /= totalWeight;
    
    vec2 centerDist = fragTexCoord - vec2(0.5);
    float caOffset = length(centerDist) * 0.005;
    
    vec3 baseCol;
    baseCol.r = texture(texture0, fragTexCoord + centerDist * caOffset).r;
    baseCol.g = texture(texture0, fragTexCoord).g;
    baseCol.b = texture(texture0, fragTexCoord - centerDist * caOffset).b;
    
    vec3 finalGlow = baseCol + (result * 1.0); 
    
    float vignette = 1.0 - smoothstep(0.4, 1.2, length(centerDist));
    finalGlow *= vignette;
    
    finalColor = vec4(finalGlow, 1.0);
}
