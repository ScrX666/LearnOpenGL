#version 330 core

in vec3 PosWS;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - PosWS);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(viewPos - PosWS);
    float specular = max(dot(reflectDir, viewDir), 0.0);
    float specularLevel = 0.5;
    // 32 is Shininess 
    specular = pow(specular, 32) * specularLevel;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}