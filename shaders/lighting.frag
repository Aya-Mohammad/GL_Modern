#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D ourTexture;

uniform bool isShadowed;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 texColor = texture(ourTexture, TexCoord).rgb;

    vec3 ambient = 0.1 * texColor;

    vec3 result;

    if(!isShadowed)
    {
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
        vec3 sunLightColor = vec3(1.0, 0.97, 0.9);
        vec3 diffuse = diff * sunLightColor * texColor;

        result = ambient + diffuse;
    }
    else
    {
        result = ambient;
    }

    FragColor = vec4(result, 1.0);
}
