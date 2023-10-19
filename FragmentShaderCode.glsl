#version 330 core

out vec4 Color;
in vec2 UV;
// for diffuse lighting
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in float brightOffset;

uniform vec3 lightPositionWorld;
uniform vec3 diffuse2Position;
uniform sampler2D textureSampler0;
uniform vec3 eyePosition;


void main()
{
    // Diffuse 1: cyan
    vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
    float brightness = dot(lightVectorWorld, normalize(normalWorld));
    // diffuse: slightly blue toned light for the night vibes
    vec4 diffuseLight = vec4(0, brightness * 180/255.0, brightness, 1.0);
    
    // Diffuse 2: pink
    vec3 diffuse2Vector = normalize(diffuse2Position - vertexPositionWorld);
    float brightness2 = dot(diffuse2Vector, normalize(normalWorld));
    // diffuse: slightly blue toned light for the night vibes
    vec4 diffuse2Light = vec4(brightness2 * 192.0/255.0, brightness2 * 22.0/255.0, brightness2 * 214.0/255.0, 1.0);
    
    // specular
    // calc direction
    vec3 reflectedVector = reflect(-lightVectorWorld, normalWorld);
    //calc direction from eye to obj
    vec3 eyeObj = normalize(eyePosition - vertexPositionWorld);
    // calc brightness
    float s = clamp(dot(reflectedVector, eyeObj), 0, 1);
    // control lobe
    s = pow(s, 2);
    vec4 specularLight = vec4(0, s * 145/255.0, s, 1.0);
    
    vec4 factor = vec4(0.5f, 0.5f, 0.5f, 1);

    // add together the light sources
    Color = factor * vec4(brightOffset - 0.3, brightOffset - 0.3, brightOffset - 0.3, 1) + factor * vec4(texture(textureSampler0, UV).rgb, 1.0)
    + factor * clamp(diffuseLight, 0, 0.5)
    + factor * clamp(diffuse2Light, 0, 0.5)
    + 0.5 * specularLight;

}
