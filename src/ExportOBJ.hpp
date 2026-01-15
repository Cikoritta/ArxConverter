#pragma once

#include <filesystem>
#include <fstream>
#include <fmt/core.h>

#include "ArxStruct.hpp"


inline void exportMTL(const std::string& mtlPath, const FTL_DATA& data)
{
	std::ofstream mtl(mtlPath);

	if (!mtl.is_open())
	{
		throw std::runtime_error("Failed to create MTL file: " + mtlPath);
	}

	mtl << "# Arx Fatalis FTL Materials\n\n";

	mtl << "newmtl default\n";
	mtl << "Ka 1.0 1.0 1.0\n";
	mtl << "Kd 0.8 0.8 0.8\n";
	mtl << "Ks 0.5 0.5 0.5\n";
	mtl << "Ns 32.0\n\n";

	for (std::size_t i = 0ULL; i < data.textures.size(); ++i)
	{
		std::string texName = data.textures[i];

		if (const std::size_t lastSlash = texName.find_last_of("/\\"); lastSlash != std::string::npos) { texName = texName.substr(lastSlash + 1); }

		while (!texName.empty() && (texName.back() == '\0' || texName.back() == ' ')) { texName.pop_back(); }

		if (texName.empty()) { continue; }

		mtl << "newmtl material_" << i << "\n";
		mtl << "Ka 1.0 1.0 1.0\n";
		mtl << "Kd 0.8 0.8 0.8\n";
		mtl << "Ks 0.5 0.5 0.5\n";
		mtl << "Ns 32.0\n";
		mtl << "map_Kd " << texName << "\n\n";
	}

	mtl.close();

	fmt::print("MTL file saved: {}\n", mtlPath);
}

inline void exportToOBJ(const std::string& outputPath, const FTL_DATA& data, const FTL_HEADERS& headers)
{
	std::ofstream obj(outputPath);

	if (!obj.is_open())
	{
		throw std::runtime_error("Failed to create OBJ file: " + outputPath);
	}

	obj << "# Arx Fatalis FTL Model\n";
	obj << "# Exported by FTL Converter\n";
	obj << "# Vertices: " << data.vertices.size() << "\n";
	obj << "# Faces: " << data.faces.size() << "\n\n";

	const std::string mtlFileName = std::filesystem::path(outputPath).stem().string() + ".mtl";
	const std::string mtlPath     = std::filesystem::path(outputPath).parent_path().string() + "/" + mtlFileName;

	obj << "mtllib " << mtlFileName << "\n\n";


	fmt::print("Exporting {} vertices...\n", data.vertices.size());

	for (const auto& vertex: data.vertices)
	{
		obj << fmt::format("v {:.6f} {:.6f} {:.6f}\n", vertex.v.x, -vertex.v.y, vertex.v.z);
	}

	obj << "\n";

	fmt::print("Exporting vertex normals...\n");

	for (const auto& vertex: data.vertices)
	{
		obj << fmt::format("vn {:.6f} {:.6f} {:.6f}\n", vertex.norm.x, -vertex.norm.y, vertex.norm.z);
	}
	obj << "\n";

	fmt::print("Exporting UV coordinates...\n");

	for (const auto& face: data.faces)
	{
		for (std::int32_t i = 0I32; i < IOPOLYVERT; ++i)
		{
			obj << fmt::format("vt {:.6f} {:.6f}\n", face.u[i], 1.0f - face.v[i]);
		}
	}

	obj << "\n";

	fmt::print("Exporting {} faces...\n", data.faces.size());

	std::int32_t currentTexId = -1I32;
	std::int32_t uvIndex      = 1I32;

	for (std::size_t faceIdx = 0ULL; faceIdx < data.faces.size(); ++faceIdx)
	{
		const auto& face = data.faces[faceIdx];

		if (face.texid != currentTexId)
		{
			currentTexId = face.texid;

			if (currentTexId >= 0I32 && currentTexId < static_cast<std::int32_t>(data.textures.size()))
			{
				std::string texName = data.textures[currentTexId];

				if (const std::size_t lastSlash = texName.find_last_of("/\\"); lastSlash != std::string::npos)
				{
					texName = texName.substr(lastSlash + 1I32);
				}

				obj << "usemtl material_" << currentTexId << "\n";
			}
			else
			{
				obj << "usemtl default\n";
			}
		}

		obj << "f";


		for (std::int32_t i = 0I32; i < IOPOLYVERT; ++i)
		{
			std::int32_t vertexIdx = face.vid[i] + 1I32;

			obj << fmt::format(" {}/{}/{}", vertexIdx, uvIndex + i, vertexIdx);
		}

		obj << "\n";

		uvIndex += IOPOLYVERT;
	}

	obj.close();

	fmt::print("OBJ file saved: {}\n", outputPath);

	exportMTL(mtlPath, data);
}