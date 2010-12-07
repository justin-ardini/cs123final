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
varying float blur;
uniform float focalDistance, focalRange;
varying int isWater;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

//constant
const vec4 L = vec4(1.0, 1.0, 1.0, 0.0); //light direction

void main(){
	//get the position
	vec4 vertCopy = gl_Vertex;
	
	//get the norm of the vertex
	vec3 vertexNorm = gl_NormalMatrix * gl_Normal;
	
	//get the tex coord
	gl_TexCoord[0] = gl_MultiTexCoord0;

	//check to see if the position is below the sea level
	if(vertCopy.z <= sea_level){
		vec4 bump = texture2D(bumpmap, gl_TexCoord[0].st);
		
		vertCopy.z = sea_level;
		vertexNorm = gl_NormalMatrix * (vec3(0.0, 0.0, 1.0) + bump.xyz);
		isWater = 1;
		
		V = gl_ModelViewMatrix * vertCopy;
		E = gl_ProjectionMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
		N = normalize(vertexNorm);
	}
	else{
		isWater = 0;
		
		V = vec4(0.0, 0.0, 0.0, 0.0);
		E = vec4(0.0, 0.0, 0.0, 0.0);
		N = vec3(0.0, 0.0, 0.0);
	}
	
	gl_Position = gl_ModelViewProjectionMatrix * vertCopy;
	
	vec4 posWV = gl_ModelViewMatrix * vertCopy;
    blur = clamp(abs(-posWV.z - focalDistance) / focalRange, 0.0, 1.0);
	
	vec3 normalizedNorm = normalize(vertexNorm);
	
	//get the light direction
	vec4 lightDirection = gl_ModelViewMatrix * L;
	vec4 normalizedLight = normalize(lightDirection);
	
	intensity = dot(normalizedNorm, normalizedLight);
	
	//get the height
	height = gl_Vertex.z;
	
}
