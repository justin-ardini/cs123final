//uniform variables
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
varying int isWater;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal


void main(){
	//get the position
	vec4 vertCopy = gl_Vertex;
	
	//get the norm of the vertex
	vec3 vertexNorm = gl_NormalMatrix * gl_Normal;
	
	//check to see if the position is below the sea level
	if(vertCopy.z <= sea_level){
		vec4 bump = texture2D(bumpmap, gl_TexCoord[0].st);
		
		vertCopy.z = sea_level;
		vertexNorm = gl_NormalMatrix * vec3(0.0, 0.0, 1.0);
		isWater = 1;
		
		V = gl_ModelViewMatrix * gl_Vertex;
		E = gl_ProjectionMatrixInverse * vec4(0.0, 0.0, -1.0, 0.0);
		N = normalize(vertexNorm);
	}
	else{
		isWater = 0;
		
		V = vec4(0.0, 0.0, 0.0, 0.0);
		E = vec4(0.0, 0.0, 0.0, 0.0);
		N = vec3(0.0, 0.0, 0.0);
	}
	
	gl_Position = gl_ModelViewProjectionMatrix * vertCopy;
	
	
	vec3 normalizedNorm = normalize(vertexNorm);
	
	//get the light direction
	vec4 lightDirection = gl_LightSource[0].position * (gl_ModelViewMatrix * gl_Vertex);
	vec4 normalizedLight = normalize(lightDirection);
	
	intensity = dot(normalizedNorm, normalizedLight);
	
	//get the height
	height = gl_Vertex.z;
	
	//get the tex coord
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
