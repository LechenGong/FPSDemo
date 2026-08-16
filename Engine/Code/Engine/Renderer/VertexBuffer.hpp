#pragma once

struct ID3D11Buffer;

class VertexBuffer
{
	friend class Renderer;

public:
	VertexBuffer( size_t size );
	VertexBuffer( VertexBuffer const& copy ) = delete;
	virtual ~VertexBuffer();

	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
	UINT m_stride = 0;
	bool m_isLinePrimitive = false;
};
