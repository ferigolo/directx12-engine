#include "GeometryGenerator.h"

using namespace DirectX;

MeshData GeometryGenerator::CreateTriangle()
{
	MeshData meshData;

	meshData.Vertices = {
		{ {  0.0f,   0.25f, 0.25f }, XMFLOAT4(Colors::Magenta) },
		{ {  0.25f, -0.25f, 0.0f }, XMFLOAT4(Colors::SpringGreen) },
		{ { -0.25f, -0.25f, 0.0f }, XMFLOAT4(Colors::Magenta) },
		{ {  0.0f,  0.125f, -0.5f }, XMFLOAT4(Colors::Aqua) }
	};

	meshData.Indexes = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	return meshData;
}

MeshData GeometryGenerator::CreateCube()
{
	MeshData meshData;

	meshData.Vertices = {
		// Frontal face
		{ { -0.25f,  0.25f, 0.0f }, XMFLOAT4(Colors::Cyan) },
		{ {  0.25f,  0.25f, 0.0f }, XMFLOAT4(Colors::DeepPink) },
		{ {  0.25f, -0.25f, 0.0f }, XMFLOAT4(Colors::Yellow) },
		{ { -0.25f, -0.25f, 0.0f }, XMFLOAT4(Colors::LimeGreen) },

		// Back face
		{ { -0.25f,  0.25f, 0.5f }, XMFLOAT4(Colors::Blue) },
		{ {  0.25f,  0.25f, 0.5f }, XMFLOAT4(Colors::Red) },
		{ {  0.25f, -0.25f, 0.5f }, XMFLOAT4(Colors::DarkOrange) },
		{ { -0.25f, -0.25f, 0.5f }, XMFLOAT4(Colors::Purple) },
	};

	meshData.Indexes = {
		0, 1, 2, 0, 2, 3, // Front face
		4, 6, 5, 4, 7, 6, // Back face
		4, 5, 1, 4, 1, 0, // Top face
		3, 2, 6, 3, 6, 7, // Bottom face
		1, 5, 6, 1, 6, 2, // Right face
		4, 0, 3, 4, 3, 7  // Left face
	};

	return meshData;
}

MeshData GeometryGenerator::CreateCylinder(float rBottom, float rTop, float height, unsigned int sliceCount, unsigned int stackCount)
{
	MeshData meshData;

	float stackHeight = height / stackCount,
		radiusStep = (rTop - rBottom) / stackCount,
		theta = 2 * XM_PI / sliceCount; // Angle for each slice
	unsigned int ringCount = stackCount + 1;

	// Compute vertices for each ring
	for (unsigned int i = 0; i < ringCount; i++)
	{
		float y = (-0.5f * height) + (i * stackHeight),
			r = rBottom + (i * radiusStep);

		for (unsigned int j = 0; j <= sliceCount; j++)
		{
			XMFLOAT4 color = (j % 2 == 0) ? XMFLOAT4(Colors::Azure) : XMFLOAT4(Colors::RoyalBlue);
			Vertex vertex{ XMFLOAT3(r * cosf(j * theta), y, r * sinf(j * theta)), color };
			meshData.Vertices.push_back(vertex);
		}
	}

	unsigned int ringVertexCount = sliceCount + 1;

	// Compute indexes for each ring
	for (unsigned int i = 0; i < stackCount; i++)
	{
		for (unsigned int j = 0; j < sliceCount; j++)
		{
			meshData.Indexes.push_back(i * ringVertexCount + j);
			meshData.Indexes.push_back((i + 1) * ringVertexCount + j);
			meshData.Indexes.push_back((i + 1) * ringVertexCount + (j + 1));
			meshData.Indexes.push_back(i * ringVertexCount + j);
			meshData.Indexes.push_back((i + 1) * ringVertexCount + (j + 1));
			meshData.Indexes.push_back(i * ringVertexCount + (j + 1));
		}
	}

	// Compute top and bottom faces (lids)
	for (unsigned int k = 0; k < 2; k++)
	{
		unsigned int startIndex = meshData.Vertices.size();
		float y = (k - 0.5f) * height,
			r = (k ? rTop : rBottom);

		XMFLOAT4 color = (k == 0) ? XMFLOAT4(Colors::LimeGreen) : XMFLOAT4(Colors::OrangeRed);
		for (unsigned int i = 0; i <= sliceCount; i++)
		{
			Vertex vertex{ XMFLOAT3(r * cosf(i * theta), y, r * sinf(i * theta)), color };
			meshData.Vertices.push_back(vertex);
		}

		// Central vertex
		unsigned int centerIndex = meshData.Vertices.size();
		Vertex vertex{ XMFLOAT3(0, y, 0), XMFLOAT4(Colors::Azure) };
		meshData.Vertices.push_back(vertex);

		// Lid indexes
		for (unsigned int i = 0; i < sliceCount; i++)
		{
			meshData.Indexes.push_back(centerIndex);
			meshData.Indexes.push_back(startIndex + i + k);
			meshData.Indexes.push_back(startIndex + i + 1 - k);
		}
	}

	return meshData;
}

MeshData GeometryGenerator::CreateSphere(float radius, unsigned int sliceCount, unsigned int stackCount)
{
	MeshData meshData;

	Vertex topVertex{ XMFLOAT3(0.0f, radius, 0.0f), XMFLOAT4(Colors::Red) }; // North pole
	meshData.Vertices.push_back(topVertex);

	const float phiStep = XM_PI / stackCount,
		thetaStep = 2 * XM_PI / sliceCount;

	for (unsigned int i = 1; i < stackCount; i++)
	{
		const float phi = i * phiStep; // Vertical angle measured from the north pole to the bottom (0≤ϕ≤π)
		for (unsigned int j = 0; j <= sliceCount; j++)
		{
			const float theta = j * thetaStep, // Horizontal angle measured from the Z axis (0≤θ<2π)

				// Espheric coordinates
				x = radius * sinf(phi) * cosf(theta),
				y = radius * cosf(phi),
				z = radius * sinf(phi) * sinf(theta);

			Vertex vertex{
				XMFLOAT3(x, y, z),
				(j % 2 == 0) ? XMFLOAT4(Colors::Azure) : XMFLOAT4(Colors::RoyalBlue) // Color
			};
			meshData.Vertices.push_back(vertex);
		}
	}

	Vertex bottomVertex{ XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT4(Colors::LimeGreen) }; // South pole
	meshData.Vertices.push_back(bottomVertex);

	// Indexes
	for (unsigned int i = 1; i <= sliceCount; i++) // Vertices connected to south pole
	{
		meshData.Indexes.push_back(0);
		meshData.Indexes.push_back(i + 1);
		meshData.Indexes.push_back(i);
	}

	// Sphere body
	unsigned int ringVertexCount = sliceCount + 1, // We repeat the first and last vertices
		baseIndex = 1;

	for (unsigned int i = 0; i < stackCount - 2; i++)
		for (unsigned int j = 0; j < sliceCount; j++)
		{
			meshData.Indexes.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indexes.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indexes.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indexes.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indexes.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indexes.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}


	unsigned int southPoleIndex = (unsigned int)meshData.Vertices.size() - 1; // Last one
	baseIndex = southPoleIndex - ringVertexCount; // Where the ring that connects to south pole vertex starts

	for (unsigned int i = 0; i < sliceCount; i++)
	{
		meshData.Indexes.push_back(southPoleIndex); // Everyone connects to south pole vertex
		meshData.Indexes.push_back(baseIndex + i);
		meshData.Indexes.push_back(baseIndex + i + 1);
	}

	return meshData;
}

MeshData GeometryGenerator::CreateGrid()
{
	MeshData meshData;
	const int gridSize = 40; // Grid Range [-20, +20]
	const float spacing = 0.75f, thickness = 0.015f;
	const XMFLOAT4 color(Colors::DimGray);

	uint16_t index = 0;

	auto AddLineAsRect = [&](float cx, float cz, float halfWidth, float halfDepth)
		{
			meshData.Vertices.push_back({ { cx - halfWidth, 0, cz + halfDepth }, color }); // Top left
			meshData.Vertices.push_back({ { cx + halfWidth, 0, cz + halfDepth }, color }); // Top right
			meshData.Vertices.push_back({ { cx - halfWidth, 0, cz - halfDepth }, color }); // Back left
			meshData.Vertices.push_back({ { cx + halfWidth, 0, cz - halfDepth }, color }); // Back right

			meshData.Indexes.push_back(index + 0);
			meshData.Indexes.push_back(index + 1);
			meshData.Indexes.push_back(index + 2);
			meshData.Indexes.push_back(index + 2);
			meshData.Indexes.push_back(index + 1);
			meshData.Indexes.push_back(index + 3);
			index += 4;
		};

	for (int i = -gridSize; i <= gridSize; ++i)
	{
		AddLineAsRect(i * spacing, 0, thickness, gridSize * spacing); // Vertical lines
		AddLineAsRect(0, i * spacing, gridSize * spacing, thickness); // Horizontal lines
	}

	return meshData;
}
