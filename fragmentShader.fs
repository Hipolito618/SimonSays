#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 direction;
    float cutoff;
    float constant;
    float lineal;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform float brillo;

void main()
{
    // Factor de brillo para modular estado de reposo (0.45) vs encendido (2.4) vs chasis (1.0)
    float factorBrillo = (brillo > 0.0) ? brillo : 1.0;

    // Vector de dirección desde el fragmento hacia la posición de la luz
    vec3 lightDir = normalize(light.position - FragPos);

    // Coseno del ángulo theta entre el vector de luz y el cono del foco (-light.direction)
    float theta = dot(lightDir, normalize(-light.direction));

    // Luz ambiental base (presente dentro y fuera del cono)
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

    vec3 result = ambient;

    // Si estamos dentro del cono del foco (theta > cutoff)
    if (theta > light.cutoff)
    {
        // Componente difusa
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

        // Componente especular (Phong)
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

        // Cálculo de atenuación por distancia cuadrática
        float distance = length(light.position - FragPos);
        float atenuacion = 1.0 / (light.constant + light.lineal * distance + light.quadratic * (distance * distance));

        diffuse *= atenuacion;
        specular *= atenuacion;

        result += diffuse + specular;
    }

    // Modulación final del color resultante por el factor de brillo
    FragColor = vec4(result * factorBrillo, 1.0);
}
