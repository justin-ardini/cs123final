//uniform variables
uniform samplerCube cubeMap;
uniform sampler2D bumpMap;

uniform float offsetX;
uniform float offsetY;

//varying variables
varying float intensity;
varying float height;
varying float blur;
varying float isWater;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

void main(){
    vec2 tempVec2 = gl_TexCoord[0].st + vec2(offsetX, offsetY);
    if(tempVec2.x > 1.0){
        tempVec2.x -= 1.0;
    }
    if(tempVec2.y > 1.0){
        tempVec2.y -= 1.0;
    }
    vec4 tempVec = texture2D(bumpMap, tempVec2);
    
    //normalize the normal
    vec3 Nn = normalize(N + (gl_NormalMatrix * tempVec.xyz));
    
    //get the incoming vector
    vec3 I = normalize(V.xyz - E.xyz);
    
    //get the reflected vector around the surface normal
    vec3 R = reflect(I, Nn);
    
    //get the refracted vector
    vec3 R2 = refract(I, Nn, 0.9);
    
    //get the environment color for the reflection
    vec4 env_color = textureCube(cubeMap, R);
    
    //get the environment color for the refraction
    vec4 env_color2 = textureCube(cubeMap, R2);
    
    //mix the two colors
    gl_FragColor = mix(mix(env_color, env_color2, 0.2), vec4(0.2, 0.2, 0.5, 1.0), 0.2) * intensity;
    gl_FragColor.a = blur;

    //gl_FragColor = texture2D(bumpMap, gl_TexCoord[0].st);
}
