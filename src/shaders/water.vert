//uniform variables
uniform sampler2D reflection;

uniform float offsetX;
uniform float offsetY;


//varying variables
varying float intensity;
varying float height;
varying float blur;
uniform float focalDistance, focalRange;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

//constant
const vec4 L = vec4(1.0, 1.0, 1.0, 0.0); //light direction

void main(){
        // normal map tex coord
        gl_TexCoord[1] = gl_MultiTexCoord1;

	
	//get the norm of the vertex
	vec3 vertexNorm = gl_NormalMatrix * gl_Normal;

	V = gl_ModelViewMatrix * gl_Vertex;
	E = gl_ProjectionMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
	N = normalize(vertexNorm);

        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	blur = clamp(abs(-gl_Position.z - focalDistance) / focalRange, 0.0, 1.0);
	
	vec3 normalizedNorm = normalize(vertexNorm);
	
	//get the light direction
	vec4 lightDirection = gl_ModelViewMatrix * L;
	vec4 normalizedLight = normalize(lightDirection);
	
	intensity = dot(normalizedNorm, normalizedLight.xyz);
	
	//get the height
	height = gl_Vertex.z;
}
