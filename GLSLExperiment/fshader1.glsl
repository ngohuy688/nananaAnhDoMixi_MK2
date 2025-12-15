#version 400

in vec3 fNormal;
in vec3 fPosition;

out vec4 color;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

uniform bool enableLighting;

void main()
{
    if (!enableLighting)
    {
        color = DiffuseProduct;
        color.a = 1.0;
        return;
    }

    vec3 N = normalize(fNormal);
    vec3 L = normalize(LightPosition.xyz - fPosition);
    vec3 E = normalize(-fPosition);
    vec3 H = normalize(L + E);

    vec4 ambient = AmbientProduct;

    float Kd = max(dot(L, N), 0.0);
    vec4 diffuse = Kd * DiffuseProduct;

    float Ks = pow(max(dot(N, H), 0.0), Shininess);
    vec4 specular = Ks * SpecularProduct;

    if (dot(L, N) < 0.0)
        specular = vec4(0.0);

    color = ambient + diffuse + specular;
    color.a = 1.0;
}
