#include "GeometryGenerator.h"

MeshData GeometryGenerator::CreateCube()
{
	MeshData meshData;

	meshData.Vertices = {
		{ { -0.25f,  0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.25f,  0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ {  0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f,  0.25f, 0.5f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
		{ {  0.25f,  0.25f, 0.5f }, { 0.0f, 1.0f, 1.0f, 1.0f } },
		{ {  0.25f, -0.25f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.5f }, { 0.0f, 0.0f, 0.0f, 1.0f } }
	};

	// 2. A lista de instruções (Índices)
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
		{ {  0.0f,  0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	meshData.Indexes = { 0, 1, 2 };

	return meshData;
}