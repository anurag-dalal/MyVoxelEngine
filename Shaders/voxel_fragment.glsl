#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in uint BlockId;  // Added flat qualifier to match vertex shader
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D textureAtlas;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;

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
    float specularStrength = 0.3;  // Reduced for a more natural look on blocks
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(normalize(lightDir), norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);  // Reduced shininess
    vec3 specular = specularStrength * spec * lightColor;
    
    // Apply lighting
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    // Output final color with texture alpha
    FragColor = vec4(result, texColor.a);
}
