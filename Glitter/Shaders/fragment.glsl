#version 450

out vec4 FragColor;

uniform vec2 screenSize;
uniform float time;

void main()
{
    vec3 colorA = vec3(1.0, 0.0, 0.0); // Purple
    vec3 colorB = vec3(0.0, 1.0, 0.0); // Pink
    vec3 colorC = vec3(0.0, 0.0, 1.0); // Teal blue

    // Create three dynamic colors
    vec3 color1 = mix(colorA, colorB, cos(time));
    vec3 color2 = mix(colorB, colorC, sin(time));

    vec2 uv = gl_FragCoord.xy / screenSize;

    // Transform to [(-1.0, -1.0), (1.0, 1.0)] range
    uv = 2.0 * uv - 1.0;

    // Have something to vary the radius (can also just be a linear counter (time))
    float wave = sin(time);

    // Calculate how near to the center (0.0) or edge (1.0) this fragment is
    float circle = uv.x * uv.x + uv.y * uv.y;

    // Put it all together
    FragColor = vec4(mix(color1, color2, circle + wave), 1.0);
}