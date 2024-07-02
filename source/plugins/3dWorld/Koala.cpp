#include "Koala.h"
#include <math.h>//floor
#include <fstream>
#include <string>
using namespace ffglex;

// FIXME get rid of this wonky log mechanism
std::ofstream logFile( "plugin_log.txt", std::ios::out | std::ios::app );

void InitializeLogging()
{
	// Open the log file
	logFile.open( "plugin_log.txt", std::ios::out | std::ios::app );
}
void Log( const std::string& message )
{
	if( logFile.is_open() )
	{
		logFile << message << std::endl;
	}
}
enum ParamType : FFUInt32
{
	// // koala sexy shaders
	PT_CAM_POS_X,
	PT_CAM_POS_Y,
	PT_CAM_POS_Z,

	//Rotation
	PT_CAM_ROTATION_X,
	PT_CAM_ROTATION_Y,

	// Sphere position
	PT_OBJ_A_POS_X,
	PT_OBJ_A_POS_Y,
	PT_OBJ_A_POS_Z,

	// object B position
	PT_OBJ_B_POS_X,
	PT_OBJ_B_POS_Y,
	PT_OBJ_B_POS_Z,

	// object C position
	PT_OBJ_C_POS_X,
	PT_OBJ_C_POS_Y,
	PT_OBJ_C_POS_Z,

	// // Sphere size
	// PT_SPHERE_SIZE,

	PID_OBJECT_OPTION,
	PID_OBJECT_OPTION_0,
	PID_OBJECT_OPTION_1,
};

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Koala >,                                                                          // Create method
	"KOA7",                                                                                          // Plugin unique ID
	"3D WORLD",                                                                                      // Plugin name
	2,                                                                                               // API major version number
	1,                                                                                               // API minor version number
	1,                                                                                               // Plugin major version number
	000,                                                                                             // Plugin minor version number
	FF_SOURCE,                                                                                       // Plugin type
	"raymarching, soft shadow, anti aliasing, 23 sdf 3d generator, sun and sky with light reflexion",// Plugin description
	"just did the very first step of the shader here"                                                // About
);

static const char vertexShaderCode[] = R"(#version 410 core
layout( location = 0 ) in vec4 vPosition;
layout( location = 1 ) in vec2 vUV;

out vec2 uv;

void main()
{
	gl_Position = vPosition;
	uv = vUV;
}
)";

static const std::string fragmentShaderCode = R"(#version 410 core
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

// object selection
uniform float selectionObjectA;
uniform float selectionObjectB;
uniform float selectionObjectC;

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
uniform float ObjAPosX;
uniform float ObjAPosY;
uniform float ObjAPosZ;

// object B position
uniform float ObjBPosX;
uniform float ObjBPosY;
uniform float ObjBPosZ;

// object C position
uniform float ObjCPosX;
uniform float ObjCPosY;
uniform float ObjCPosZ;

// sphere size
uniform float sphereSize;
#define sphereSize sphereSize

// camera rotation
uniform float cameraX;
uniform float cameraY;

uniform float pyramid_fractal_offset_x;
uniform float pyramid_fractal_offset_y;
uniform float pyramid_fractal_offset_z;

#define camPos vec3(camPosX * 20.0 - 10.0, camPosY * 20.0 - 10.0, camPosZ * 20.0 - 10.0)
#define cameraRotation vec2(cameraX * 3.14159265358979323846 * 10.0, cameraY * 3.14159265358979323846 * 10.0)
#define fractal_x_offset pyramid_fractal_offset_x * 2.0 - 1.0
#define fractal_y_offset pyramid_fractal_offset_y * 2.0 - 1.0
#define fractal_z_offset pyramid_fractal_offset_z * 2.0 - 1.0
#define pyramidPos vec3(fractal_x_offset, fractal_y_offset, fractal_z_offset)
#define ObjectAPos vec3(ObjAPosX * 20.0 - 10.0, ObjAPosY * 20.0 - 10.0, ObjAPosZ * 20.0 - 10.0)
#define ObjectBPos vec3(ObjBPosX * 20.0 - 10.0, ObjBPosY * 20.0 - 10.0, ObjBPosZ * 20.0 - 10.0)
#define ObjectCPos vec3(ObjCPosX * 20.0 - 10.0, ObjCPosY * 20.0 - 10.0, ObjCPosZ * 20.0 - 10.0)

in vec2 uv;

out vec4 fragColor;

#define AA 1   // make this 2 or 3 for antialiasing

float dot2(in vec2 v) {
    return dot(v, v);
}
float dot2(in vec3 v) {
    return dot(v, v);
}
float ndot(in vec2 a, in vec2 b) {
    return a.x * b.x - a.y * b.y;
}

float sdPlane(vec3 p) {
    return p.y;
}

float sdSphere(vec3 p, float s) {
    return length(p) - s;
}

float sdBox(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sdBoxFrame(vec3 p, vec3 b, float e) {
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;

    return min(min(length(max(vec3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0), length(max(vec3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)), length(max(vec3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}
float sdEllipsoid(in vec3 p, in vec3 r) // approximated
{
    float k0 = length(p / r);
    float k1 = length(p / (r * r));
    return k0 * (k0 - 1.0) / k1;
}

float sdTorus(vec3 p, vec2 t) {
    return length(vec2(length(p.xz) - t.x, p.y)) - t.y;
}

float sdCappedTorus(in vec3 p, in vec2 sc, in float ra, in float rb) {
    p.x = abs(p.x);
    float k = (sc.y * p.x > sc.x * p.y) ? dot(p.xy, sc) : length(p.xy);
    return sqrt(dot(p, p) + ra * ra - 2.0 * ra * k) - rb;
}

float sdHexPrism(vec3 p, vec2 h) {
    vec3 q = abs(p);

    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = abs(p);
    p.xy -= 2.0 * min(dot(k.xy, p.xy), 0.0) * k.xy;
    vec2 d = vec2(length(p.xy - vec2(clamp(p.x, -k.z * h.x, k.z * h.x), h.x)) * sign(p.y - h.x), p.z - h.y);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdOctogonPrism(in vec3 p, in float r, float h) {
    const vec3 k = vec3(-0.9238795325,   // sqrt(2+sqrt(2))/2 
    0.3826834323,   // sqrt(2-sqrt(2))/2
    0.4142135623); // sqrt(2)-1 
  // reflections
    p = abs(p);
    p.xy -= 2.0 * min(dot(vec2(k.x, k.y), p.xy), 0.0) * vec2(k.x, k.y);
    p.xy -= 2.0 * min(dot(vec2(-k.x, k.y), p.xy), 0.0) * vec2(-k.x, k.y);
  // polygon side
    p.xy -= vec2(clamp(p.x, -k.z * r, k.z * r), r);
    vec2 d = vec2(length(p.xy) * sign(p.y), p.z - h);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r) {
    vec3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

float sdRoundCone(in vec3 p, in float r1, float r2, float h) {
    vec2 q = vec2(length(p.xz), p.y);

    float b = (r1 - r2) / h;
    float a = sqrt(1.0 - b * b);
    float k = dot(q, vec2(-b, a));

    if (k < 0.0)
        return length(q) - r1;
    if (k > a * h)
        return length(q - vec2(0.0, h)) - r2;

    return dot(q, vec2(a, b)) - r1;
}

float sdRoundCone(vec3 p, vec3 a, vec3 b, float r1, float r2) {
    // sampling independent computations (only depend on shape)
    vec3 ba = b - a;
    float l2 = dot(ba, ba);
    float rr = r1 - r2;
    float a2 = l2 - rr * rr;
    float il2 = 1.0 / l2;

    // sampling dependant computations
    vec3 pa = p - a;
    float y = dot(pa, ba);
    float z = y - l2;
    float x2 = dot2(pa * l2 - ba * y);
    float y2 = y * y * l2;
    float z2 = z * z * l2;

    // single square root!
    float k = sign(rr) * rr * rr * x2;
    if (sign(z) * a2 * z2 > k)
        return sqrt(x2 + z2) * il2 - r2;
    if (sign(y) * a2 * y2 < k)
        return sqrt(x2 + y2) * il2 - r1;
    return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}

float sdTriPrism(vec3 p, vec2 h) {
    const float k = sqrt(3.0);
    h.x *= 0.5 * k;
    p.xy /= h.x;
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0)
        p.xy = vec2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x -= clamp(p.x, -2.0, 0.0);
    float d1 = length(p.xy) * sign(-p.y) * h.x;
    float d2 = abs(p.z) - h.y;
    return length(max(vec2(d1, d2), 0.0)) + min(max(d1, d2), 0.);
}

// vertical
float sdCylinder(vec3 p, vec2 h) {
    vec2 d = abs(vec2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

// arbitrary orientation
float sdCylinder(vec3 p, vec3 a, vec3 b, float r) {
    vec3 pa = p - a;
    vec3 ba = b - a;
    float baba = dot(ba, ba);
    float paba = dot(pa, ba);

    float x = length(pa * baba - ba * paba) - r * baba;
    float y = abs(paba - baba * 0.5) - baba * 0.5;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = (max(x, y) < 0.0) ? -min(x2, y2) : (((x > 0.0) ? x2 : 0.0) + ((y > 0.0) ? y2 : 0.0));
    return sign(d) * sqrt(abs(d)) / baba;
}

// vertical
float sdCone(in vec3 p, in vec2 c, float h) {
    vec2 q = h * vec2(c.x, -c.y) / c.y;
    vec2 w = vec2(length(p.xz), p.y);

    vec2 a = w - q * clamp(dot(w, q) / dot(q, q), 0.0, 1.0);
    vec2 b = w - q * vec2(clamp(w.x / q.x, 0.0, 1.0), 1.0);
    float k = sign(q.y);
    float d = min(dot(a, a), dot(b, b));
    float s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    return sqrt(d) * sign(s);
}

float sdCappedCone(in vec3 p, in float h, in float r1, in float r2) {
    vec2 q = vec2(length(p.xz), p.y);

    vec2 k1 = vec2(r2, h);
    vec2 k2 = vec2(r2 - r1, 2.0 * h);
    vec2 ca = vec2(q.x - min(q.x, (q.y < 0.0) ? r1 : r2), abs(q.y) - h);
    vec2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(k2), 0.0, 1.0);
    float s = (cb.x < 0.0 && ca.y < 0.0) ? -1.0 : 1.0;
    return s * sqrt(min(dot2(ca), dot2(cb)));
}

float sdCappedCone(vec3 p, vec3 a, vec3 b, float ra, float rb) {
    float rba = rb - ra;
    float baba = dot(b - a, b - a);
    float papa = dot(p - a, p - a);
    float paba = dot(p - a, b - a) / baba;

    float x = sqrt(papa - paba * paba * baba);

    float cax = max(0.0, x - ((paba < 0.5) ? ra : rb));
    float cay = abs(paba - 0.5) - 0.5;

    float k = rba * rba + baba;
    float f = clamp((rba * (x - ra) + paba * baba) / k, 0.0, 1.0);

    float cbx = x - ra - f * rba;
    float cby = paba - f;

    float s = (cbx < 0.0 && cay < 0.0) ? -1.0 : 1.0;

    return s * sqrt(min(cax * cax + cay * cay * baba, cbx * cbx + cby * cby * baba));
}

// c is the sin/cos of the desired cone angle
float sdSolidAngle(vec3 pos, vec2 c, float ra) {
    vec2 p = vec2(length(pos.xz), pos.y);
    float l = length(p) - ra;
    float m = length(p - c * clamp(dot(p, c), 0.0, ra));
    return max(l, m * sign(c.y * p.x - c.x * p.y));
}

float sdOctahedron(vec3 p, float s) {
    p = abs(p);
    float m = p.x + p.y + p.z - s;

    // exact distance
    #if 0
    vec3 o = min(3.0 * p - m, 0.0);
    o = max(6.0 * p - m * 2.0 - o * 3.0 + (o.x + o.y + o.z), 0.0);
    return length(p - s * o / (o.x + o.y + o.z));
    #endif

    // exact distance
    #if 1
    vec3 q;
    if (3.0 * p.x < m)
        q = p.xyz;
    else if (3.0 * p.y < m)
        q = p.yzx;
    else if (3.0 * p.z < m)
        q = p.zxy;
    else
        return m * 0.57735027;
    float k = clamp(0.5 * (q.z - q.y + s), 0.0, s);
    return length(vec3(q.x, q.y - s + k, q.z - k)); 
    #endif

    // bound, not exact
    #if 0
    return m * 0.57735027;
    #endif
}

float sdPyramid(in vec3 p, in float h) {
    float m2 = h * h + 0.25;

    // symmetry
    p.xz = abs(p.xz);
    p.xz = (p.z > p.x) ? p.zx : p.xz;
    p.xz -= 0.5;

    // project into face plane (2D)
    vec3 q = vec3(p.z, h * p.y - 0.5 * p.x, h * p.x + 0.5 * p.y);

    float s = max(-q.x, 0.0);
    float t = clamp((q.y - 0.5 * p.z) / (m2 + 0.25), 0.0, 1.0);

    float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
    float b = m2 * (q.x + 0.5 * t) * (q.x + 0.5 * t) + (q.y - m2 * t) * (q.y - m2 * t);

    float d2 = min(q.y, -q.x * m2 - q.y * 0.5) > 0.0 ? 0.0 : min(a, b);

    // recover 3D and scale, and add sign
    return sqrt((d2 + q.z * q.z) / m2) * sign(max(q.z, -p.y));
    ;
}

// la,lb=semi axis, h=height, ra=corner
float sdRhombus(vec3 p, float la, float lb, float h, float ra) {
    p = abs(p);
    vec2 b = vec2(la, lb);
    float f = clamp((ndot(b, b - 2.0 * p.xz)) / dot(b, b), -1.0, 1.0);
    vec2 q = vec2(length(p.xz - 0.5 * b * vec2(1.0 - f, 1.0 + f)) * sign(p.x * b.y + p.z * b.x - b.x * b.y) - ra, p.y - h);
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0));
}

float sdHorseshoe(in vec3 p, in vec2 c, in float r, in float le, vec2 w) {
    p.x = abs(p.x);
    float l = length(p.xy);
    p.xy = mat2(-c.x, c.y, c.y, c.x) * p.xy;
    p.xy = vec2((p.y > 0.0 || p.x > 0.0) ? p.x : l * sign(-c.x), (p.x > 0.0) ? p.y : l);
    p.xy = vec2(p.x, abs(p.y - r)) - vec2(le, 0.0);

    vec2 q = vec2(length(max(p.xy, 0.0)) + min(0.0, max(p.x, p.y)), p.z);
    vec2 d = abs(q) - w;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdU(in vec3 p, in float r, in float le, vec2 w) {
    p.x = (p.y > 0.0) ? abs(p.x) : length(p.xy);
    p.x = abs(p.x - r);
    p.y = p.y - le;
    float k = max(p.x, p.y);
    vec2 q = vec2((k < 0.0) ? -k : length(max(p.xy, 0.0)), abs(p.z)) - w;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0);
}


vec2 opU(vec2 d1, vec2 d2) {
    return (d1.x < d2.x) ? d1 : d2;
}


#define ZERO (min(0,0))

)"
											  // TODO find a better way to link the shaders together (make a shader class that generates the code ? or add a file input for the shader to resolum if ffgl can handle it)
											  R"(
vec2 map(in vec3 pos) {
    vec2 res = vec2(pos.y, 0.0);

    if (selectionObjectA == 0.0) {
        res = opU(res, vec2(sdSphere(pos - vec3(-2.0, 0.25, 0.0), 0.25), 26.9));
    }
    if (selectionObjectA == 1.0) {
        res = opU(res, vec2(sdRhombus((pos - vec3(-2.0, 0.25, 1.0)).xzy, 0.15, 0.25, 0.04, 0.08), 17.0));
    }

    if (selectionObjectA == 2.0) {
        res = opU(res, vec2(sdCappedTorus((pos - vec3(0.0, 0.30, 1.0)) * vec3(1, -1, 1), vec2(0.866025, -0.5), 0.25, 0.05), 25.0));
    }

    if (selectionObjectA == 3.0) {
        res = opU(res, vec2(sdBoxFrame(pos - vec3(0.0, 0.25, 0.0), vec3(0.3, 0.25, 0.2), 0.025), 16.9));
    }

    if (selectionObjectA == 4.0) {
        res = opU(res, vec2(sdCone(pos - vec3(0.0, 0.45, -1.0), vec2(0.6, 0.8), 0.45), 55.0));
    }

    if (selectionObjectA == 5.0) {
        res = opU(res, vec2(sdCappedCone(pos - vec3(0.0, 0.25, -2.0), 0.25, 0.25, 0.1), 13.67));

    }
    if (selectionObjectA == 6.0) {
        res = opU(res, vec2(sdSolidAngle(pos - vec3(0.0, 0.00, -3.0), vec2(3, 4) / 5.0, 0.4), 49.13));
    }

    if (selectionObjectA == 7.0) {
        res = opU(res, vec2(sdTorus((pos - vec3(1.0, 0.30, 1.0)).xzy, vec2(0.25, 0.05)), 7.1));
    }
        // res = opU(res, vec2(sdBox(pos - vec3(1.0, 0.25, 0.0), vec3(0.3, 0.25, 0.1)), 3.0));
    if (selectionObjectB == 8.0) {
        res = opU(res, vec2(sdSphere(pos - vec3(1.0, 0.25, 0.0), 0.25), 26.9));
    }
        // res = opU(res, vec2(sdCapsule(pos - vec3(1.0, 0.00, -1.0), vec3(-0.1, 0.1, -0.1), vec3(0.2, 0.4, 0.2), 0.1), 31.9));
    if (selectionObjectB == 9.0) {
        res = opU(res, vec2(sdCappedCone(pos - vec3(1.0, 0.25, -2.0), 0.25, 0.25, 0.1), 13.67));
    }
        // res = opU(res, vec2(sdCylinder(pos - vec3(1.0, 0.25, -2.0), vec2(0.15, 0.25)), 8.0));
    if (selectionObjectB == 10.0) {
        res = opU(res, vec2(sdCylinder(pos - ObjectBPos, vec3(0.15, 0.25, 0.0), vec3(-0.2, 0.35, 0.1), 0.08), 31.2));
    }
    if (selectionObjectB == 11.0) {
        res = opU(res, vec2(sdHexPrism(pos - ObjectBPos, vec2(0.2, 0.05)), 18.4));
    }

    if (selectionObjectB == 12.0) {
        res = opU(res, vec2(sdPyramid(pos - ObjectBPos, 1.0), 13.56));
    }
    if (selectionObjectB == 13.0) {
        res = opU(res, vec2(sdOctahedron(pos - ObjectBPos, 0.35), 23.56));
    }
        // res = opU(res, vec2(sdTriPrism(pos - ObjectBPos, vec2(0.3, 0.05)), 43.5));
    if (selectionObjectB == 14.0) {
        res = opU(res, vec2(sdTriPrism(pos - ObjectBPos, vec2(0.3, 0.05)), 43.5));
    }
    if (selectionObjectB == 15.0) {
        res = opU(res, vec2(sdEllipsoid(pos - ObjectBPos, vec3(0.2, 0.25, 0.05)), 43.17));
    }

    return res;
}

// https://iquilezles.org/articles/boxfunctions
vec2 iBox(in vec3 ro, in vec3 rd, in vec3 rad) {
    vec3 m = 1.0 / rd;
    vec3 n = m * ro;
    vec3 k = abs(m) * rad;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    return vec2(max(max(t1.x, t1.y), t1.z), min(min(t2.x, t2.y), t2.z));
}

vec2 raycast(in vec3 ro, in vec3 rd) {
    vec2 res = vec2(-1.0, -1.0);

    float tmin = 1.0;
    float tmax = 20.0;

    // raytrace floor plane
    float tp1 = (0.0 - ro.y) / rd.y;
    if (tp1 > 0.0) {
        tmax = min(tmax, tp1);
        res = vec2(tp1, 1.0);
    }
    //else return res;

    // raymarch primitives   
    vec2 tb = iBox(ro - vec3(0.0, 0.4, -0.5), rd, vec3(2.5, 0.41, 3.0));
    if (tb.x < tb.y && tb.y > 0.0 && tb.x < tmax) {
        //return vec2(tb.x,2.0);
        tmin = max(tb.x, tmin);
        tmax = min(tb.y, tmax);

        float t = tmin;
        for (int i = 0; i < 70 && t < tmax; i++) {
            vec2 h = map(ro + rd * t);
            if (abs(h.x) < (0.0001 * t)) {
                res = vec2(t, h.y);
                break;
            }
            t += h.x;
        }
    }

    return res;
}

// https://iquilezles.org/articles/rmshadows
float calcSoftshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax) {
    // bounding volume
    float tp = (0.8 - ro.y) / rd.y;
    if (tp > 0.0)
        tmax = min(tmax, tp);

    float res = 1.0;
    float t = mint;
    for (int i = ZERO; i < 24; i++) {
        float h = map(ro + rd * t).x;
        float s = clamp(8.0 * h / t, 0.0, 1.0);
        res = min(res, s);
        t += clamp(h, 0.01, 0.2);
        if (res < 0.004 || t > tmax)
            break;
    }
    res = clamp(res, 0.0, 1.0);
    return res * res * (3.0 - 2.0 * res);
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal(in vec3 pos) {
#if 0
    vec2 e = vec2(1.0, -1.0) * 0.5773 * 0.0005;
    return normalize(e.xyy * map(pos + e.xyy).x +
        e.yyx * map(pos + e.yyx).x +
        e.yxy * map(pos + e.yxy).x +
        e.xxx * map(pos + e.xxx).x);
#else
    // inspired by tdhooper and klems - a way to prevent the compiler from inlining map() 4 times
    vec3 n = vec3(0.0);
    for (int i = ZERO; i < 4; i++) {
        vec3 e = 0.5773 * (2.0 * vec3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0);
        n += e * map(pos + 0.0005 * e).x;
      //if( n.x+n.y+n.z>100.0 ) break;
    }
    return normalize(n);
#endif    
}

// https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
float calcAO(in vec3 pos, in vec3 nor) {
    float occ = 0.0;
    float sca = 1.0;
    for (int i = ZERO; i < 5; i++) {
        float h = 0.01 + 0.12 * float(i) / 4.0;
        float d = map(pos + h * nor).x;
        occ += (h - d) * sca;
        sca *= 0.95;
        if (occ > 0.35)
            break;
    }
    return clamp(1.0 - 3.0 * occ, 0.0, 1.0) * (0.5 + 0.5 * nor.y);
}

// https://iquilezles.org/articles/checkerfiltering
float checkersGradBox(in vec2 p, in vec2 dpdx, in vec2 dpdy) {
    // filter kernel
    vec2 w = abs(dpdx) + abs(dpdy) + 0.001;
    // analytical integral (box filter)
    vec2 i = 2.0 * (abs(fract((p - 0.5 * w) * 0.5) - 0.5) - abs(fract((p + 0.5 * w) * 0.5) - 0.5)) / w;
    // xor pattern
    return 0.5 - 0.5 * i.x * i.y;
}

vec3 render(in vec3 ro, in vec3 rd, in vec3 rdx, in vec3 rdy) { 
    // background
    vec3 col = vec3(0.7, 0.7, 0.9) - max(rd.y, 0.0) * 0.3;

    // raycast scene
    vec2 res = raycast(ro, rd);
    float t = res.x;
    float m = res.y;
    if (m > -0.5) {
        vec3 pos = ro + t * rd;
        vec3 nor = (m < 1.5) ? vec3(0.0, 1.0, 0.0) : calcNormal(pos);
        vec3 ref = reflect(rd, nor);

        // material        
        col = 0.2 + 0.2 * sin(m * 2.0 + vec3(0.0, 1.0, 2.0));
        float ks = 1.0;

        if (m < 1.5) {
            // project pixel footprint into the plane
            vec3 dpdx = ro.y * (rd / rd.y - rdx / rdx.y);
            vec3 dpdy = ro.y * (rd / rd.y - rdy / rdy.y);

            float f = checkersGradBox(3.0 * pos.xz, 3.0 * dpdx.xz, 3.0 * dpdy.xz);
            col = 0.15 + f * vec3(0.05);
            ks = 0.4;
        }

        // lighting
        float occ = calcAO(pos, nor);

        vec3 lin = vec3(0.0);

        // sun
        {
            vec3 lig = normalize(vec3(-0.5, 0.4, -0.6));
            vec3 hal = normalize(lig - rd);
            float dif = clamp(dot(nor, lig), 0.0, 1.0);
          //if( dif>0.0001 )
            dif *= calcSoftshadow(pos, lig, 0.02, 2.5);
            float spe = pow(clamp(dot(nor, hal), 0.0, 1.0), 16.0);
            spe *= dif;
            spe *= 0.04 + 0.96 * pow(clamp(1.0 - dot(hal, lig), 0.0, 1.0), 5.0);
                //spe *= 0.04+0.96*pow(clamp(1.0-sqrt(0.5*(1.0-dot(rd,lig))),0.0,1.0),5.0);
            lin += col * 2.20 * dif * vec3(1.30, 1.00, 0.70);
            lin += 5.00 * spe * vec3(1.30, 1.00, 0.70) * ks;
        }
        // sky
        {
            float dif = sqrt(clamp(0.5 + 0.5 * nor.y, 0.0, 1.0));
            dif *= occ;
            float spe = smoothstep(-0.2, 0.2, ref.y);
            spe *= dif;
            spe *= 0.04 + 0.96 * pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 5.0);
          //if( spe>0.001 )
            spe *= calcSoftshadow(pos, ref, 0.02, 2.5);
            lin += col * 0.60 * dif * vec3(0.40, 0.60, 1.15);
            lin += 2.00 * spe * vec3(0.40, 0.60, 1.30) * ks;
        }
        // back
        {
            float dif = clamp(dot(nor, normalize(vec3(0.5, 0.0, 0.6))), 0.0, 1.0) * clamp(1.0 - pos.y, 0.0, 1.0);
            dif *= occ;
            lin += col * 0.55 * dif * vec3(0.25, 0.25, 0.25);
        }
        // sss
        {
            float dif = pow(clamp(1.0 + dot(nor, rd), 0.0, 1.0), 2.0);
            dif *= occ;
            lin += col * 0.25 * dif * vec3(1.00, 1.00, 1.00);
        }

        col = lin;

        col = mix(col, vec3(0.7, 0.7, 0.9), 1.0 - exp(-0.0001 * t * t * t));
    }

    return vec3(clamp(col, 0.0, 1.0));
}

mat3 setCamera(in vec3 ro, in vec3 ta, float cr) {
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(sin(cr), cos(cr), 0.0);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = (cross(cu, cw));
    return mat3(cu, cv, cw);
}

void main() {
    vec2 mo = camPos.xy / resolution.xy;
    float time = 32.0 + time * 1.5;

    // camera	
    vec3 ta = vec3(0.25, -0.75, -0.75);
    vec3 ro = ta + vec3(4.5 * cos(0.1 + 7.0 * mo.x), mo.y, 4.5 * sin(0.1 + 7.0 * mo.x));
    // camera-to-world transformation
    mat3 ca = setCamera(ro, ta, 0.0);

    vec3 tot = vec3(0.0);
#if AA>1
    for (int m = ZERO; m < AA; m++) for (int n = ZERO; n < AA; n++) {
        // pixel coordinates
            vec2 o = vec2(float(m), float(n)) / float(AA) - 0.5;
            vec2 p = (2.0 * (uv + o) - resolution.xy) / resolution.y;
#else    
            vec2 p = (2.0 * uv - resolution.xy) / resolution.y;
#endif

        // focal length
            const float fl = 2.5;

        // ray direction
            vec3 rd = ca * normalize(vec3(p, fl));

         // ray differentials
            vec2 px = (2.0 * (uv + vec2(1.0, 0.0)) - resolution.xy) / resolution.y;
            vec2 py = (2.0 * (uv + vec2(0.0, 1.0)) - resolution.xy) / resolution.y;
            vec3 rdx = ca * normalize(vec3(px, fl));
            vec3 rdy = ca * normalize(vec3(py, fl));

        // render	
            vec3 col = render(ro, rd, rdx, rdy);

        // gain
        // col = col*3.0/(2.5+col);

		// gamma
            col = pow(col, vec3(0.4545));

            tot += col;
#if AA>1
        }
    tot /= float(AA * AA);
#endif

    fragColor = vec4(tot, 1.0);
}
)";

void CloseLogging()
{
	if( logFile.is_open() )
	{
		logFile.close();
	}
}

Koala::~Koala()
{
	CloseLogging();
}

Koala::Koala() :
	m_TimeLocation( -1 ),
	m_ResolutionLocation( -1 ),

	// // camera position location
	m_CamXLocation( -1 ),
	m_CamYLocation( -1 ),
	m_CamZLocation( -1 ),

	// // camera position values
	m_CamX( 0.5f ),
	m_CamY( 0.5f ),
	m_CamZ( 0.5f ),

	// // camera rotation location
	m_CamRotXLocation( -1 ),
	m_CamRotYLocation( -1 ),

	// // camera rotation values
	m_CamRotX( 0.5f ),
	m_CamRotY( 0.5f ),

	// objA position location
	m_ObjAXLocation( -1 ),
	m_ObjAYLocation( -1 ),
	m_ObjAZLocation( -1 ),

	// objA position values
	m_ObjAX( 0.5f ),
	m_ObjAY( 0.5f ),
	m_ObjAZ( 0.5f ),

	// objB position location
	m_ObjBXLocation( -1 ),
	m_ObjBYLocation( -1 ),
	m_ObjBZLocation( -1 ),

	// objB position values
	m_ObjBX( 0.5f ),
	m_ObjBY( 0.5f ),
	m_ObjBZ( 0.5f ),

	// objC position location
	m_ObjCXLocation( -1 ),
	m_ObjCYLocation( -1 ),
	m_ObjCZLocation( -1 ),

	// objC position values
	m_ObjCX( 0.5f ),
	m_ObjCY( 0.5f ),
	m_ObjCZ( 0.5f ),

	// object option
	m_selectionObjectALocation( -1 ),
	m_selectionObjectA( 0.0f ),
	m_selectionObjectBLocation( -1 ),
	m_selectionObjectB( 0.0f ),
	m_selectionObjectCLocation( -1 ),
	m_selectionObjectC( 0.0f )

{
	InitializeLogging();
	// Input properties
	SetMinInputs( 0 );
	SetMaxInputs( 0 );

	// Parameters
	SetParamInfof( PT_CAM_POS_X, "Cam Pos X", FF_TYPE_STANDARD );
	SetParamInfof( PT_CAM_POS_Y, "Cam Pos Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_CAM_POS_Z, "Cam Pos Z", FF_TYPE_STANDARD );

	SetParamInfof( PT_CAM_ROTATION_X, "Cam Rotation X", FF_TYPE_STANDARD );
	SetParamInfof( PT_CAM_ROTATION_Y, "Cam Rotation Y", FF_TYPE_STANDARD );

	SetParamInfof( PT_OBJ_A_POS_X, "ObjA Pos X", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_A_POS_Y, "ObjA Pos Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_A_POS_Z, "ObjA Pos Z", FF_TYPE_STANDARD );

	SetParamInfof( PT_OBJ_B_POS_X, "ObjB Pos X", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_B_POS_Y, "ObjB Pos Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_B_POS_Z, "ObjB Pos Z", FF_TYPE_STANDARD );

	SetParamInfof( PT_OBJ_C_POS_X, "ObjC Pos X", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_C_POS_Y, "ObjC Pos Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_OBJ_C_POS_Z, "ObjC Pos Z", FF_TYPE_STANDARD );

	// Set defaults object_option
	SetOptionParamInfo( PID_OBJECT_OPTION, "object 1", 8, FF_TYPE_OPTION );
	SetParamElementInfo( PID_OBJECT_OPTION, 0, "Sphere", 0 );
	SetParamElementInfo( PID_OBJECT_OPTION, 1, "Rhombus", 1 );
	SetParamElementInfo( PID_OBJECT_OPTION, 2, "CappedTorus", 2 );
	SetParamElementInfo( PID_OBJECT_OPTION, 3, "BoxFrame", 3 );
	SetParamElementInfo( PID_OBJECT_OPTION, 4, "Cone", 4 );
	SetParamElementInfo( PID_OBJECT_OPTION, 5, "CappedCone", 5 );
	SetParamElementInfo( PID_OBJECT_OPTION, 6, "SolidAngle", 6 );
	SetParamElementInfo( PID_OBJECT_OPTION, 7, "Torus", 7 );

	SetOptionParamInfo( PID_OBJECT_OPTION_0, "object 2", 8, FF_TYPE_OPTION );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 0, "Sphere", 8 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 1, "CappedCone", 9 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 2, "Cylinder", 10 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 3, "HexPrism", 11 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 4, "Pyramid", 12 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 5, "Octahedron", 13 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 6, "TriPrism", 14 );
	SetParamElementInfo( PID_OBJECT_OPTION_0, 7, "Ellipsoid", 15 );

	SetOptionParamInfo( PID_OBJECT_OPTION_1, "object 3", 6, FF_TYPE_OPTION );
	SetParamElementInfo( PID_OBJECT_OPTION_1, 0, "sdHorseshoe", 16 );
	SetParamElementInfo( PID_OBJECT_OPTION_1, 1, "sdOctogonPrism", 17 );
	SetParamElementInfo( PID_OBJECT_OPTION_1, 2, "sdCylinder", 18 );
	SetParamElementInfo( PID_OBJECT_OPTION_1, 3, "sdCappedCone", 19 );
	SetParamElementInfo( PID_OBJECT_OPTION_1, 4, "sdRoundCone", 20 );

	FFGLLog::LogToHost( "Created Gradient generator" );
}
FFResult Koala::InitGL( const FFGLViewportStruct* vp )
{
	if( !shader.Compile( vertexShaderCode, fragmentShaderCode ) )
	{
		DeInitGL();
		return FF_FAIL;
	}
	if( !quad.Initialise() )
	{
		DeInitGL();
		return FF_FAIL;
	}

	//FFGL requires us to leave the context in a default state on return, so use this scoped binding to help us do that.
	ScopedShaderBinding shaderBinding( shader.GetGLID() );

	m_TimeLocation = shader.FindUniform( "time" );
	// m_TimeDeltaLocation  = shader.FindUniform( "timeDelta" );
	m_ResolutionLocation = shader.FindUniform( "resolution" );

	m_CamXLocation = shader.FindUniform( "camPosX" );
	m_CamYLocation = shader.FindUniform( "camPosY" );
	m_CamZLocation = shader.FindUniform( "camPosZ" );

	m_CamRotXLocation = shader.FindUniform( "cameraX" );
	m_CamRotYLocation = shader.FindUniform( "cameraY" );

	// m_PyramidSizeLocation     = shader.FindUniform( "pyramidSize" );
	// m_PyramidSizeBaseLocation = shader.FindUniform( "basePyramidSize" );

	// m_ElongationXLocation = shader.FindUniform( "elongateX" );
	// m_ElongationYLocation = shader.FindUniform( "elongateY" );
	// m_ElongationZLocation = shader.FindUniform( "elongateZ" );

	// m_pyramid_fractal_offset_x_location = shader.FindUniform( "pyramid_fractal_offset_x" );
	// m_pyramid_fractal_offset_y_location = shader.FindUniform( "pyramid_fractal_offset_y" );
	// m_pyramid_fractal_offset_z_location = shader.FindUniform( "pyramid_fractal_offset_z" );

	m_ObjAXLocation = shader.FindUniform( "ObjAPosX" );
	m_ObjAYLocation = shader.FindUniform( "ObjAPosY" );
	m_ObjAZLocation = shader.FindUniform( "ObjAPosZ" );

	m_ObjBXLocation = shader.FindUniform( "ObjBPosX" );
	m_ObjBYLocation = shader.FindUniform( "ObjBPosY" );
	m_ObjBZLocation = shader.FindUniform( "ObjBPosZ" );

	m_ObjCXLocation = shader.FindUniform( "ObjCPosX" );
	m_ObjCYLocation = shader.FindUniform( "ObjCPosY" );
	m_ObjCZLocation = shader.FindUniform( "ObjCPosZ" );

	m_selectionObjectALocation = shader.FindUniform( "selectionObjectA" );
	m_selectionObjectBLocation = shader.FindUniform( "selectionObjectB" );

	// m_SphereSizeLocation = shader.FindUniform( "sphereSize" );

	//Use base-class init as success result so that it retains the viewport.
	return CFFGLPlugin::InitGL( vp );
}
float GetTimeInSeconds()
{
	return (float)clock() / CLOCKS_PER_SEC;
}
FFResult Koala::ProcessOpenGL( ProcessOpenGLStruct* pGL )
{
	//FFGL requires us to leave the context in a default state on return, so use this scoped binding to help us do that.
	ScopedShaderBinding shaderBinding( shader.GetGLID() );

	//enable the shader
	if( m_TimeLocation != -1 )
	{
		glUniform1f( m_TimeLocation, GetTimeInSeconds() );
	}
	// calculate iFrame
	// float iFrame = floor( GetTimeInSeconds() * 60.0f );
	// if( m_TimeDeltaLocation != -1 )
	// {
	// 	glUniform1f( m_TimeDeltaLocation, iFrame );
	// }
	if( m_ResolutionLocation != -1 )
	{
		//
		if( pGL->numInputTextures < 1 || pGL->inputTextures[ 0 ] == NULL )
		{
			glUniform2f( m_ResolutionLocation, 1.0f, 1.0f );
		}
		else
		{
			glUniform2f( m_ResolutionLocation, (float)pGL->inputTextures[ 0 ]->Width, (float)pGL->inputTextures[ 0 ]->Height );
		}
	}

	// camera position
	if( m_CamXLocation != -1 )
	{
		glUniform1f( m_CamXLocation, m_CamX );
	}
	if( m_CamYLocation != -1 )
	{
		glUniform1f( m_CamYLocation, m_CamY );
	}
	if( m_CamZLocation != -1 )
	{
		glUniform1f( m_CamZLocation, m_CamZ );
	}

	// camera rotation
	if( m_CamRotXLocation != -1 )
	{
		glUniform1f( m_CamRotXLocation, m_CamRotX );
	}
	if( m_CamRotYLocation != -1 )
	{
		glUniform1f( m_CamRotYLocation, m_CamRotY );
	}

	// m_ObjAXLocation position
	if( m_ObjAXLocation != -1 )
	{
		glUniform1f( m_ObjAXLocation, m_ObjAX );
	}
	if( m_ObjAYLocation != -1 )
	{
		glUniform1f( m_ObjAYLocation, m_ObjAY );
	}
	if( m_ObjAZLocation != -1 )
	{
		glUniform1f( m_ObjAZLocation, m_ObjAZ );
	}

	// objB position
	if( m_ObjBXLocation != -1 )
	{
		glUniform1f( m_ObjBXLocation, m_ObjBX );
	}
	if( m_ObjBYLocation != -1 )
	{
		glUniform1f( m_ObjBYLocation, m_ObjBY );
	}
	if( m_ObjBZLocation != -1 )
	{
		glUniform1f( m_ObjBZLocation, m_ObjBZ );
	}

	// objC position
	if( m_ObjCXLocation != -1 )
	{
		glUniform1f( m_ObjCXLocation, m_ObjCX );
	}
	if( m_ObjCYLocation != -1 )
	{
		glUniform1f( m_ObjCYLocation, m_ObjCY );
	}
	if( m_ObjCZLocation != -1 )
	{
		glUniform1f( m_ObjCZLocation, m_ObjCZ );
	}

	// object option
	if( m_selectionObjectALocation != -1 )
	{
		glUniform1f( m_selectionObjectALocation, m_selectionObjectA );
	}
	if( m_selectionObjectBLocation != -1 )
	{
		glUniform1f( m_selectionObjectBLocation, m_selectionObjectB );
	}
	if( m_selectionObjectCLocation != -1 )
	{
		glUniform1f( m_selectionObjectCLocation, m_selectionObjectC );
	}

	quad.Draw();

	return FF_SUCCESS;
}
FFResult Koala::DeInitGL()
{
	shader.FreeGLResources();
	quad.Release();

	return FF_SUCCESS;
}

FFResult Koala::SetFloatParameter( unsigned int dwIndex, float value )
{
	switch( dwIndex )
	{
	case PT_CAM_POS_X:
		m_CamX = value;
		break;
	case PT_CAM_POS_Y:
		m_CamY = value;
		break;
	case PT_CAM_POS_Z:
		m_CamZ = value;
		break;

	case PT_CAM_ROTATION_X:
		m_CamRotX = value;
		break;
	case PT_CAM_ROTATION_Y:
		m_CamRotY = value;
		break;

	case PID_OBJECT_OPTION:
		m_selectionObjectA = value;
		Log( "m_selectionObjectA: %f", m_selectionObjectA );
		break;
	case PID_OBJECT_OPTION_0:
		m_selectionObjectB = value;
		break;
	case PID_OBJECT_OPTION_1:
		m_selectionObjectC = value;
		break;

	case PT_OBJ_A_POS_X:
		m_ObjAX = value;
		break;
	case PT_OBJ_A_POS_Y:
		m_ObjAY = value;
		break;
	case PT_OBJ_A_POS_Z:
		m_ObjAZ = value;
		break;

	case PT_OBJ_B_POS_X:
		m_ObjBX = value;
		break;
	case PT_OBJ_B_POS_Y:
		m_ObjBY = value;
		break;
	case PT_OBJ_B_POS_Z:
		m_ObjBZ = value;
		break;

	case PT_OBJ_C_POS_X:
		m_ObjCX = value;
		break;
	case PT_OBJ_C_POS_Y:
		m_ObjCY = value;
		break;
	case PT_OBJ_C_POS_Z:
		m_ObjCZ = value;
		break;

	default:
		return FF_FAIL;
	}

	return FF_SUCCESS;
}

float Koala::GetFloatParameter( unsigned int index )
{
	switch( index )
	{
	case PT_CAM_POS_X:
		return m_CamX;
	case PT_CAM_POS_Y:
		return m_CamY;
	case PT_CAM_POS_Z:
		return m_CamZ;

	case PID_OBJECT_OPTION:
		return m_selectionObjectA;
	case PID_OBJECT_OPTION_0:
		return m_selectionObjectB;
	case PID_OBJECT_OPTION_1:
		return m_selectionObjectC;

	case PT_CAM_ROTATION_X:
		return m_CamRotX;
	case PT_CAM_ROTATION_Y:
		return m_CamRotY;

	case PT_OBJ_A_POS_X:
		return m_ObjAX;
	case PT_OBJ_A_POS_Y:
		return m_ObjAY;
	case PT_OBJ_A_POS_Z:
		return m_ObjAZ;

	case PT_OBJ_B_POS_X:
		return m_ObjBX;
	case PT_OBJ_B_POS_Y:
		return m_ObjBY;
	case PT_OBJ_B_POS_Z:
		return m_ObjBZ;

	case PT_OBJ_C_POS_X:
		return m_ObjCX;
	case PT_OBJ_C_POS_Y:
		return m_ObjCY;
	case PT_OBJ_C_POS_Z:
		return m_ObjCZ;
	}

	return 0.0f;
}
