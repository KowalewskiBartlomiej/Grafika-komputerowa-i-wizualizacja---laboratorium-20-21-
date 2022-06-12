#version 330

#define NUMBER_OF_LIGHTS 5

uniform vec3 light_pos[NUMBER_OF_LIGHTS];
uniform vec3 camera_pos;
uniform vec3 light_color[NUMBER_OF_LIGHTS];

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

//Zmienne interpolowane
in vec3 pixel_position;
in vec3 pixel_normal;
in vec2 itexcoord;

uniform sampler2D texture_diffuse1;

vec3 add_lights(vec3 light_pos, vec3 light_color, vec3 normal, vec3 pixel_pos, vec3 view_direction)
{
	float distance = length(light_pos - pixel_pos); //Obliczamy odleg³oœæ Ÿród³a œwiat³a od ka¿dego wierzcho³ka
	float attenuation = 1.0 / (1.0 + 0.022	 * distance + 0.0019 * (distance * distance)); //Os³abienie œwiat³a wraz ze wzrostem odleg³oœci 
	

	float ambient_s = 0.03; // sila oswietlenia od otoczenia
	vec3 ambient = ambient_s * light_color; //jak intesnywne oswieltenie

	vec3 light_direction = normalize(light_pos - pixel_position); //zormalizowany wektor pomiêdzy pozycj¹ œwiat³a a pikselem (kierunek œwiat³a) 

	float diff = max(dot(normal, light_direction), 0); //Si³a œwiat³a na dany wierzcho³ek
	vec3 diffuse = diff * light_color * vec3(texture(texture_diffuse1, itexcoord)); //Si³a kolorowania (kolor danego wierzcho³ka)

	float specular_s = 0.05; //Si³a odbicia œwiat³a przez dany wierzcho³ek

	//Model oœwietlenia Blinn'a - Phonga
	vec3 half_way = normalize(light_direction + view_direction); //wektor po³owiczny pomiêdzy wektorem padania œwiat³a a wektorem obserwatora
	float spec = pow(max(dot(pixel_normal, half_way), 0), 16); //si³a odbicia œwiat³a
	
	vec3 specular = specular_s * spec * light_color; //obliczenie koloru odbicia

	//Wziêcie pod uwagê os³abienia
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	return (ambient + diffuse + specular); //po³¹czone sk³adowe œwiat³a
}


void main(void) 
{
		vec3 norm = normalize(pixel_normal); //znormalizowany wektor normalny wierzcho³ka
		vec3 view_direction = normalize(camera_pos - pixel_position); //znormalizowany wektor od obserwatora do wierzcho³ka
		vec3 result = vec3(0.0, 0.0, 0.0);
		//Dodawanie wp³ywu kolejnych Ÿróde³ œwiat³a na dany model
		for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
		{
			result += add_lights(light_pos[i], light_color[i], norm, pixel_position, view_direction);
		}
		pixelColor = vec4(result, 1.0);
}
