#version 410 core
precision mediump float; // Declare the precision qualifier for the float type

in vec2 uv;
in vec2 resolution;
in float time;

out vec4 fragColor; // Declare the output variable for the fragment shader

void main() {
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord / resolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5 * cos(time + uv.xyx + vec3(0, 2, 4));

    // Output to screen
    gl_FragCoord = vec4(col, 1.0);
}

// #version 410 core
// uniform vec4 RGBALeft;
// uniform vec4 RGBARight;
// uniform float blur;
// uniform float time;
// uniform vec2 resolution;

// in vec2 uv;

// out vec4 fragColor;

// void main()
// {
// 	// use time to change the gradient
// 	float x = (sin(time) + 1.0) / 2.0;
// 	float x1 = mix(0., .5, blur);
// 	float x2 = mix(1., .5, blur);
// 	// float x = smoothstep(x1, x2, uv.x);
// 	fragColor = mix( RGBALeft, RGBARight, x );
// }