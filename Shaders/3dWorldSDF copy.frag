uniform float sizeCylinder = 0.5;
uniform float sizeSphere = 1.0;
uniform float time = StandalonePlayableCueTransport;
uniform vec3 colors = Color;
uniform bool fractal = false;
uniform float distanceFractal = 1.0;
uniform float shapeAbsorption = 0.1;

float smin(float a, float b, float k) {
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

mat2 rot2D(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c, -s, s, c);
}

float sdfSphere(vec3 p, float r) {
    return length(p) - r;
}

float sdfInfinitCylinder(vec3 p, float r) {
    return length(p.xz) - r;
}

float map(vec3 p) {
    if (fractal == true) {
        p = mod(p, 2.0 * distanceFractal) - distanceFractal;
    }
    float d = sdfSphere(p, sizeSphere);
    d = smin(d, sdfInfinitCylinder(p, sizeCylinder), shapeAbsorption);
    // rotate new cylinder 90 degrees
    vec3 pRot = p;
    pRot.xy = rot2D(1.5708) * p.xy;
    d = smin(d, sdfInfinitCylinder(pRot, sizeCylinder), shapeAbsorption);
    return d;
}

vec4 main(in vec3 origin, in vec3 direction, in ivec2 px, out vec3 intersectionPoint) {
    vec3 ro = origin - vec3(0.0, 0.0, time);
    vec3 p = ro;
    for (int i = 0; i < 64; ++i) {
        float d = map(p);
        p += direction * d * 0.8;
    }
    vec2 eps = vec2(0.01, 0.0);
    vec3 n = normalize(vec3(map(p + eps.xyy) - map(p - eps.xyy), map(p + eps.yxy) - map(p - eps.yxy), map(p + eps.yyx) - map(p - eps.yyx)));

    vec3 color = (n * 0.5 + 0.5) * exp(-distance(p, ro) * 0.1) * colors;
    intersectionPoint = p;
    return vec4(color, 1.0);
}