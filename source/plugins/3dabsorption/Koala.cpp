#include "Koala.h"
#include <math.h>//floor
using namespace ffglex;

enum ParamType : FFUInt32
{
	// koala sexy shaders
	PT_BOXLEFTRIGHTCROSSFADE,
	PT_BOXSIZE,
	PT_BOXUPDOWNCROSSFADE,
	PT_BRIGHTNESS,
	PT_CAMERA_X,
	PT_CAMERA_Y,
	PT_COLORPULSE,
	PT_FIELDOFVIEW,
	PT_FRACTALAMOUNT,
	PT_FRACTALAMOUNTSPHERE,
	PT_GROUNDABSORPTION,
	PT_GROUNDHEIGHT,
	PT_ITERATIONSX,
	PT_ITERATIONSXSPHERE,
	PT_ITERATIONSY,
	PT_ITERATIONSYSPHERE,
	PT_ITERATIONSZ,
	PT_ITERATIONSZSPHERE,
	PT_LINETHICKNESS,
	PT_MODULATIONRAYX,
	PT_MODULATIONRAYY,
	PT_MODULATIONRAYZ,
	PT_PALETTE_0_BLUE,
	PT_PALETTE_0_GREEN,
	PT_PALETTE_0_RED,
	PT_PALETTE_1_BLUE,
	PT_PALETTE_1_GREEN,
	PT_PALETTE_1_RED,
	PT_PALETTE_2_BLUE,
	PT_PALETTE_2_GREEN,
	PT_PALETTE_2_RED,
	PT_PALETTE_3_BLUE,
	PT_PALETTE_3_GREEN,
	PT_PALETTE_3_RED,
	PT_POSITION_X_Box,
	PT_POSITION_X,
	PT_POSITION_Y_Box,
	PT_POSITION_Y,
	PT_POSITION_Z_Box,
	PT_POSITION_Z,
	PT_RAYMODULATIONFACTOR,
	PT_REVERSE_TIME,
	PT_SCALEBOX,
	PT_SCALEBOXX,
	PT_SCALEBOXY,
	PT_SCALEBOXZ,
	PT_SCALESPHERE,
	PT_SCALESPHEREX,
	PT_SCALESPHEREY,
	PT_SCALESPHEREZ,
	PT_SPEED,
	PT_SPHEREBOXABSORPTION,
	PT_SPHERELEFTRIGHTCROSSFADE,
	PT_SPHERESIZE,
	PT_SPHEREUPDOWNCROSSFADE
};

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Koala >,               // Create method
	"KOA9",                               // Plugin unique ID
	"3D Absorption",                      // Plugin name
	2,                                    // API major version number
	1,                                    // API minor version number
	1,                                    // Plugin major version number
	000,                                  // Plugin minor version number
	FF_SOURCE,                            // Plugin type
	"3D fractal using minimal absorption",// Plugin description
	"Resolume FFGL plug by wonky koala"   // About
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

static const char fragmentShaderCode[] = R"(
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

uniform float sphereBoxAbsorption;
uniform float groundAbsorption;

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

float map(vec3 p, float posXsphere, float posYsphere, float posZsphere, float posXbox, float posYbox, float posZbox, float sphereSize, float boxSize, float groundHeight, float time, float boxLeftRightCrossfade, float boxUpDownCrossfade, float sphereLeftRightCrossfade, float sphereUpDownCrossfade, float modulationX, float modulationY, float modulationZ, float fractalAmountBox, float boxDisplacementX, float boxDisplacementY, float boxDisplacementZ, float modulationSphereX, float modulationSphereY, float modulationSphereZ, float fractalAmountSphere, float sphereBoxAbsorption, float groundAbsorption) {
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
    return smin(ground, smin(sphere, box, sphereBoxAbsorption), groundAbsorption);
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
    float fractalAmount = fractalAmount;
    float fractalAmountSphere = fractalAmountSphere;

    // map camera
    float fov = fieldOfView * 3.0;

    // object scaling
    float fscaleSphere = scaleSphere;
    float scaleSphereX = scaleSphereX * 10.0 - 5.0;
    float scaleSphereY = scaleSphereY * 10.0 - 5.0;
    float scaleSphereZ = scaleSphereZ * 10.0 - 5.0;
    float fscaleBox = scaleBox;
    float scaleBoxX = scaleBoxX * 10.0 - 5.0;
    float scaleBoxY = scaleBoxY * 10.0 - 5.0;
    float scaleBoxZ = scaleBoxZ * 10.0 - 5.0;

	// object size
	float sphereSize = sphereSize * .10;
	float boxSize = boxSize * .10;

	float sphereBoxAbsorption = sphereBoxAbsorption * 10.0;
	float groundAbsorption = groundAbsorption * 10.0;

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
        float d = map(p, posX0, posY0, posZ0, posX1, posY1, posZ1, sphereSize, boxSize, GroundHeight, time, boxLeftRightCrossfade, boxUpDownCrossfade, sphereLeftRightCrossfade, sphereUpDownCrossfade, modulationX, modulationY, modulationZ, fractalAmount, scaleBoxX, scaleBoxY, scaleBoxZ, modulationSphereX, modulationSphereY, modulationSphereZ, fractalAmountSphere, sphereBoxAbsorption, groundAbsorption);

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
)";

Koala::Koala() :
	iterationsX( 0.5f ),
	iterationsXLocation( -1 ),
	iterationsY( 0.5f ),
	iterationsYLocation( -1 ),
	iterationsZ( 0.5f ),
	iterationsZLocation( -1 ),
	fractalAmountLocation( -1 ),
	fractalAmount( 1.5f ),

	fractalAmountSphere( 1.5f ),
	fractalAmountSphereLocation( -1 ),
	iterationsSphereX( 0.5f ),
	iterationsSphereXLocation( -1 ),
	iterationsSphereY( 0.5f ),
	iterationsSphereYLocation( -1 ),
	iterationsSphereZ( 0.5f ),
	iterationsSphereZLocation( -1 ),

	speedLocation( -1 ),
	colorPulseLocation( -1 ),
	colorPulse( 0.5f ),
	speed( 0.4f ),

	palette0RedLocation( -1 ),
	palette0Red( 0.5f ),
	palette0GreenLocation( -1 ),
	palette0Green( 0.5f ),
	palette0BlueLocation( -1 ),
	palette0Blue( 0.5f ),

	palette1RedLocation( -1 ),
	palette1Red( 0.5f ),
	palette1GreenLocation( -1 ),
	palette1Green( 0.5f ),
	palette1BlueLocation( -1 ),
	palette1Blue( 0.5f ),

	palette2RedLocation( -1 ),
	palette2Red( 1.0f ),
	palette2GreenLocation( -1 ),
	palette2Green( 1.0f ),
	palette2BlueLocation( -1 ),
	palette2Blue( 1.0f ),

	palette3RedLocation( -1 ),
	palette3Red( 0.263f ),
	palette3GreenLocation( -1 ),
	palette3Green( 0.416f ),
	palette3BlueLocation( -1 ),
	palette3Blue( 0.557f ),

	lineThicknessLocation( -1 ),
	lineThickness( 0.5f ),
	reverseTimeLocation( -1 ),
	reverseTime( false ),

	positionXLocation( -1 ),
	positionX( 0.5f ),
	positionYLocation( -1 ),
	positionY( 0.5f ),
	positionZLocation( -1 ),
	positionZ( 0.5f ),

	fieldOfViewLocation( -1 ),
	fieldOfView( 0.5f ),

	positionXboxLocation( -1 ),
	positionXbox( 0.5f ),
	positionYboxLocation( -1 ),
	positionYbox( 0.5f ),
	positionZboxLocation( -1 ),
	positionZbox( 0.5f ),

	scaleSphereLocation( -1 ),
	scaleSphere( 0.02f ),

	scaleBoxLocation( -1 ),
	scaleBox( 0.015f ),

	groundHeightLocation( -1 ),
	groundHeight( 0.5f ),

	scaleSphereXLocation( -1 ),
	scaleSphereX( 0.5f ),
	scaleSphereYLocation( -1 ),
	scaleSphereY( 0.5f ),
	scaleSphereZLocation( -1 ),
	scaleSphereZ( 0.5f ),

	sizeBoxLocation( -1 ),
	boxSize( 0.2f ),
	sizeSphereLocation( -1 ),
	sphereSize( 0.1f ),

	scaleBoxXLocation( -1 ),
	scaleBoxX( 0.5f ),
	scaleBoxYLocation( -1 ),
	scaleBoxY( 0.5f ),
	scaleBoxZLocation( -1 ),
	scaleBoxZ( 0.5f ),

	boxLeftRightCrossfadeLocation( -1 ),
	boxLeftRightCrossfade( 0.5f ),
	boxUpDownCrossfadeLocation( -1 ),
	boxUpDownCrossfade( 0.5f ),

	sphereLeftRightCrossfadeLocation( -1 ),
	sphereLeftRightCrossfade( 0.5f ),
	sphereUpDownCrossfadeLocation( -1 ),
	sphereUpDownCrossfade( 0.5f ),

	cameraXLocation( -1 ),
	cameraX( 0.5f ),
	cameraYLocation( -1 ),
	cameraY( 0.5f ),
	brightnessLocation( -1 ),
	brightness( 0.05f ),

	modulationRayXLocation( -1 ),
	modulationRayX( 0.0f ),
	modulationRayYLocation( -1 ),
	modulationRayY( 0.0f ),
	modulationRayZLocation( -1 ),
	modulationRayZ( 0.0f ),
	rayModulationFactorLocation( -1 ),
	rayModulationFactor( 0.5f ),

	// sphereBoxAbsorption, groundAbsorption
	sphereBoxAbsorptionLocation( -1 ),
	sphereBoxAbsorption( 0.2f ),
	groundAbsorptionLocation( -1 ),
	groundAbsorption( 0.005f )
{
	// Input properties
	SetMinInputs( 0 );
	SetMaxInputs( 0 );

	// position
	SetParamInfof( PT_POSITION_X, "Position X Sphere", FF_TYPE_STANDARD );
	SetParamInfof( PT_POSITION_Y, "Position Y Sphere", FF_TYPE_STANDARD );
	SetParamInfof( PT_POSITION_Z, "Position Z Sphere", FF_TYPE_STANDARD );
	SetParamInfof( PT_POSITION_X_Box, "Position X Box", FF_TYPE_STANDARD );
	SetParamInfof( PT_POSITION_Y_Box, "Position Y Box", FF_TYPE_STANDARD );
	SetParamInfof( PT_POSITION_Z_Box, "Position Z Box", FF_TYPE_STANDARD );

	// scale
	SetParamInfof( PT_SCALESPHERE, "Scale Sphere", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPHERESIZE, "Size Sphere", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALEBOX, "Scale Box", FF_TYPE_STANDARD );
	SetParamInfof( PT_BOXSIZE, "Size Box", FF_TYPE_STANDARD );
	SetParamInfof( PT_GROUNDHEIGHT, "Ground Height", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALESPHEREX, "Scale Sphere X", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALESPHEREY, "Scale Sphere Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALESPHEREZ, "Scale Sphere Z", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALEBOXX, "Move Box X", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALEBOXY, "Move Box Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_SCALEBOXZ, "Move Box Z", FF_TYPE_STANDARD );

	// rotation
	SetParamInfof( PT_BOXLEFTRIGHTCROSSFADE, "Rotate Box Z", FF_TYPE_STANDARD );
	SetParamInfof( PT_BOXUPDOWNCROSSFADE, "Rotate Box X", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPHERELEFTRIGHTCROSSFADE, "Rotate Sphere Z", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPHEREUPDOWNCROSSFADE, "Rotate Sphere X", FF_TYPE_STANDARD );

	// camera
	SetParamInfof( PT_FIELDOFVIEW, "Field of View", FF_TYPE_STANDARD );
	SetParamInfof( PT_CAMERA_X, "Camera X", FF_TYPE_STANDARD );
	SetParamInfof( PT_CAMERA_Y, "Camera Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_BRIGHTNESS, "Brightness", FF_TYPE_BRIGHTNESS );

	// Parameters
	SetParamInfof( PT_ITERATIONSX, "BoxX Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_ITERATIONSY, "BoxY Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_ITERATIONSZ, "BoxZ Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_FRACTALAMOUNT, "Box Fractal", FF_TYPE_STANDARD );

	SetParamInfof( PT_ITERATIONSXSPHERE, "SphereX Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_ITERATIONSYSPHERE, "SphereY Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_ITERATIONSZSPHERE, "SphereZ Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_FRACTALAMOUNTSPHERE, "Sphere Fractal", FF_TYPE_STANDARD );

	// modulation ray
	SetParamInfof( PT_MODULATIONRAYX, "LFO X", FF_TYPE_STANDARD );
	SetParamInfof( PT_MODULATIONRAYY, "LFO Y", FF_TYPE_STANDARD );
	SetParamInfof( PT_MODULATIONRAYZ, "LFO Z", FF_TYPE_STANDARD );
	SetParamInfof( PT_RAYMODULATIONFACTOR, "LFOFactor", FF_TYPE_STANDARD );

	// sphereBoxAbsorption, groundAbsorption
	SetParamInfof( PT_SPHEREBOXABSORPTION, "Sphere Box Absorption", FF_TYPE_STANDARD );
	SetParamInfof( PT_GROUNDABSORPTION, "Ground Absorption", FF_TYPE_STANDARD );

	SetParamInfof( PT_COLORPULSE, "Color Pulse", FF_TYPE_STANDARD );
	SetParamInfof( PT_SPEED, "Speed", FF_TYPE_STANDARD );
	SetParamInfof( PT_REVERSE_TIME, "Reverse Time", FF_TYPE_BOOLEAN );
	SetParamInfof( PT_LINETHICKNESS, "Line Thickness", FF_TYPE_STANDARD );
	SetParamInfof( PT_PALETTE_0_RED, "Palette 0 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_0_GREEN, "Palette 0 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_0_BLUE, "Palette 0 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_1_RED, "Palette 1 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_1_GREEN, "Palette 1 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_1_BLUE, "Palette 1 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_2_RED, "Palette 2 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_2_GREEN, "Palette 2 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_2_BLUE, "Palette 2 Blue", FF_TYPE_BLUE );
	SetParamInfof( PT_PALETTE_3_RED, "Palette 3 Red", FF_TYPE_RED );
	SetParamInfof( PT_PALETTE_3_GREEN, "Palette 3 Green", FF_TYPE_GREEN );
	SetParamInfof( PT_PALETTE_3_BLUE, "Palette 3 Blue", FF_TYPE_BLUE );

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
	m_TimeLocation       = shader.FindUniform( "time" );
	reverseTimeLocation  = shader.FindUniform( "reverseTime" );
	m_ResolutionLocation = shader.FindUniform( "resolution" );

	fractalAmountLocation = shader.FindUniform( "fractalAmount" );
	iterationsXLocation   = shader.FindUniform( "iterationsX" );
	iterationsYLocation   = shader.FindUniform( "iterationsY" );
	iterationsZLocation   = shader.FindUniform( "iterationsZ" );

	fractalAmountSphereLocation = shader.FindUniform( "fractalAmountSphere" );
	iterationsSphereXLocation   = shader.FindUniform( "iterationsSphereX" );
	iterationsSphereYLocation   = shader.FindUniform( "iterationsSphereY" );
	iterationsSphereZLocation   = shader.FindUniform( "iterationsSphereZ" );

	speedLocation         = shader.FindUniform( "speed" );
	colorPulseLocation    = shader.FindUniform( "colorPulse" );
	lineThicknessLocation = shader.FindUniform( "lineThickness" );

	// get palette locations
	palette0RedLocation   = shader.FindUniform( "PALETTE_0_RED" );
	palette0GreenLocation = shader.FindUniform( "PALETTE_0_GREEN" );
	palette0BlueLocation  = shader.FindUniform( "PALETTE_0_BLUE" );

	palette1RedLocation   = shader.FindUniform( "PALETTE_1_RED" );
	palette1GreenLocation = shader.FindUniform( "PALETTE_1_GREEN" );
	palette1BlueLocation  = shader.FindUniform( "PALETTE_1_BLUE" );

	palette2RedLocation   = shader.FindUniform( "PALETTE_2_RED" );
	palette2GreenLocation = shader.FindUniform( "PALETTE_2_GREEN" );
	palette2BlueLocation  = shader.FindUniform( "PALETTE_2_BLUE" );

	palette3RedLocation   = shader.FindUniform( "PALETTE_3_RED" );
	palette3GreenLocation = shader.FindUniform( "PALETTE_3_GREEN" );
	palette3BlueLocation  = shader.FindUniform( "PALETTE_3_BLUE" );

	// position
	positionXLocation = shader.FindUniform( "positionXSphere" );
	positionYLocation = shader.FindUniform( "positionYSphere" );
	positionZLocation = shader.FindUniform( "positionZSphere" );

	// position box
	positionXboxLocation = shader.FindUniform( "positionXBox" );
	positionYboxLocation = shader.FindUniform( "positionYBox" );
	positionZboxLocation = shader.FindUniform( "positionZBox" );

	// scale
	scaleSphereLocation  = shader.FindUniform( "scaleSphere" );
	sizeSphereLocation   = shader.FindUniform( "sphereSize" );
	scaleBoxLocation     = shader.FindUniform( "scaleBox" );
	sizeBoxLocation      = shader.FindUniform( "boxSize" );
	groundHeightLocation = shader.FindUniform( "groundHeight" );
	scaleSphereXLocation = shader.FindUniform( "scaleSphereX" );
	scaleSphereYLocation = shader.FindUniform( "scaleSphereY" );
	scaleSphereZLocation = shader.FindUniform( "scaleSphereZ" );
	scaleBoxXLocation    = shader.FindUniform( "scaleBoxX" );
	scaleBoxYLocation    = shader.FindUniform( "scaleBoxY" );
	scaleBoxZLocation    = shader.FindUniform( "scaleBoxZ" );

	// rotation
	boxLeftRightCrossfadeLocation    = shader.FindUniform( "boxLeftRightCrossfade" );
	boxUpDownCrossfadeLocation       = shader.FindUniform( "boxUpDownCrossfade" );
	sphereLeftRightCrossfadeLocation = shader.FindUniform( "sphereLeftRightCrossfade" );
	sphereUpDownCrossfadeLocation    = shader.FindUniform( "sphereUpDownCrossfade" );

	// modulation ray
	modulationRayXLocation      = shader.FindUniform( "modulationRayX" );
	modulationRayYLocation      = shader.FindUniform( "modulationRayY" );
	modulationRayZLocation      = shader.FindUniform( "modulationRayZ" );
	rayModulationFactorLocation = shader.FindUniform( "rayModulationFactor" );

	// camera
	fieldOfViewLocation = shader.FindUniform( "fieldOfView" );
	cameraXLocation     = shader.FindUniform( "cameraX" );
	cameraYLocation     = shader.FindUniform( "cameraY" );
	brightnessLocation  = shader.FindUniform( "brightness" );

	// sphereBoxAbsorption, groundAbsorption
	sphereBoxAbsorptionLocation = shader.FindUniform( "sphereBoxAbsorption" );
	groundAbsorptionLocation    = shader.FindUniform( "groundAbsorption" );

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
	// modulation ray
	if( modulationRayXLocation != -1 )
	{
		glUniform1f( modulationRayXLocation, modulationRayX );
	}
	if( modulationRayYLocation != -1 )
	{
		glUniform1f( modulationRayYLocation, modulationRayY );
	}
	if( modulationRayZLocation != -1 )
	{
		glUniform1f( modulationRayZLocation, modulationRayZ );
	}
	if( rayModulationFactorLocation != -1 )
	{
		glUniform1f( rayModulationFactorLocation, 0.5f );
	}

	if( m_TimeLocation != -1 )
	{
		glUniform1f( m_TimeLocation, GetTimeInSeconds() );
	}
	if( reverseTimeLocation != -1 )
	{
		glUniform1i( reverseTimeLocation, reverseTime );
	}
	if( m_ResolutionLocation != -1 )
	{
		glUniform2f( m_ResolutionLocation, pGL->inputTextures[ 0 ]->Width, pGL->inputTextures[ 0 ]->Height );
	}

	if( iterationsXLocation != -1 )
	{
		glUniform1f( iterationsXLocation, iterationsX );
	}
	if( iterationsYLocation != -1 )
	{
		glUniform1f( iterationsYLocation, iterationsY );
	}
	if( iterationsZLocation != -1 )
	{
		glUniform1f( iterationsZLocation, iterationsZ );
	}

	if( iterationsSphereXLocation != -1 )
	{
		glUniform1f( iterationsSphereXLocation, iterationsSphereX );
	}
	if( iterationsSphereYLocation != -1 )
	{
		glUniform1f( iterationsSphereYLocation, iterationsSphereY );
	}
	if( iterationsSphereZLocation != -1 )
	{
		glUniform1f( iterationsSphereZLocation, iterationsSphereZ );
	}

	if( fractalAmountLocation != -1 )
	{
		glUniform1f( fractalAmountLocation, fractalAmount );
	}
	if( fractalAmountSphereLocation != -1 )
	{
		glUniform1f( fractalAmountSphereLocation, fractalAmountSphere );
	}

	if( colorPulseLocation != -1 )
	{
		glUniform1f( colorPulseLocation, colorPulse );
	}
	if( speedLocation != -1 )
	{
		glUniform1f( speedLocation, speed );
	}
	if( lineThicknessLocation != -1 )
	{
		glUniform1f( lineThicknessLocation, lineThickness );
	}

	// pallete
	if( palette0RedLocation != -1 )
	{
		glUniform1f( palette0RedLocation, palette0Red );
	}
	if( palette0GreenLocation != -1 )
	{
		glUniform1f( palette0GreenLocation, palette0Green );
	}
	if( palette0BlueLocation != -1 )
	{
		glUniform1f( palette0BlueLocation, palette0Blue );
	}

	if( palette1RedLocation != -1 )
	{
		glUniform1f( palette1RedLocation, palette1Red );
	}

	if( palette1GreenLocation != -1 )
	{
		glUniform1f( palette1GreenLocation, palette1Green );
	}

	if( palette1BlueLocation != -1 )
	{
		glUniform1f( palette1BlueLocation, palette1Blue );
	}

	if( palette2RedLocation != -1 )
	{
		glUniform1f( palette2RedLocation, palette2Red );
	}

	if( palette2GreenLocation != -1 )
	{
		glUniform1f( palette2GreenLocation, palette2Green );
	}
	if( palette2BlueLocation != -1 )
	{
		glUniform1f( palette2BlueLocation, palette2Blue );
	}
	if( palette3RedLocation != -1 )
	{
		glUniform1f( palette3RedLocation, palette3Red );
	}
	if( palette3GreenLocation != -1 )
	{
		glUniform1f( palette3GreenLocation, palette3Green );
	}
	if( palette3BlueLocation != -1 )
	{
		glUniform1f( palette3BlueLocation, palette3Blue );
	}

	// position
	if( positionXLocation != -1 )
	{
		glUniform1f( positionXLocation, positionX );
	}
	if( positionYLocation != -1 )
	{
		glUniform1f( positionYLocation, positionY );
	}
	if( positionZLocation != -1 )
	{
		glUniform1f( positionZLocation, positionZ );
	}

	// position box
	if( positionXboxLocation != -1 )
	{
		glUniform1f( positionXboxLocation, positionXbox );
	}
	if( positionYboxLocation != -1 )
	{
		glUniform1f( positionYboxLocation, positionYbox );
	}
	if( positionZboxLocation != -1 )
	{
		glUniform1f( positionZboxLocation, positionZbox );
	}

	// scale
	if( scaleSphereLocation != -1 )
	{
		glUniform1f( scaleSphereLocation, scaleSphere );
	}
	if( scaleBoxLocation != -1 )
	{
		glUniform1f( scaleBoxLocation, scaleBox );
	}
	if( groundHeightLocation != -1 )
	{
		glUniform1f( groundHeightLocation, groundHeight );
	}
	if( sizeSphereLocation != -1 )
	{
		glUniform1f( sizeSphereLocation, scaleSphere );
	}
	if( sizeBoxLocation != -1 )
	{
		glUniform1f( sizeBoxLocation, scaleBox );
	}
	if( scaleSphereXLocation != -1 )
	{
		glUniform1f( scaleSphereXLocation, scaleSphereX );
	}
	if( scaleSphereYLocation != -1 )
	{
		glUniform1f( scaleSphereYLocation, scaleSphereY );
	}
	if( scaleSphereZLocation != -1 )
	{
		glUniform1f( scaleSphereZLocation, scaleSphereZ );
	}
	if( scaleBoxXLocation != -1 )
	{
		glUniform1f( scaleBoxXLocation, scaleBoxX );
	}
	if( scaleBoxYLocation != -1 )
	{
		glUniform1f( scaleBoxYLocation, scaleBoxY );
	}
	if( scaleBoxZLocation != -1 )
	{
		glUniform1f( scaleBoxZLocation, scaleBoxZ );
	}

	// camera
	if( fieldOfViewLocation != -1 )
	{
		glUniform1f( fieldOfViewLocation, fieldOfView );
	}
	if( cameraXLocation != -1 )
	{
		glUniform1f( cameraXLocation, cameraX );
	}
	if( cameraYLocation != -1 )
	{
		glUniform1f( cameraYLocation, cameraY );
	}
	if( brightnessLocation != -1 )
	{
		glUniform1f( brightnessLocation, brightness );
	}

	// rotation
	if( boxLeftRightCrossfadeLocation != -1 )
	{
		glUniform1f( boxLeftRightCrossfadeLocation, boxLeftRightCrossfade );
	}
	if( boxUpDownCrossfadeLocation != -1 )
	{
		glUniform1f( boxUpDownCrossfadeLocation, boxUpDownCrossfade );
	}
	if( sphereLeftRightCrossfadeLocation != -1 )
	{
		glUniform1f( sphereLeftRightCrossfadeLocation, sphereLeftRightCrossfade );
	}
	if( sphereUpDownCrossfadeLocation != -1 )
	{
		glUniform1f( sphereUpDownCrossfadeLocation, sphereUpDownCrossfade );
	}

	// sphereBoxAbsorption, groundAbsorption
	if( sphereBoxAbsorptionLocation != -1 )
	{
		glUniform1f( sphereBoxAbsorptionLocation, sphereBoxAbsorption );
	}
	if( groundAbsorptionLocation != -1 )
	{
		glUniform1f( groundAbsorptionLocation, groundAbsorption );
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
	case PT_ITERATIONSX:
		iterationsX = value;
		break;
	case PT_ITERATIONSY:
		iterationsY = value;
		break;
	case PT_ITERATIONSZ:
		iterationsZ = value;
		break;
	case PT_FRACTALAMOUNT:
		fractalAmount = value;
		break;
	case PT_ITERATIONSXSPHERE:
		iterationsSphereX = value;
		break;
	case PT_ITERATIONSYSPHERE:
		iterationsSphereY = value;
		break;
	case PT_ITERATIONSZSPHERE:
		iterationsSphereZ = value;
		break;
	case PT_FRACTALAMOUNTSPHERE:
		fractalAmountSphere = value;
		break;
	case PT_COLORPULSE:
		colorPulse = value;
		break;
	case PT_SPEED:
		speed = value;
		break;
	case PT_REVERSE_TIME:
		reverseTime = value;
		break;
	case PT_PALETTE_0_RED:
		palette0Red = value;
		break;
	case PT_PALETTE_0_GREEN:
		palette0Green = value;
		break;
	case PT_PALETTE_0_BLUE:
		palette0Blue = value;
		break;
	case PT_PALETTE_1_RED:
		palette1Red = value;
		break;
	case PT_PALETTE_1_GREEN:
		palette1Green = value;
		break;
	case PT_PALETTE_1_BLUE:
		palette1Blue = value;
		break;
	case PT_PALETTE_2_RED:
		palette2Red = value;
		break;
	case PT_PALETTE_2_GREEN:
		palette2Green = value;
		break;
	case PT_PALETTE_2_BLUE:
		palette2Blue = value;
		break;
	case PT_PALETTE_3_RED:
		palette3Red = value;
		break;
	case PT_PALETTE_3_GREEN:
		palette3Green = value;
		break;
	case PT_PALETTE_3_BLUE:
		palette3Blue = value;
		break;
	case PT_LINETHICKNESS:
		lineThickness = value;
		break;
	case PT_POSITION_X:
		positionX = value;
		break;
	case PT_POSITION_Y:
		positionY = value;
		break;
	case PT_POSITION_Z:
		positionZ = value;
		break;
	case PT_FIELDOFVIEW:
		fieldOfView = value;
		break;
	case PT_POSITION_X_Box:
		positionXbox = value;
		break;
	case PT_POSITION_Y_Box:
		positionYbox = value;
		break;
	case PT_POSITION_Z_Box:
		positionZbox = value;
		break;
	case PT_SCALESPHERE:
		scaleSphere = value;
		break;
	case PT_SCALEBOX:
		scaleBox = value;
		break;
	case PT_GROUNDHEIGHT:
		groundHeight = value;
		break;
	case PT_SPHERESIZE:
		sphereSize = value;
		break;
	case PT_BOXSIZE:
		boxSize = value;
		break;
	case PT_SCALESPHEREX:
		scaleSphereX = value;
		break;
	case PT_SCALESPHEREY:
		scaleSphereY = value;
		break;
	case PT_SCALESPHEREZ:
		scaleSphereZ = value;
		break;
	case PT_SCALEBOXX:
		scaleBoxX = value;
		break;
	case PT_SCALEBOXY:
		scaleBoxY = value;
		break;
	case PT_SCALEBOXZ:
		scaleBoxZ = value;
		break;
	case PT_BOXLEFTRIGHTCROSSFADE:
		boxLeftRightCrossfade = value;
		break;
	case PT_BOXUPDOWNCROSSFADE:
		boxUpDownCrossfade = value;
		break;
	case PT_SPHERELEFTRIGHTCROSSFADE:
		sphereLeftRightCrossfade = value;
		break;
	case PT_SPHEREUPDOWNCROSSFADE:
		sphereUpDownCrossfade = value;
		break;
	case PT_CAMERA_X:
		cameraX = value;
		break;
	case PT_CAMERA_Y:
		cameraY = value;
		break;
	case PT_BRIGHTNESS:
		brightness = value;
		break;
	case PT_MODULATIONRAYX:
		modulationRayX = value;
		break;
	case PT_MODULATIONRAYY:
		modulationRayY = value;
		break;
	case PT_MODULATIONRAYZ:
		modulationRayZ = value;
		break;
	case PT_RAYMODULATIONFACTOR:
		rayModulationFactor = value;
		break;
	case PT_SPHEREBOXABSORPTION:
		sphereBoxAbsorption = value;
		break;
	case PT_GROUNDABSORPTION:
		groundAbsorption = value;
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
	case PT_ITERATIONSX:
		return iterationsX;
	case PT_ITERATIONSY:
		return iterationsY;
	case PT_ITERATIONSZ:
		return iterationsZ;
	case PT_FRACTALAMOUNT:
		return fractalAmount;
	case PT_ITERATIONSXSPHERE:
		return iterationsSphereX;
	case PT_ITERATIONSYSPHERE:
		return iterationsSphereY;
	case PT_ITERATIONSZSPHERE:
		return iterationsSphereZ;
	case PT_FRACTALAMOUNTSPHERE:
		return fractalAmountSphere;
	case PT_COLORPULSE:
		return colorPulse;
	case PT_SPEED:
		return speed;
	case PT_REVERSE_TIME:
		return reverseTime;
	case PT_LINETHICKNESS:
		return lineThickness;
	case PT_PALETTE_0_RED:
		return palette0Red;
	case PT_PALETTE_0_GREEN:
		return palette0Green;
	case PT_PALETTE_0_BLUE:
		return palette0Blue;
	case PT_PALETTE_1_RED:
		return palette1Red;
	case PT_PALETTE_1_GREEN:
		return palette1Green;
	case PT_PALETTE_1_BLUE:
		return palette1Blue;
	case PT_PALETTE_2_RED:
		return palette2Red;
	case PT_PALETTE_2_GREEN:
		return palette2Green;
	case PT_PALETTE_2_BLUE:
		return palette2Blue;
	case PT_PALETTE_3_RED:
		return palette3Red;
	case PT_PALETTE_3_GREEN:
		return palette3Green;
	case PT_PALETTE_3_BLUE:
		return palette3Blue;
	case PT_POSITION_X:
		return positionX;
	case PT_POSITION_Y:
		return positionY;
	case PT_POSITION_Z:
		return positionZ;
	case PT_FIELDOFVIEW:
		return fieldOfView;
	case PT_POSITION_X_Box:
		return positionXbox;
	case PT_POSITION_Y_Box:
		return positionYbox;
	case PT_POSITION_Z_Box:
		return positionZbox;
	case PT_SCALESPHERE:
		return scaleSphere;
	case PT_SCALEBOX:
		return scaleBox;
	case PT_GROUNDHEIGHT:
		return groundHeight;
	case PT_SPHERESIZE:
		return sphereSize;
	case PT_BOXSIZE:
		return boxSize;
	case PT_SCALESPHEREX:
		return scaleSphereX;
	case PT_SCALESPHEREY:
		return scaleSphereY;
	case PT_SCALESPHEREZ:
		return scaleSphereZ;
	case PT_SCALEBOXX:
		return scaleBoxX;
	case PT_SCALEBOXY:
		return scaleBoxY;
	case PT_SCALEBOXZ:
		return scaleBoxZ;
	case PT_BOXLEFTRIGHTCROSSFADE:
		return boxLeftRightCrossfade;
	case PT_BOXUPDOWNCROSSFADE:
		return boxUpDownCrossfade;
	case PT_SPHERELEFTRIGHTCROSSFADE:
		return sphereLeftRightCrossfade;
	case PT_SPHEREUPDOWNCROSSFADE:
		return sphereUpDownCrossfade;
	case PT_CAMERA_X:
		return cameraX;
	case PT_CAMERA_Y:
		return cameraY;
	case PT_BRIGHTNESS:
		return brightness;
	case PT_MODULATIONRAYX:
		return modulationRayX;
	case PT_MODULATIONRAYY:
		return modulationRayY;
	case PT_MODULATIONRAYZ:
		return modulationRayZ;
	case PT_RAYMODULATIONFACTOR:
		return rayModulationFactor;
	case PT_SPHEREBOXABSORPTION:
		return sphereBoxAbsorption;
	case PT_GROUNDABSORPTION:
		return groundAbsorption;
	}

	return 0.0f;
}
