#version 330 core

in vec3 PosWS;
in vec3 Normal;

out vec4 FragColor;

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    vec3 ambient = material.ambient * light.ambient;

    // diffuse 
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.lightPos - PosWS);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * diff * light.diffuse;

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(viewPos - PosWS);
    float spec = max(dot(reflectDir, viewDir), 0.0);
    
    // 32 is Shininess 
    spec = pow(spec, material.shininess);
    vec3 specular = spec * material.specular * light.specular;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}