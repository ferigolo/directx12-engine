#include "GeometryGenerator.h"

using namespace DirectX;

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
		0, 1, 2, 0, 2, 3, // Face Frontal
		4, 6, 5, 4, 7, 6, // Face Traseira
		4, 5, 1, 4, 1, 0, // Face Superior
		3, 2, 6, 3, 6, 7, // Face Inferior
		1, 5, 6, 1, 6, 2, // Face Direita
		4, 0, 3, 4, 3, 7  // Face Esquerda
	};

	return meshData;
}

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