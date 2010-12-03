const int MAX_KERNEL_SIZE = 25;
uniform sampler2D tex;
uniform vec2 offsets[MAX_KERNEL_SIZE]; 
uniform float kernel[MAX_KERNEL_SIZE];
void main(void) { 
    vec4 final = vec4(0, 0, 0, 1.0);
    vec4 sample;
    for (int i = 0; i < MAX_KERNEL_SIZE; ++i) {
        sample = texture2D(tex, gl_TexCoord[0].st + offsets[i]);
        final.r += kernel[i] * sample.r;
        final.g += kernel[i] * sample.g;
        final.b += kernel[i] * sample.b;
    }
    gl_FragColor = final;
}
