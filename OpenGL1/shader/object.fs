#version 330 core

in vec3 PosWS;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

struct Material{
    
    sampler2D diffuseTex;
    sampler2D specularTex;
    sampler2D emissionTex;
    float shininess;
};

struct Light{
    vec3 lightPos;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform Material material;
uniform vec3 viewPos;

void main()
{

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuseTex, TexCoords).rgb;

    // diffuse 
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.lightPos - PosWS);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = texture(material.diffuseTex, TexCoords).rgb * diff * light.diffuse;

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(viewPos - PosWS);
    float spec = max(dot(reflectDir, viewDir), 0.0);
    
    // 32 is Shininess 
    spec = pow(spec, material.shininess);
    vec3 specular = spec * texture(material.specularTex, TexCoords).rgb * light.specular;

    //emission
    vec3 emission = texture(material.emissionTex, TexCoords).rgb;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}