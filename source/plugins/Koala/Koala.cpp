#include "Koala.h"
#include <math.h>//floor
using namespace ffglex;

enum ParamType : FFUInt32
{
	// koala sexy shaders
	PT_ITERATIONS,
	PT_FRACTALAMOUNT,
	PT_COLORPULSE,
	PT_SPEED,
	PT_REVERSE_TIME,
	PT_LINETHICKNESS,
	PT_PALETTE_0_RED,
	PT_PALETTE_0_GREEN,
	PT_PALETTE_0_BLUE,
	PT_PALETTE_1_RED,
	PT_PALETTE_1_GREEN,
	PT_PALETTE_1_BLUE,
	PT_PALETTE_2_RED,
	PT_PALETTE_2_GREEN,
	PT_PALETTE_2_BLUE,
	PT_PALETTE_3_RED,
	PT_PALETTE_3_GREEN,
	PT_PALETTE_3_BLUE,
	PT_POSITION_X,
	PT_POSITION_Y,
	PT_POSITION_Z,
	PT_FIELDOFVIEW,
	PT_POSITION_X_Box,
	PT_POSITION_Y_Box,
	PT_POSITION_Z_Box,
	PT_SCALESPHERE,
	PT_SCALEBOX,
	PT_GROUNDHEIGHT,
	PT_SPHERESIZE,
	PT_BOXSIZE,
	PT_SCALESPHEREX,
	PT_SCALESPHEREY,
	PT_SCALESPHEREZ,
	PT_SCALEBOXX,
	PT_SCALEBOXY,
	PT_SCALEBOXZ,
	PT_BOXLEFTRIGHTCROSSFADE,
	PT_BOXUPDOWNCROSSFADE,
	PT_SPHERELEFTRIGHTCROSSFADE,
	PT_SPHEREUPDOWNCROSSFADE,
	PT_CAMERA_X,
	PT_CAMERA_Y,
	PT_BRIGHTNESS
};

static CFFGLPluginInfo PluginInfo(
	PluginFactory< Koala >,              // Create method
	"KOA1",                              // Plugin unique ID
	"KoalaSexyShaders",                  // Plugin name
	2,                                   // API major version number
	1,                                   // API minor version number
	1,                                   // Plugin major version number
	000,                                 // Plugin minor version number
	FF_SOURCE,                           // Plugin type
	"Koala Sample FFGL Gradients plugin",// Plugin description
	"Resolume FFGL Example koala"        // About
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
uniform float iterations;

uniform float speed;

uniform float fractalAmount;

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

float map(
    vec3 p,
    float posXsphere,
    float posYsphere,
    float posZsphere,
    float posXbox,
    float posYbox,
    float posZbox,
    float sphereSize,
    float boxSize,
    float groundHeight,
    float time,
    float boxLeftRightCrossfade,
    float boxUpDownCrossfade,
    float sphereLeftRightCrossfade,
    float sphereUpDownCrossfade,
    float modulation,
    float fractalAmount,
    float boxDisplacementX,
    float boxDisplacementY,
    float boxDisplacementZ
) {
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
    float sphere = sdSphere(sphereFinalPos, sphereSize);

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
    vec3 boxFinalPostmp = fract(boxFinalPos * modulation) - 0.5;
    boxFinalPos = mix(boxFinalPos, boxFinalPostmp, fractalAmount);

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

    float modulation = iterations * 10.0 - 5.0;

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
    for(int i = 0; i < 64; i++) {
        vec3 p = ro + rd * t; // current position
        // distance to closest object
        float d = map(p, posX0, posY0, posZ0, posX1, posY1, posZ1, sphereSize, boxSize, GroundHeight, time, boxLeftRightCrossfade, boxUpDownCrossfade, sphereLeftRightCrossfade, sphereUpDownCrossfade, modulation, fractalAmount, scaleBoxX, scaleBoxY, scaleBoxZ);

        t += d; // move along ray by distance to closest object

        // col = vec3(float(i)) / 64.0; // color based on iteration
        if(d < 0.001)
            break; // break if close enough to object
        if(t > 100.0)
            break; // break if too far away
    }

    // color debugging
    col = vec3(t * brightness);

    // fragColor = vec4(uvBase, 0.0, 1.0);
    fragColor = vec4(col, 1.0);
}
)";

Koala::Koala() :
	iterations( 1.0f ),
	iterationsLocation( -1 ),
	fractalAmountLocation( -1 ),
	fractalAmount( 1.5f ),
	speedLocation( -1 ),
	colorPulseLocation( -1 ),
	colorPulse( 1.5f ),
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
	scaleSphere( 0.5f ),
	scaleBoxLocation( -1 ),
	scaleBox( 0.5f ),
	groundHeightLocation( -1 ),
	groundHeight( 0.5f ),
	scaleSphereXLocation( -1 ),
	scaleSphereX( 0.5f ),
	scaleSphereYLocation( -1 ),
	scaleSphereY( 0.5f ),
	scaleSphereZLocation( -1 ),
	scaleSphereZ( 0.5f ),
	sizeBoxLocation( -1 ),
	boxSize( 0.5f ),
	sizeSphereLocation( -1 ),
	sphereSize( 0.5f ),
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
	brightness( 0.05f )

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
	SetParamInfof( PT_ITERATIONS, "Iterations", FF_TYPE_STANDARD );
	SetParamInfof( PT_FRACTALAMOUNT, "Fractal Amount", FF_TYPE_STANDARD );
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
	m_TimeLocation        = shader.FindUniform( "time" );
	reverseTimeLocation   = shader.FindUniform( "reverseTime" );
	m_ResolutionLocation  = shader.FindUniform( "resolution" );
	iterationsLocation    = shader.FindUniform( "iterations" );
	speedLocation         = shader.FindUniform( "speed" );
	fractalAmountLocation = shader.FindUniform( "fractalAmount" );
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

	// camera
	fieldOfViewLocation = shader.FindUniform( "fieldOfView" );
	cameraXLocation     = shader.FindUniform( "cameraX" );
	cameraYLocation     = shader.FindUniform( "cameraY" );
	brightnessLocation  = shader.FindUniform( "brightness" );

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
	if( iterationsLocation != -1 )
	{
		glUniform1f( iterationsLocation, iterations );
	}
	if( fractalAmountLocation != -1 )
	{
		glUniform1f( fractalAmountLocation, fractalAmount );
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
		glUniform1f( palette1RedLocation, palette0Red );
	}

	if( palette1GreenLocation != -1 )
	{
		glUniform1f( palette1GreenLocation, palette0Green );
	}

	if( palette1BlueLocation != -1 )
	{
		glUniform1f( palette1BlueLocation, palette0Blue );
	}

	if( palette2RedLocation != -1 )
	{
		glUniform1f( palette2RedLocation, palette0Red );
	}

	if( palette2GreenLocation != -1 )
	{
		glUniform1f( palette2GreenLocation, palette0Green );
	}
	if( palette2BlueLocation != -1 )
	{
		glUniform1f( palette2BlueLocation, palette0Blue );
	}
	if( palette3RedLocation != -1 )
	{
		glUniform1f( palette3RedLocation, palette0Red );
	}
	if( palette3GreenLocation != -1 )
	{
		glUniform1f( palette3GreenLocation, palette0Green );
	}
	if( palette3BlueLocation != -1 )
	{
		glUniform1f( palette3BlueLocation, palette0Blue );
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
	case PT_ITERATIONS:
		iterations = value;
		break;
	case PT_FRACTALAMOUNT:
		fractalAmount = value;
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
	default:
		return FF_FAIL;
	}

	return FF_SUCCESS;
}

float Koala::GetFloatParameter( unsigned int index )
{
	switch( index )
	{
	case PT_ITERATIONS:
		return iterations;
	case PT_FRACTALAMOUNT:
		return fractalAmount;
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
	}

	return 0.0f;
}
