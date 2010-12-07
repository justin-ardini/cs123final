//uniform variables
uniform samplerCube CubeMap;

uniform sampler2D bumpmap;
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
varying float isWater;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

void main(){
	//if it is water
	if(isWater == 1.0){
		//normalize the normal
		vec3 Nn = normalize(N);
		
		//get the incoming vector
		vec3 I = normalize(V.xyz - E.xyz);
		
		//get the reflected vector around the surface normal
		vec3 R = reflect(I, Nn);
		
		//get the refracted vector
		vec3 R2 = refract(I, Nn, 0.9);
		
		//get the environment color for the reflection
		vec4 env_color = textureCube(CubeMap, R);
		
		//get the environment color for the refraction
		vec4 env_color2 = textureCube(CubeMap, R2);
		
		//mix the two colors
		gl_FragColor = mix(mix(env_color, env_color2, 0.2), vec4(0.2, 0.2, 0.5, 1.0), 0.2) * intensity;
		//gl_FragColor = texture2D(bumpmap, gl_TexCoord[0].st) * intensity;
		
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