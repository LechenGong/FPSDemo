#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera
{
public:
	enum class Mode
	{
		Orthographic,
		Perspective,
		Count,
	};

	void SetOrthoView( Vec2 const& bottomLeft, Vec2 const& topRight, float zNear = 0.0f, float zFar = 1.0f );
	void SetPerspeciveView( float aspect, float fov, float zNear, float zFar);

	void Translate2D( const Vec2& translation2D );

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	Vec2 GetCenter() const;
	AABB2 GetBounds() const;
	Vec2 GetDimensions() const;

	Mat44 GetOrthoMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

	void SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis );
	Mat44 GetRenderMatrix() const;

	void SetTransform( Vec3 const& position, EulerAngles const& orientation );
	Mat44 GetViewMatrix() const;

	void MoveCenterTo( const Vec2& newCenter );

	Mat44 GetTransformMatrix() const;

	void SetViewport( AABB2 viewport );
	AABB2 GetViewport() const;

	void SetFov( float fov ) { m_perspectiveFOV = fov; }

	Vec3 m_position;
	EulerAngles m_orientation;

private:
	Mode m_mode = Mode::Orthographic;

	Vec2 m_orthoBL;
	Vec2 m_orthoTR;

	AABB2 m_viewport;

	float m_orthoNear;
	float m_orthoFar;

	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Vec3 m_renderIBasis = Vec3( 1.f, 0.f, 0.f );
	Vec3 m_renderJBasis = Vec3( 0.f, 1.f, 0.f );
	Vec3 m_renderKBasis = Vec3( 0.f, 0.f, 1.f );

};
