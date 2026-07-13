#version 330

in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDirection;
uniform vec4 lightColor;
uniform vec4 ambientColor;
uniform vec3 viewPosition;
uniform float rimStrength;

out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    vec3 normal = normalize(fragNormal);
    vec3 light = normalize(-lightDirection);
    vec3 view = normalize(viewPosition - fragPosition);

    float diffuse = max(dot(normal, light), 0.0);
    float rim = pow(1.0 - max(dot(normal, view), 0.0), 2.0) * rimStrength;
    vec3 lighting = ambientColor.rgb + lightColor.rgb * diffuse + lightColor.rgb * rim;

    finalColor = vec4(texel.rgb * lighting, texel.a);
}
