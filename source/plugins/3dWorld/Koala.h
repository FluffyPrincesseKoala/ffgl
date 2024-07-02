#pragma once
#include <FFGLSDK.h>

class Koala : public CFFGLPlugin
{
public:
	Koala();
	~Koala();

	//CFFGLPlugin
	FFResult InitGL( const FFGLViewportStruct* vp ) override;
	FFResult ProcessOpenGL( ProcessOpenGLStruct* pGL ) override;
	FFResult DeInitGL() override;

	FFResult SetFloatParameter( unsigned int dwIndex, float value ) override;

	float GetFloatParameter( unsigned int index ) override;

	FFUInt32 m_TimeLocation;
	FFUInt32 m_ResolutionLocation;

	FFUInt32 m_CamXLocation;
	FFUInt32 m_CamYLocation;
	FFUInt32 m_CamZLocation;

	FFUInt32 m_CamRotXLocation;
	FFUInt32 m_CamRotYLocation;

	FFUInt32 m_PyramidSizeLocation;
	FFUInt32 m_PyramidSizeBaseLocation;

	FFUInt32 m_pyramid_fractal_offset_x_location;
	FFUInt32 m_pyramid_fractal_offset_y_location;
	FFUInt32 m_pyramid_fractal_offset_z_location;

	FFUInt32 m_ElongationXLocation;
	FFUInt32 m_ElongationYLocation;
	FFUInt32 m_ElongationZLocation;

	FFUInt32 m_SphereXLocation;
	FFUInt32 m_SphereYLocation;
	FFUInt32 m_SphereZLocation;

	FFUInt32 m_ObjAXLocation;
	FFUInt32 m_ObjAYLocation;
	FFUInt32 m_ObjAZLocation;

	FFUInt32 m_ObjBXLocation;
	FFUInt32 m_ObjBYLocation;
	FFUInt32 m_ObjBZLocation;

	FFUInt32 m_ObjCXLocation;
	FFUInt32 m_ObjCYLocation;
	FFUInt32 m_ObjCZLocation;

	FFUInt32 m_SphereSizeLocation;

	FFUInt32 m_TimeDeltaLocation;

	FFUInt32 m_selectionObjectALocation;
	FFUInt32 m_selectionObjectBLocation;
	FFUInt32 m_selectionObjectCLocation;

	// object selection
	float m_selectionObjectA = 0.0f;
	float m_selectionObjectB = 1.0f;
	float m_selectionObjectC = 2.0f;

	// object position
	float m_ObjAX = 0.0f;
	float m_ObjAY = 0.0f;
	float m_ObjAZ = 0.0f;

	float m_ObjBX = 0.0f;
	float m_ObjBY = 0.0f;
	float m_ObjBZ = 0.0f;

	float m_ObjCX = 0.0f;
	float m_ObjCY = 0.0f;
	float m_ObjCZ = 0.0f;

	// camera position
	float m_CamX = 0.0f;
	float m_CamY = 0.0f;
	float m_CamZ = 0.0f;

	// camera rotation
	float m_CamRotX = 0.0f;
	float m_CamRotY = 0.0f;

	// pyramidSize
	float m_PyramidSize     = 1.0f;
	float m_PyramidSizeBase = 0.3f;

	// elongation
	float m_ElongationX = 0.5f;
	float m_ElongationY = 0.5f;
	float m_ElongationZ = 0.5f;

	// fractal offset
	float m_pyramid_fractal_offset_x = 0.0f;
	float m_pyramid_fractal_offset_y = 0.0f;
	float m_pyramid_fractal_offset_z = 0.0f;

	// Sphere position
	float m_SphereX = 0.0f;
	float m_SphereY = 0.0f;
	float m_SphereZ = 0.0f;

	// Sphere size
	float m_SphereSize = 0.5f;

	// texture
	GLuint m_texture = 0;

private:
	struct RGBA
	{
		float red   = 1.0f;
		float green = 1.0f;
		float blue  = 0.0f;
		float alpha = 1.0f;
	};
	struct HSBA
	{
		float hue   = 0.0f;
		float sat   = 1.0f;
		float bri   = 1.0f;
		float alpha = 1.0f;
	};
	HSBA hsba2;

	ffglex::FFGLShader shader;  //!< Utility to help us compile and link some shaders into a program.
	ffglex::FFGLScreenQuad quad;//!< Utility to help us render a full screen quad.
};
