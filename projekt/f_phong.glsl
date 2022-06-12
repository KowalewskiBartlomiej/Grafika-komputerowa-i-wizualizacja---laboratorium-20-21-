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
	float distance = length(light_pos - pixel_pos); //Obliczamy odleg�o�� �r�d�a �wiat�a od ka�dego wierzcho�ka
	float attenuation = 1.0 / (1.0 + 0.022	 * distance + 0.0019 * (distance * distance)); //Os�abienie �wiat�a wraz ze wzrostem odleg�o�ci 
	

	float ambient_s = 0.03; // sila oswietlenia od otoczenia
	vec3 ambient = ambient_s * light_color; //jak intesnywne oswieltenie

	vec3 light_direction = normalize(light_pos - pixel_position); //zormalizowany wektor pomi�dzy pozycj� �wiat�a a pikselem (kierunek �wiat�a) 

	float diff = max(dot(normal, light_direction), 0); //Si�a �wiat�a na dany wierzcho�ek
	vec3 diffuse = diff * light_color * vec3(texture(texture_diffuse1, itexcoord)); //Si�a kolorowania (kolor danego wierzcho�ka)

	float specular_s = 0.05; //Si�a odbicia �wiat�a przez dany wierzcho�ek

	//Model o�wietlenia Blinn'a - Phonga
	vec3 half_way = normalize(light_direction + view_direction); //wektor po�owiczny pomi�dzy wektorem padania �wiat�a a wektorem obserwatora
	float spec = pow(max(dot(pixel_normal, half_way), 0), 16); //si�a odbicia �wiat�a
	
	vec3 specular = specular_s * spec * light_color; //obliczenie koloru odbicia

	//Wzi�cie pod uwag� os�abienia
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	
	return (ambient + diffuse + specular); //po��czone sk�adowe �wiat�a
}


void main(void) 
{
		vec3 norm = normalize(pixel_normal); //znormalizowany wektor normalny wierzcho�ka
		vec3 view_direction = normalize(camera_pos - pixel_position); //znormalizowany wektor od obserwatora do wierzcho�ka
		vec3 result = vec3(0.0, 0.0, 0.0);
		//Dodawanie wp�ywu kolejnych �r�de� �wiat�a na dany model
		for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
		{
			result += add_lights(light_pos[i], light_color[i], norm, pixel_position, view_direction);
		}
		pixelColor = vec4(result, 1.0);
}
