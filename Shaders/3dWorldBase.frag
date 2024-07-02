#version 410 core
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

in vec2 uv;

out vec4 fragColor;

// ok

float map(vec3 p) {
    float d = length(p) - 1.0;
    return d;
}

void main() {
    // vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec2 uvBase = uv * 2.0 - 1.0;

    vec3 ro = vec3(0, 0, -3);
    vec3 rd = normalize(vec3(uvBase, 1));
    vec3 col = vec3(0);

    float t = 0.0;

    // ray marching
    for(int i = 0; i < 80; i++) {
        vec3 p = ro + rd * t;

        float d = map(p);

        t += d;
        // col = vec3(i) / 80.0;
        if(d < 0.001)
            break;
        if(t > 100.0)
            break;
    }

    col = vec3(t * 0.2);

    fragColor = vec4(col, 1);
}