//uniform variables
uniform samplerCube CubeMap;

uniform float sea_level;
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
<<<<<<< HEAD:src/shaders/testshader1.vert
varying float blur;
uniform float focalDistance, focalRange;
varying int isWater;
=======
varying float isWater;
>>>>>>> 2b7591c3a38bb604b91566f909d42216a4a1a729:src/shaders/testshader1.vert

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

//constant
const vec4 L = vec4(1.0, 1.0, 1.0, 0.0); //light direction

void main(){
	//get the tex coord
	gl_TexCoord[0] = gl_MultiTexCoord0;	

	//get the position
	vec4 vertCopy = gl_Vertex;
	
	//get the norm of the vertex
	vec3 vertexNorm = gl_NormalMatrix * gl_Normal;
	
	//get the tex coord
	gl_TexCoord[0] = gl_MultiTexCoord0;

	//check to see if the position is below the sea level
	if(vertCopy.z <= sea_level){
		//vec4 bump = texture2D(bumpmap, gl_TexCoord[0].st);
		vec4 bump = texture2D(region2ColorMap, gl_TexCoord[0].st);
		
		vertCopy.z = sea_level;
		vertexNorm = gl_NormalMatrix * (normalize(vec3(0.0, 0.0, 1.0) + bump.xyz));
		isWater = 1.0;
		
		V = gl_ModelViewMatrix * vertCopy;
		E = gl_ProjectionMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
		N = normalize(vertexNorm);
	}
	else{
		isWater = 0.0;
		
		V = vec4(0.0, 0.0, 0.0, 0.0);
		E = vec4(0.0, 0.0, 0.0, 0.0);
		N = vec3(0.0, 0.0, 0.0);
	}
	
	gl_Position = gl_ModelViewProjectionMatrix * vertCopy;
	
    blur = clamp(abs(-gl_Position.z - focalDistance) / focalRange, 0.0, 1.0);
	
	vec3 normalizedNorm = normalize(vertexNorm);
	
	//get the light direction
	vec4 lightDirection = gl_ModelViewMatrix * L;
	vec4 normalizedLight = normalize(lightDirection);
	
	intensity = dot(normalizedNorm, normalizedLight.xyz);
	
	//get the height
	height = gl_Vertex.z;
<<<<<<< HEAD:src/shaders/testshader1.vert
	
=======
>>>>>>> 2b7591c3a38bb604b91566f909d42216a4a1a729:src/shaders/testshader1.vert
}
