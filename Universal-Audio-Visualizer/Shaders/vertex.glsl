#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 a_Normal;

uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;
uniform mat4 u_ProjMatrix;
uniform mat4 u_ViewMatrix;

uniform vec3 u_Color;
uniform vec3 u_lightColor;
uniform vec3 u_lightDirection;

out vec4 v_Color;
void main()
{
    gl_Position = u_ProjMatrix * u_ViewMatrix * u_ModelMatrix * vec4(aPos, 1.0);

    vec3 lightDir = normalize(u_lightDirection);
    vec3 normal = normalize(vec3(u_NormalMatrix * a_Normal));

    float nDotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_lightColor * u_Color * nDotL;

    vec3 ambient = vec3(0.2, 0.2, 0.2) * u_Color;

    v_Color = vec4(clamp(diffuse + ambient, 0.0, 1.0), 1.0);
}
