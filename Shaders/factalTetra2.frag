#version 410 core

uniform float PALETTE_0_RED;
uniform float PALETTE_0_GREEN;
uniform float PALETTE_0_BLUE;
uniform float PALETTE_1_RED;
uniform float PALETTE_1_GREEN;
uniform float PALETTE_1_BLUE;
uniform float PALETTE_2_RED;
uniform float PALETTE_2_GREEN;
uniform float PALETTE_2_BLUE;
uniform float PALETTE_3_RED;
uniform float PALETTE_3_GREEN;
uniform float PALETTE_3_BLUE;

uniform float time;
uniform bool reverseTime;
uniform vec2 resolution;
uniform float iterations;

uniform float uvOffsetX;
uniform float uvOffsetY;

uniform float lineThickness;

uniform float shape;
uniform float distortion;
uniform float displacement;

uniform float fractalAmount;
uniform float colorPulse;

in vec2 uv;

out vec4 fragColor;

//https://iquilezles.org/articles/palettes/ << blog post
// http://dev.thi.ng/gradients/ << palettes value generator
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

float sdEquilateralTriangle(in vec2 p, in float r) {
    const float k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r / k;
    if(p.x + k * p.y > 0.0)
        p = vec2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x -= clamp(p.x, -2.0 * r, 0.0);
    return -length(p) * sign(p.y);
}

//https://www.shadertoy.com/view/mtyGWy
void main() {

    float pal0R = clamp(PALETTE_0_RED, 0.0, 1.0);
    float pal0G = clamp(PALETTE_0_GREEN, 0.0, 1.0);
    float pal0B = clamp(PALETTE_0_BLUE, 0.0, 1.0);
    float pal1R = clamp(PALETTE_1_RED, 0.0, 1.0);
    float pal1G = clamp(PALETTE_1_GREEN, 0.0, 1.0);
    float pal1B = clamp(PALETTE_1_BLUE, 0.0, 1.0);
    float pal2R = clamp(PALETTE_2_RED, 0.0, 1.0);
    float pal2G = clamp(PALETTE_2_GREEN, 0.0, 1.0);
    float pal2B = clamp(PALETTE_2_BLUE, 0.0, 1.0);
    float pal3R = clamp(PALETTE_3_RED, 0.0, 1.0);
    float pal3G = clamp(PALETTE_3_GREEN, 0.0, 1.0);
    float pal3B = clamp(PALETTE_3_BLUE, 0.0, 1.0);

    vec3 palettes[4] = vec3[4](vec3(pal0R, pal0G, pal0B), vec3(pal1R, pal1G, pal1B), vec3(pal2R, pal2G, pal2B), vec3(pal3R, pal3G, pal3B));

    vec2 uvBase = uv * 2.0 - 1.0;
    vec2 uv0 = uvBase;
    vec3 finalColor = vec3(0.0);

    float size = lineThickness;
    if(reverseTime) {
        size -= time * distortion;
    } else {
        size += time * distortion;
    }
    float iter = iterations * 8.0;

    float fAmount = clamp(fractalAmount, 0.0, 4.0);

	// TODO add uv offset and scale
    uvBase.x += clamp(uvOffsetX, -1.0, 1.0);
    uvBase.y += clamp(uvOffsetY, -1.0, 1.0);

    for(float i = 0.0; i < iter; i++) {
		// i -= 0.5;
        uvBase = fract(uvBase * fAmount) - 0.5;

        float triangleSize;
        if(reverseTime) {
            triangleSize = size + displacement * i;
        } else {
            triangleSize = size - displacement * i;
        }
        float d = sdEquilateralTriangle(uvBase, triangleSize);
        d *= exp(-sdEquilateralTriangle(uv0, size * shape * 2.0));

        float t = time * colorPulse;
        float td; // time delta direction
        if(reverseTime) {
            td = length(uv0) - t * 0.4;
        } else {
            td = length(uv0) + i + t * 0.4;
        }
        vec3 col = palette(td, palettes[0], palettes[1], palettes[2], palettes[3]);
        d = abs(d);

        d = sin(d * 8.) / 8.;

        d = abs(d);

        // d = (lineThickness * 0.01) / d;
        d = pow((lineThickness * 0.01) / d, 1.2);

    // col *= d;

        finalColor += col * d;
    }

    fragColor = vec4(finalColor, 1.0);
}