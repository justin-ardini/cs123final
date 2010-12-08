varying float blur;
uniform sampler2D tex0;

void main()
{
	gl_FragData[0] = vec4(texture2D(tex0, gl_TexCoord[0].st).rgb, blur);
}
