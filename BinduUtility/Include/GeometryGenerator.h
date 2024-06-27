#pragma once

#include <cstdint>
#include <DirectXMath.h>
#include <vector>

class GeometryGenerator
{

public:

	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;

	struct Vertex
	{
	public:

		Vertex() : Position(0.f, 0.f, 0.f), Normal(0.f, 0.f, 0.f), TangentU(0.0f, 0.f, 0.f), TexCoord(0.f, 0.f)
		{}

		/*
			p = Position of the vertex
			n = Normal of the vertex
			t = TangentU of the vertex
			uv = Texture Coordinate of the vertex
		*/
		Vertex(const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT3& t,
			const DirectX::XMFLOAT2& uv) : Position(p), Normal(n), TangentU(t), TexCoord(uv)
		{}

		/*
			px, py, pz = Position of the vertex
			nx, ny, nz = Normal of the vertex
			tx, ty, tz = TangentU of the vertex
			u, v = Texture Coordinate of the vertex
		*/
		Vertex(float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) : Position(px, py, pz), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexCoord(u, v)
		{}



		DirectX::XMFLOAT3	Position;
		DirectX::XMFLOAT3	Normal;
		DirectX::XMFLOAT3	TangentU;
		DirectX::XMFLOAT2	TexCoord;

	};

	struct MeshData
	{
	public:

		std::vector<Vertex>	Vertices;
		std::vector<uint32>	Indices32;

		std::vector<uint16>& GetIndices16()
		{
			if (m_indices16.empty())
			{
				m_indices16.resize(Indices32.size());

				for (size_t i = 0; i < Indices32.size(); i++)
					m_indices16[i] = static_cast<uint16>(Indices32[i]);
			}

			return m_indices16;
		}

	private:
		std::vector<uint16> m_indices16;
	};

	///<summary>
	/// Creates a box centered at the origin with the given dimensions, where each
	/// face has m rows and n columns of vertices.
	///</summary>
	MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	MeshData CreateGeosphere(float radius, uint32 numSubdivisions);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

	///<summary>
	/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
	///</summary>
	MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:
	void Subdivide(MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);


};
