uniform vec4 color;
uniform bool u_invertColors;


void main() {
    if (u_invertColors) {
        colour = vec4(1.0 - colour.r, 1.0 - colour.g, 1.0 - colour.b, colour.a);
    }
    gl_FragColor = color;
}
