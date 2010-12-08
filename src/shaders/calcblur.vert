uniform float focalDistance, focalRange;
varying float blur;

void main()
{
	vec4 posWV = gl_ModelViewMatrix * gl_Vertex;

	blur = clamp(abs(-posWV.y - focalDistance) / focalRange, 0.0, 1.0);

	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
