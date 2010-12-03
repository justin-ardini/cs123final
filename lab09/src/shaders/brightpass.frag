uniform sampler2D tex;
const vec3 avgVector = vec3(0.299, 0.587, 0.114);
void main(void) {
    vec4 sample = texture2D(tex, gl_TexCoord[0].st);
    float luminance = max(0.0, dot(avgVector, sample.rgb));
    if (luminance < 1.8) {
        gl_FragColor = vec4(0, 0, 0, 1.0);
    } else {
        gl_FragColor = sample;
    }
}
