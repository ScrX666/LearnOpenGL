#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

#define NUM_SAMPLES 100
#define PI 3.1415
//采样圈数
#define NUM_RINGS 10
vec2 poissonDisk[NUM_SAMPLES];

float rand_2to1(vec2 uv ) {//传入一个二维数，传出一个假随机数。
    // 0 - 1
    const float a = 12.9898, b = 78.233, c = 43758.5453;
    float dt = dot( uv.xy, vec2( a,b ));
    float sn = mod( dt, PI );
    return fract(sin(sn) * c);//只取小数部分（取值范围0~1，若为负+1）
}

void poissonDiskSamples(vec2 randomSeed)
{
    float angle_step = 2 * PI * float( NUM_RINGS)/float( NUM_SAMPLES);//角位移大小
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES); //采样数的倒数

    float angle = rand_2to1(randomSeed) * PI * 2;//初始角度（弧度）
    float radius = INV_NUM_SAMPLES;//初始半径
    float radiusStep = radius;     //半径增量

    for( int i = 0; i < NUM_SAMPLES; i ++ ) 
    {
      poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
      radius += radiusStep;//半径增加
      angle += angle_step;//弧度增加
    }
}

// 得到PCSS 的block平均值
float averageBlockDepth(vec3 projCoords,vec2 texelSize){
    float blockerZ = 0.0;//遮挡物总深度
    int count = 0;
    int r=5;
    //在一定范围内判断是否有遮挡物，最重要的采样方法
    poissonDiskSamples(projCoords.xy+vec2(0.1314,0.351));
    for(int i=0;i<NUM_SAMPLES;++i){
        float depth = texture(shadowMap, projCoords.xy + r * poissonDisk[i] * texelSize).r;
        if(depth < projCoords.z){//如果为遮挡物
            count++;
            blockerZ +=depth;
        }
    }

    if(count == 0||count==(r*2+1)*(r*2+1))return 1.0f;
    return blockerZ / count;
}

float PCF(vec3 projCoords, int radius)
{
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float currentDepth = projCoords.z;
    vec2  pixelSize = 1.0 / textureSize(shadowMap, 0);
    // 9 * 9 PCF
    float shadow = 0;
    for(int x = -radius ; x <= radius; ++x){
        for(int y = -radius; y <= radius; ++y){
            float pcfDepth = texture(shadowMap, projCoords.xy + pixelSize *  vec2(x, y)).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= (2*radius+1)*(2*radius+1);

    if(currentDepth > 1.0)
        shadow = 0;

    return shadow;
}

float PCSS(vec3 projCoords)
{
    const float weightOfLight = 10.0;

    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片段是否在阴影中
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //每像素偏移距离
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    //PCSS核心算法
    float visibility = 0.0;
    //第一步计算平均遮挡物深度
    float averBlocker = averageBlockDepth(projCoords,texelSize);
    //第二步，计算半影半径
    float penumbra = (projCoords.z - averBlocker) * weightOfLight / averBlocker;
    //第三步 PCF
    visibility = PCF(projCoords,int(penumbra));
    return visibility;
}



float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float shadow = 0;
    shadow = PCSS(projCoords);
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = texture(normalTexture, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2 -1);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}