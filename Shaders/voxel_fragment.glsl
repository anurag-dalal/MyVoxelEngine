#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in uint BlockId;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D textureAtlas;
uniform float atlasSize;

// Lighting uniforms
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;

void main() {
    vec2 uv = TexCoord;
    vec4 texColor = texture(textureAtlas, uv);
    
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -normalize(lightDir)), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(normalize(lightDir), norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine lighting
    vec3 lighting = ambient + diffuse + specular;
    
    FragColor = vec4(lighting, 1.0) * texColor;
}
