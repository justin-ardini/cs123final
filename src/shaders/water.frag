//uniform variables
uniform sampler2D reflection;
uniform sampler2D refraction;
uniform sampler2D bumpMap;

uniform float offsetX;
uniform float offsetY;

//varying variables
varying float intensity;
varying float height;
varying float blur;
uniform float screenWidth;
uniform float screenHeight;

varying vec4 V; //vertex
varying vec4 E; //eye
varying vec3 N; //surface normal

const vec4 L = vec4(1.0, 1.0, 1.0, 0.0); //light direction

void main(){
    vec2 tempVec2 = gl_TexCoord[1].st + vec2(offsetX, offsetY);
    if(tempVec2.x > 1.0){
        tempVec2.x -= 1.0;
    }
    if(tempVec2.y > 1.0){
        tempVec2.y -= 1.0;
    }
    vec4 tempVec = texture2D(bumpMap, tempVec2);
    tempVec.w = 0.0;
    

    //normalize the normal
    vec3 Nn = normalize(N + (gl_NormalMatrix * tempVec.xyz));
    
    //get the incoming vector
    vec3 I = normalize(V.xyz - E.xyz);
    
    //get light intensity
    vec4 normalizedLight = normalize(gl_ModelViewMatrix * L);
    float intensity2 = dot(Nn, normalizedLight.xyz);
    
    float angle = 1.0 - max(0.0, dot(Nn, -I));
    vec3 d = V.xyz - E.xyz;
    float dist = (d.x * d.x) + (d.y * d.y) + (d.z * d.z);
    if(angle < 0.3){
        if(dist > 1.0){
            angle = angle * angle * angle * angle;
        }
        angle = angle * angle;
    }
    if(dist > 5.0){
        angle = angle * angle;
    }
    
    //get the normal in camera space
    vec4 camNorm = angle * 10.0 * (gl_ModelViewProjectionMatrix * tempVec);
    //vec4 camNorm = 10.0 * (gl_ModelViewProjectionMatrix * tempVec);

    vec2 reflectLocation = ((gl_FragCoord.xy - vec2(camNorm.x, 0.0)) / vec2(screenWidth, screenHeight));
    //vec2 reflectLocation = ((gl_FragCoord.xy) / vec2(screenWidth, screenHeight));
    reflectLocation.x = max(0.0, min(1.0, reflectLocation.x));
    
    // get the reflected vector around the surface normal
    //vec4 R = texture2D(reflection, gl_FragCoord.xy / vec2(screenWidth, screenHeight));
    vec4 R = texture2D(reflection, reflectLocation);
    
    //get the refracted vector
    vec3 R2 = refract(I, Nn, 0.9);
    
    //get the environment color for the refraction
    //vec4 env_color = textureCube(cubeMap, R2);

    // TESTING
    gl_FragColor = mix(texture2D(refraction, gl_FragCoord.xy / vec2(screenWidth, screenHeight)),
                    vec4(0.2, 0.2, 0.5, 1.0), 0.2) * intensity;

    //mix the reflection with the blue of the water
    //gl_FragColor = mix(R, vec4(0.2, 0.2, 0.5, 1.0), 0.2) * intensity;
    gl_FragColor = mix(R, vec4(0.2, 0.2, 0.5, 1.0), 0.2) * intensity2;
    gl_FragColor.a = R.a;

    gl_FragColor = texture2D(bumpMap, gl_TexCoord[1].st);
}
