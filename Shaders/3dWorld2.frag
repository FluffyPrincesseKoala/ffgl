#version 410 core
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

// camera position
uniform float camPosX;
uniform float camPosY;
uniform float camPosZ;

// elgonate vec3
uniform float elongateX;
uniform float elongateY;
uniform float elongateZ;

// pyramid size
uniform float pyramidSize;
uniform float basePyramidSize;
#define basePyramidSize basePyramidSize

// sphere position
uniform float spherePosX;
uniform float spherePosY;
uniform float spherePosZ;

// sphere size
uniform float sphereSize;
#define sphereSize sphereSize

// camera rotation
uniform float cameraX;
uniform float cameraY;

uniform float pyramid_fractal_offset_x;
uniform float pyramid_fractal_offset_y;
uniform float pyramid_fractal_offset_z;

#define camPos vec3(camPosX * 5.0 - 2.5, camPosY * 5.0 - 2.5, camPosZ * 5.0 - 2.5)
#define cameraRotation vec2(cameraX * 3.14159265358979323846 * 10.0, cameraY * 3.14159265358979323846 * 10.0)
#define fractal_x_offset pyramid_fractal_offset_x * 2.0 - 1.0
#define fractal_y_offset pyramid_fractal_offset_y * 2.0 - 1.0
#define fractal_z_offset pyramid_fractal_offset_z * 2.0 - 1.0
#define pyramidPos vec3(fractal_x_offset, fractal_y_offset, fractal_z_offset)
#define spherePos vec3(spherePosX * 5.0 - 2.5, spherePosY * 5.0 - 2.5, spherePosZ * 5.0 - 2.5)

in vec2 uv;

out vec4 fragColor;

// ok

mat2 rot2D(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

float smin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

float sdPyramid(vec3 p, float h) {
    float m2 = h * h + 0.25;

    p.xz = abs(p.xz);
    p.xz = (p.z > p.x) ? p.zx : p.xz;
    p.xz -= basePyramidSize;

    vec3 q = vec3(p.z, h * p.y - 0.5 * p.x, h * p.x + 0.5 * p.y);

    float s = max(-q.x, 0.0);
    float t = clamp((q.y - 0.5 * p.z) / (m2 + 0.25), 0.0, 1.0);

    float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
    float b = m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);

    float d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0 ? 0.0 : min(a, b);

    return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y));
}

float sdBoxFrame(vec3 p, vec3 b, float e) {
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;
    return min(min(length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0), length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)), length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

vec3 opRep(vec3 p, vec3 c) {
    return mod(p, c) - 0.5 * c;
}

vec3 opElongate(in vec3 p, in vec3 h) {
    vec3 q = p - clamp(p, -h, h);
    return q;
}

float map(vec3 p) {
	// copy of p
    vec3 pSave = vec3(p);
    float ground = p.y + 0.1;

    pSave = opElongate(pSave, vec3(elongateX, elongateY, elongateZ));

    // float pyramid = sdPyramid(pSave - pyramidPos, pyramidSize);

    float pyramids[7];

	// add more pyramid
    for(int i = 0; i < 7; i++) {
        vec3 pNext = vec3(pSave.x, pSave.y, pSave.z);

        if(i % 2 == 0) {
            pNext.x += float(i) * 0.5;
            pNext.x -= abs(sin(time * 0.4) * 3.14159265358979323846 * 0.80);
        } else {
            pNext.z += float(i) * 0.5;
            pNext.z -= abs(cos(time * 0.4) * 3.14159265358979323846 * 0.80);
        }

		// rotate the next pyramid
        pNext.xz *= rot2D(-sin(time * 0.4) * 3.14159265358979323846 * 2.0);

        float pyramidNext = sdPyramid(pNext - pyramidPos, pyramidSize);

        pyramids[i] = pyramidNext;
    }

    float pyramid = pyramids[0];
    for(int i = 1; i < 7; i++) {
        pyramid = smin(pyramid, pyramids[i], 0.1);
    }

    // return min(min(pyramid, ground), 0.0);
	// return pyramid;
    return smin(ground, pyramid, 0.1);
}

// color palette
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

void main() {
    // vec2 uv = (gl_FragCoord.xy * 2.0 - u_resolution.xy) / u_resolution.y;
    vec2 uvBase = uv * 2.0 - 1.0;

    vec3 ro = vec3(0.0, 0.0, -3.0) + camPos;

    vec3 rd = normalize(vec3(uvBase, 1));

    ro.yz *= rot2D(-cameraRotation.y);
    ro.xz *= rot2D(-cameraRotation.x);
    rd.yz *= rot2D(-cameraRotation.y);
    rd.xz *= rot2D(-cameraRotation.x);

    vec3 col = vec3(0);

    float t = 0.0;

    // ray marching
    for(int i = 0; i < 80; i++) {
        vec3 p = ro + rd * t;

        vec3 pSave = vec3(p);

        float d = map(p);

        // add a big red sphere
        float sphere = length(pSave - vec3(sphereSize) - spherePos) - 0.5;
        d = smin(d, sphere, 0.1);

        t += d;
        // col = vec3(i) / 80.0;
        if(d < 0.001)
            break;
        if(t > 100.0)
            break;
    }

    float tBright = t * 0.05;

    // col = vec3(tBright);

    vec3 color = palette(tBright, vec3(0.5), vec3(0.5), vec3(1.0), vec3(0.8, 0.7, 0.6));
    col = color;

    fragColor = vec4(col, 1);
}