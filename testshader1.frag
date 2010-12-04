//uniform variables
uniform sampler2D region1ColorMap;
uniform sampler2D region2ColorMap;
uniform sampler2D region3ColorMap;
uniform sampler2D region4ColorMap;
uniform float region1Max;
uniform float region2Max;
uniform float region3Max;
uniform float region4Max;
uniform float region1Min;
uniform float region2Min;
uniform float region3Min;
uniform float region4Min;

//varying variables
varying float intensity;
varying float height;
varying int isWater;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

//constants
const vec3 L = vec3(0.0, 0.0, 1.0); //light direction
const vec4 diff = vec4(0.0, 0.0, 1.0, 0.0); //diffuse material color
const vec4 spec = vec4(0.7, 0.7, 0.7, 0.0); //specular material color
const float spec_pow = 20.0; //specular exponent

void main(){
	//if it is water
	if(isWater == 1){
		//normalize the normal
		vec3 Nn = normalize(N);
		
		//get the diffuse coefficient
		float kd = max(0.0, dot(Nn, L));
		
		//get the incoming vector
		vec3 I = normalize(V.xyz - E.xyz);
		
		//get the half vector
		vec3 H = normalize(L - I);
		
		//get the specular coefficent
		float ks = dot(Nn, H);
		
		//get the material color
		vec4 mat_color = (kd * diff * intensity) + (max(pow(ks, spec_pow), 0.0) * spec);
		
		//get the reflected vector around the surface normal
		//vec3 R = reflect(I, Nn);
		
		//get the environment color
		//vec4 env_color = textureCube(CubeMap, R);
		
		//mix the two colors
		//gl_FragColor = mix(mat_color, env_color, F);
		gl_FragColor = mat_color;
	}
	
	//if not
	else{
		//get the colors
		vec4 color_1 = texture2D(region1ColorMap, gl_TexCoord[0].st);
		vec4 color_2 = texture2D(region2ColorMap, gl_TexCoord[0].st);
		vec4 color_3 = texture2D(region3ColorMap, gl_TexCoord[0].st);
		vec4 color_4 = texture2D(region4ColorMap, gl_TexCoord[0].st);
		
		//get the region weights
		float region1Range = region1Max - region1Min;
		float region2Range = region2Max - region2Min;
		float region3Range = region3Max - region3Min;
		float region4Range = region4Max - region4Min;
		
		float region1Weight = max(0.0, (region1Range - abs(height - region1Max)) / region1Range);
		float region2Weight = max(0.0, (region2Range - abs(height - region2Max)) / region2Range);
		float region3Weight = max(0.0, (region3Range - abs(height - region3Max)) / region3Range);
		float region4Weight = max(0.0, (region4Range - abs(height - region4Max)) / region4Range);
		
		vec4 totalColor = (color_1 * region1Weight) + (color_2 * region2Weight) + (color_3 * region3Weight) + (color_4 * region4Weight);
		
		gl_FragColor = totalColor * intensity;
	}
}