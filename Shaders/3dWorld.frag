#version 410 core

uniform float colorPulse;
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
uniform vec2 resolution;
uniform float iterationsX;
uniform float iterationsY;
uniform float iterationsZ;
uniform float iterationsSphereX;
uniform float iterationsSphereY;
uniform float iterationsSphereZ;

uniform float speed;

uniform float fractalAmount;
uniform float fractalAmountSphere;

uniform float scaleSphere;
uniform float scaleBox;
uniform float sphereSize;
uniform float boxSize;
uniform float scaleSphereX;
uniform float scaleSphereY;
uniform float scaleSphereZ;
uniform float scaleBoxX;
uniform float scaleBoxY;
uniform float scaleBoxZ;

uniform float boxLeftRightCrossfade;
uniform float boxUpDownCrossfade;
uniform float sphereLeftRightCrossfade;
uniform float sphereUpDownCrossfade;

uniform float groundHeight;

uniform float positionXSphere;
uniform float positionYSphere;
uniform float positionZSphere;

uniform float positionXBox;
uniform float positionYBox;
uniform float positionZBox;

uniform float boxDisplacementX;
uniform float boxDisplacementY;
uniform float boxDisplacementZ;

uniform float fieldOfView;
uniform float cameraX;
uniform float cameraY;
uniform float brightness;

uniform float modulationRayX;
uniform float modulationRayY;
uniform float modulationRayZ;
uniform float rayModulationFactor;

in vec2 uv;

out vec4 fragColor;

//https://iquilezles.org/articles/palettes/ << blog post
// http://dev.thi.ng/gradients/ << palettes value generator
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

float sdSphere(vec3 p, float s) {
    return length(p) - s;
}

float sdBox(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float smin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

mat2 rot2D(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

float map(vec3 p, float posXsphere, float posYsphere, float posZsphere, float posXbox, float posYbox, float posZbox, float sphereSize, float boxSize, float groundHeight, float time, float boxLeftRightCrossfade, float boxUpDownCrossfade, float sphereLeftRightCrossfade, float sphereUpDownCrossfade, float modulationX, float modulationY, float modulationZ, float fractalAmountBox, float boxDisplacementX, float boxDisplacementY, float boxDisplacementZ, float modulationSphereX, float modulationSphereY, float modulationSphereZ, float fractalAmountSphere) {
    float left = time * 0.5;
    float right = -time * 0.5;
    float up = time * 0.5;
    float down = -time * 0.5;

    vec3 spherePos = vec3(posXsphere, posYsphere, posZsphere);
    vec3 sphereFinalPos = p - spherePos;
    // rotate sphere
    float lrResA = mix(left, right, sphereLeftRightCrossfade);
    sphereFinalPos.xy = rot2D(lrResA) * sphereFinalPos.xy;

    float udResA = mix(up, down, sphereUpDownCrossfade);
    sphereFinalPos.xz = rot2D(udResA) * sphereFinalPos.xz;

		// fractal sphere
	// sphereFinalPos = fract(sphereFinalPos) - 0.5;
	// vec3 sphereFractal = fract(sphereFinalPos * modulation) - 0.5;
    vec3 sphereFractal;
    sphereFractal.x = fract(sphereFinalPos.x * modulationSphereX) - 0.5;
    sphereFractal.y = fract(sphereFinalPos.y * modulationSphereY) - 0.5;
    sphereFractal.z = fract(sphereFinalPos.z * modulationSphereZ) - 0.5;

	// fractalAmount = 1.0;
    sphereFinalPos = mix(sphereFinalPos, sphereFractal, smoothstep(0.0, 1.0, fractalAmountSphere));

    vec3 sphereFinalFractal = sphereFinalPos;

    float sphere = sdSphere(sphereFinalFractal, sphereSize);

    vec3 boxPos = vec3(posXbox, posYbox, posZbox);
    vec3 boxFinalPos = p - boxPos;

    // rotate box
    float lrResB = mix(left, right, boxLeftRightCrossfade);
    boxFinalPos.xy = rot2D(lrResB) * boxFinalPos.xy;

    float udResB = mix(up, down, boxUpDownCrossfade);
    boxFinalPos.xz = rot2D(udResB) * boxFinalPos.xz;

    // box displacement
    boxFinalPos.x -= time * boxDisplacementX;
    boxFinalPos.y -= time * boxDisplacementY;
    boxFinalPos.z -= time * boxDisplacementZ;

	// fractal box
    // boxFinalPos = fract(boxFinalPos) - 0.5;
    // vec3 boxFractal = fract(boxFinalPos * modulation) - 0.5;
    vec3 boxFractal;
    boxFractal.x = fract(boxFinalPos.x * modulationX) - 0.5;
    boxFractal.y = fract(boxFinalPos.y * modulationY) - 0.5;
    boxFractal.z = fract(boxFinalPos.z * modulationZ) - 0.5;

    boxFinalPos = mix(boxFinalPos, boxFractal, smoothstep(0.0, 1.0, fractalAmountBox));

    float box = sdBox(boxFinalPos, vec3(boxSize));

    // ground plane
    float ground = p.y + groundHeight + 0.5;

    // ground rotation
    float c = cos(0.5 * time);
    float s = sin(0.5 * time);
    p.xz = mat2(c, -s, s, c) * p.xz;

    // return min(sphere, box);
    // return smin(sphere, box, 2.0);
    // return min(ground, smin(sphere, box, 2.0));
    return smin(ground, smin(sphere, box, 2.0), 1.0);
}

//https://www.shadertoy.com/view/mtyGWy
void main() {

		// clamp to 0-1
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

	// map position from -5.0 to 5.0
    float posX0 = positionXSphere * 10.0 - 5.0;
    float posY0 = positionYSphere * 10.0 - 5.0;
    float posZ0 = positionZSphere * 10.0 - 5.0;

    float posX1 = positionXBox * 10.0 - 5.0;
    float posY1 = positionYBox * 10.0 - 5.0;
    float posZ1 = positionZBox * 10.0 - 5.0;

    float GroundHeight = groundHeight * 10.0 - 5.0;

    float modulationX = iterationsX;
    float modulationY = iterationsY;
    float modulationZ = iterationsZ;
    float modulationSphereX = iterationsSphereX;
    float modulationSphereY = iterationsSphereY;
    float modulationSphereZ = iterationsSphereZ;
    float fractalAmount = fractalAmount * 10.0 - 5.0;
    float fractalAmountSphere = fractalAmountSphere * 10.0 - 5.0;

    // map camera
    float fov = fieldOfView * 3.0;

    // object scaling
    float fscaleSphere = scaleSphere * 10.0 - 5.0;
    float scaleSphereX = scaleSphereX * 10.0 - 5.0;
    float scaleSphereY = scaleSphereY * 10.0 - 5.0;
    float scaleSphereZ = scaleSphereZ * 10.0 - 5.0;
    float fscaleBox = scaleBox * 10.0 - 5.0;
    float scaleBoxX = scaleBoxX * 10.0 - 5.0;
    float scaleBoxY = scaleBoxY * 10.0 - 5.0;
    float scaleBoxZ = scaleBoxZ * 10.0 - 5.0;

    vec2 uvBase = uv * 2.0 - 1.0;
    vec2 camera = vec2(cameraX, cameraY) * 20.0 - 10.0;

    // raymarching initialization
    vec3 ro = vec3(0.0, 0.0, -3.0); // ray origin
    vec3 rd = normalize(vec3(uvBase * fov, 1.0)); // ray direction
    vec3 col = vec3(0.0); // color

    float t = 0.0; // total distance traveled

    ro.yz *= rot2D(-camera.y);
    ro.xz *= rot2D(-camera.x);
    rd.yz *= rot2D(-camera.y);
    rd.xz *= rot2D(-camera.x);

    // raymarching
    int i;
    for(i = 0; i < 64; i++) {
        vec3 p = ro + rd * t; // current position

		// raycontrole
        p.y += sin(t * modulationRayY * 5.0 - 2.5) * rayModulationFactor;
        p.x += sin(t * modulationRayX * 5.0 - 2.5) * rayModulationFactor;
        p.z += sin(t * modulationRayZ * 5.0 - 2.5) * rayModulationFactor;

        // distance to closest object
        float d = map(p, posX0, posY0, posZ0, posX1, posY1, posZ1, sphereSize, boxSize, GroundHeight, time, boxLeftRightCrossfade, boxUpDownCrossfade, sphereLeftRightCrossfade, sphereUpDownCrossfade, modulationX, modulationY, modulationZ, fractalAmount, scaleBoxX, scaleBoxY, scaleBoxZ, modulationSphereX, modulationSphereY, modulationSphereZ, fractalAmountSphere);

        t += d; // move along ray by distance to closest object

        // col = vec3(float(i)) / 64.0; // color based on iteration
        if(d < 0.001)
            break; // break if close enough to object
        if(t > 100.0)
            break; // break if too far away
    }

    // color debugging
    // col = vec3(t * brightness);
    col = palette(((t * (colorPulse * 20.0 - 10.0)) * brightness) + i * 0.005, vec3(pal0R, pal0G, pal0B), vec3(pal1R, pal1G, pal1B), vec3(pal2R, pal2G, pal2B), vec3(pal3R, pal3G, pal3B));

    // fragColor = vec4(uvBase, 0.0, 1.0);
    fragColor = vec4(col, 1.0);
}