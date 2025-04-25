
uniform sampler2D diffuse;
uniform int invert_colors;
varying vec2 texCoordVar;

void main() {
    vec4 texColor = texture2D(diffuse, texCoordVar);
    if (invert_colors == 1) {
	gl_FragColor = vec4(1.0 - texColor.r, 1.0 - texColor.g, 1.0 - texColor.b, texColor.a);
    }
    else {
        gl_FragColor = texture2D(diffuse, texCoordVar);
    }
}
