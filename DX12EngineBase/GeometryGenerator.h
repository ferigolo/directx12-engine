#pragma once
#include "Mesh.h"
#include <DirectXColors.h>
#include <vector>

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indexes;
};

class GeometryGenerator
{
public:
	static MeshData CreateCube();
	static MeshData CreateTriangle();
};

