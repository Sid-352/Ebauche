#version 330

// Input uniform values
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    finalColor = colDiffuse;
}
