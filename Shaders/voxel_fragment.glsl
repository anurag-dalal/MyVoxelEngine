#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in uint BlockId;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;

uniform sampler2D textureAtlas;
uniform sampler2D shadowMap;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is beyond shadow map bounds
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0 ||
       projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0; // No shadow outside bounds
    }
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get current depth
    float currentDepth = projCoords.z;
    
    // Calculate bias based on surface angle relative to light
    float bias = max(0.025 * (1.0 - dot(Normal, -normalize(lightDir))), 0.0025);
    
    // PCF (Percentage Closer Filtering) with larger kernel for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int pcfRadius = 2; // Larger radius for softer shadows
    
    for(int x = -pcfRadius; x <= pcfRadius; ++x) {
        for(int y = -pcfRadius; y <= pcfRadius; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    
    shadow /= ((2 * pcfRadius + 1) * (2 * pcfRadius + 1)); // Average over kernel size
    
    // Keep the shadow at 0.0 when outside the light's far plane
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main() {
    // Sample texture based on UV coordinates
    vec4 texColor = texture(textureAtlas, TexCoord);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -normalize(lightDir)), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(normalize(lightDir), norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    // Apply lighting with shadows
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}
