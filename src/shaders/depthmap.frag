uniform sampler2D Tex0;

void main (void)
{
	vec4 tex = texture2D(Tex0, gl_TexCoord[0].st);

	gl_FragColor = vec4(vec3(tex.a), 1.0);
}
