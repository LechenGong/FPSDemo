#include "Engine/Renderer/Camera.hpp"

void Camera::SetOrthoView( Vec2 const& bottomLeft, Vec2 const& topRight, float zNear, float zFar )
{
	m_mode = Mode::Orthographic;
	m_orthoBL = bottomLeft;
	m_orthoTR = topRight;
	m_orthoNear = zNear;
	m_orthoFar = zFar;
}

void Camera::SetPerspeciveView( float aspect, float fov, float zNear, float zFar )
{
	m_mode = Mode::Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = zNear;
	m_perspectiveFar = zFar;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthoBL;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthoTR;
}

void Camera::Translate2D( const Vec2& translation2D )
{
	m_orthoTR += translation2D;
	m_orthoBL += translation2D;
}

Vec2 Camera::GetCenter() const
{
	return (m_orthoBL + m_orthoTR) * 0.5;
}

AABB2 Camera::GetBounds() const
{
	return AABB2( GetOrthoBottomLeft(), GetOrthoTopRight() );
}

Vec2 Camera::GetDimensions() const
{
	return m_orthoTR - m_orthoBL;
}

Mat44 Camera::GetOrthoMatrix() const
{
	return Mat44::CreateOrthoProjection( m_orthoBL.x, m_orthoTR.x, m_orthoBL.y, m_orthoTR.y, m_orthoNear, m_orthoFar );
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection( m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar );
}

Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 mat;
	switch (m_mode)
	{
	case Mode::Orthographic:
		mat = GetOrthoMatrix();
		break;
	case Mode::Perspective:
		mat = GetPerspectiveMatrix();
		mat.Append( GetRenderMatrix() );
		break;
	}
	return mat;
}

void Camera::SetRenderBasis( Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis )
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

Mat44 Camera::GetRenderMatrix() const
{
	return Mat44( m_renderIBasis, m_renderJBasis, m_renderKBasis, Vec3::ZERO );
}

void Camera::SetTransform( Vec3 const& position, EulerAngles const& orientation )
{
	m_position = position;
	m_orientation = orientation;
}

Mat44 Camera::GetViewMatrix() const
{
	Mat44 mat = Mat44::CreateTranslation3D( m_position );
	mat.Append( m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	return mat.GetOrthonormalInverse();
}

void Camera::MoveCenterTo( const Vec2& newCenter )
{
	Translate2D( newCenter - GetCenter() );
}

Mat44 Camera::GetTransformMatrix() const
{
	Mat44 mat = Mat44::CreateTranslation3D( m_position );
	mat.Append( m_orientation.GetAsMatrix_IFwd_JLeft_KUp() );
	return mat;
}

void Camera::SetViewport( AABB2 viewport )
{
	m_viewport = viewport;
}

AABB2 Camera::GetViewport() const
{
	return m_viewport;
}
