module;

#include <map>
#include <array>
#include <vector>
#include <numeric>
#include <clocale>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "nlohmann/json.hpp"
#include "tinyxml2.h"

module ArxConverter.ArxExporter;

namespace fs = std::filesystem;
using json   = nlohmann::json;

using namespace tinyxml2;

ArxExporter::ArxExporter(const FtlHeaders& headers, const FtlFileData& data, const String& outputDir, Logger& logger) noexcept : m_headers{ headers }, m_data{ data }, m_baseOutputDirectory{ outputDir }, m_logger{ logger }
{

}

Void ArxExporter::exportAll()
{
    std::setlocale(LC_NUMERIC, "C");

    createDirectories();

    m_logger.print<LogLevel::Info>("Exporting JSON...");
    exportJson();

    m_logger.print<LogLevel::Info>("Exporting XML...");
    exportXml();

    m_logger.print<LogLevel::Info>("Exporting OBJ/MTL...");
    exportObjMtl();

    m_logger.print<LogLevel::Info>("Exporting GLTF 2.0...");
    exportGltf();
}

Void ArxExporter::createDirectories() const
{
    const fs::path base{ m_baseOutputDirectory };

    fs::create_directories(base / "RAW" / "JSON");
    fs::create_directories(base / "RAW" / "XML");
    fs::create_directories(base / "OBJ");
    fs::create_directories(base / "GLTF");
}

Void ArxExporter::exportJson() const
{
    const fs::path jsonDir = fs::path(m_baseOutputDirectory) / "RAW" / "JSON";

	constexpr Size bufSize = 64 * 1024;

    DynamicArray<Char8> buffer(bufSize);

    {
        std::ofstream file(jsonDir / "Headers.json");

        file.rdbuf()->pubsetbuf(buffer.data(), bufSize);

        json jHead;

        jHead["primary"] =
        {
            { "identifier", String(m_headers.primary.identifier.data(), 3) },
            { "version",    m_headers.primary.version }
        };

        jHead["secondary_offsets"] =
        {
            { "3dData",           m_headers.secondary.offset3dData },
            { "clothesData",      m_headers.secondary.offsetClothesData },
            { "collisionSpheres", m_headers.secondary.offsetCollisionSpheres },
            { "progressiveData",  m_headers.secondary.offsetProgressiveData },
            { "cylinder",         m_headers.secondary.offsetCylinder },
            { "physicsBox",       m_headers.secondary.offsetPhysicsBox }
        };

        jHead["section_headers"] =
        {
            { "3dData", {
                { "vertexCount",       m_headers.data3D.vertexCount },
                { "faceCount",         m_headers.data3D.faceCount },
                { "textureCount",      m_headers.data3D.textureCount },
                { "groupCount",        m_headers.data3D.groupCount },
                { "actionCount",       m_headers.data3D.actionCount },
                { "selectionCount",    m_headers.data3D.selectionCount },
                { "originVertexIndex", m_headers.data3D.originVertexIndex },
                { "modelName",         String(m_headers.data3D.modelName.data()) }
            }},
            {
            	"clothes",
			{
                { "vertexCount", m_headers.clothesData.clothVertexCount },
                { "springCount", m_headers.clothesData.springCount }
            }},
            { "progressive",
            {
                { "vertexCount", m_headers.progressiveData.vertexCount }
            }},
            { "collisionSpheres",
            {
                { "count", m_headers.collisionSpheresData.sphereCount }
            }}
        };

        file << jHead.dump(4);
    }

    {
        std::ofstream file(jsonDir / "Data.json");

        file.rdbuf()->pubsetbuf(buffer.data(), bufSize);

        json jData;

        DynamicArray<json> jVertices;

        jVertices.reserve(m_data.vertices.size());

        for (const auto& [legacyVertex, position, normal] : m_data.vertices)
        {
            jVertices.push_back(
            {
                { "position", { position.x, position.y, position.z } },
                { "normal",   { normal.x, normal.y, normal.z } },
                { "legacy",
                {
                    { "screen", { legacyVertex.screenX, legacyVertex.screenY, legacyVertex.screenZ } },
                    { "rhw", legacyVertex.rhw },
                    { "color", legacyVertex.color },
                    { "specular", legacyVertex.specular },
                    { "uv", { legacyVertex.textureU, legacyVertex.textureV } }
                }}
            });
        }

        jData["vertices"] = std::move(jVertices);

        DynamicArray<json> jFaces;

        jFaces.reserve(m_data.faces.size());

        for (const auto& f : m_data.faces)
        {
            jFaces.push_back(
            {
                { "indices", f.vertexIndices },
                { "texIndices", f.textureIndex },
                { "flags", f.faceTypeFlags },
                { "transparency", f.transparency },
                { "texCoords",
                {
                    { "u", f.textureU },
                    { "v", f.textureV }
                }},
                { "vertexColors", f.vertexColors },
                { "faceNormal", { f.faceNormal.x, f.faceNormal.y, f.faceNormal.z } }
            });
        }
        jData["faces"] = std::move(jFaces);


        DynamicArray<json> jGroups;

        jGroups.reserve(m_data.vertexGroups.size());

        for (Index i = 0; i < m_data.vertexGroups.size(); ++i)
        {
            const auto& g = m_data.vertexGroups[i];

            json gJson;

            gJson["name"] = String(g.groupName.data());
            gJson["originVertex"] = g.originVertexIndex;
            gJson["radius"] = g.boundingRadius;

            if (i < m_data.groupVertexIndices.size())
            {
                gJson["vertexIndices"] = m_data.groupVertexIndices[i];
            }

            jGroups.push_back(std::move(gJson));
        }

        jData["groups"] = std::move(jGroups);

        DynamicArray<json> jActions;

        jActions.reserve(m_data.actionPoints.size());

        for (const auto& [actionName, vertexIndex, actionTypeFlags, soundEffectId] : m_data.actionPoints)
        {
            jActions.push_back(
			{
                { "name", String(actionName.data()) },
                { "vertex", vertexIndex },
                { "flags", actionTypeFlags },
                { "soundId", soundEffectId }
            });
        }
        jData["actions"] = std::move(jActions);

        DynamicArray<json> jSelections;

        jSelections.reserve(m_data.vertexSelections.size());

        for (Size i = 0; i < m_data.vertexSelections.size(); ++i)
        {
            const auto& sel = m_data.vertexSelections[i];

            json sJson;

            sJson["name"] = String(sel.selectionName.data());

            if (i < m_data.selectionVertexIndices.size())
            {
                sJson["vertexIndices"] = m_data.selectionVertexIndices[i];
            }

            jSelections.push_back(std::move(sJson));
        }

        jData["selections"] = std::move(jSelections);
        jData["textures"] = m_data.texturePaths;

        json jCloth;

        DynamicArray<json> jClothVerts;

        jClothVerts.reserve(m_data.clothVertices.size());

        for (const auto& cv : m_data.clothVertices)
        {
            jClothVerts.push_back(
            {
                { "meshIndex", cv.meshVertexIndex },
                { "pos", { cv.position.x, cv.position.y, cv.position.z } },
                { "mass", cv.mass },
                { "flags", cv.behaviorFlags },
                { "collision", cv.collisionEnabled }
            });
        }

        jCloth["vertices"] = std::move(jClothVerts);

        DynamicArray<json> jClothSprings;

        jClothSprings.reserve(m_data.clothSprings.size());

        for (const auto& [startVertexIndex, endVertexIndex, restLength, stiffness, damping, springType] : m_data.clothSprings)
        {
            jClothSprings.push_back(
            {
                { "v1", startVertexIndex },
                { "v2", endVertexIndex },
                { "restLength", restLength },
                { "stiffness", stiffness },
                { "damping", damping },
                { "type", springType }
            });
        }

        jCloth["springs"] = std::move(jClothSprings);
        jData["cloth"] = std::move(jCloth);

        file << jData.dump(4);
    }
}

Void ArxExporter::exportXml() const
{
    const fs::path xmlDir = fs::path(m_baseOutputDirectory) / "RAW" / "XML";

    auto setVec3 = [](XMLElement* el, const char* name, const Vector3D& v)
	{
        Char8 buf[64];

        std::snprintf(buf, sizeof(buf), "%s_x", name); el->SetAttribute(buf, v.x);
        std::snprintf(buf, sizeof(buf), "%s_y", name); el->SetAttribute(buf, v.y);
        std::snprintf(buf, sizeof(buf), "%s_z", name); el->SetAttribute(buf, v.z);
    };

    {
        XMLDocument docHead;

        XMLElement* rootHead = docHead.NewElement("FtlHeaders");

        docHead.InsertEndChild(rootHead);


        XMLElement* prim = docHead.NewElement("Primary");

        prim->SetAttribute("ID", String(m_headers.primary.identifier.data(), 3).c_str());
        prim->SetAttribute("Ver", m_headers.primary.version);
        rootHead->InsertEndChild(prim);


        XMLElement* sec = docHead.NewElement("Offsets");

        sec->SetAttribute("Data3D", m_headers.secondary.offset3dData);
        sec->SetAttribute("Clothes", m_headers.secondary.offsetClothesData);
        sec->SetAttribute("Collision", m_headers.secondary.offsetCollisionSpheres);
        sec->SetAttribute("Progressive", m_headers.secondary.offsetProgressiveData);
        rootHead->InsertEndChild(sec);


        XMLElement* h3d = docHead.NewElement("Header3D");

        h3d->SetAttribute("Vertices", m_headers.data3D.vertexCount);
        h3d->SetAttribute("Faces", m_headers.data3D.faceCount);
        h3d->SetAttribute("Groups", m_headers.data3D.groupCount);
        h3d->SetAttribute("ModelName", m_headers.data3D.modelName.data());
        rootHead->InsertEndChild(h3d);

        docHead.SaveFile((xmlDir / "Headers.xml").string().c_str());
    }

    {
        XMLDocument docData;
        XMLElement* rootData = docData.NewElement("FtlData");

        docData.InsertEndChild(rootData);


        XMLElement* xVerts = docData.NewElement("Vertices");

        xVerts->SetAttribute("count", static_cast<Int64>(m_data.vertices.size()));

        for (Size i = 0; i < m_data.vertices.size(); ++i)
        {
            const auto& [legacyVertex, position, normal] = m_data.vertices[i];


            XMLElement* el                               = docData.NewElement("V");

            el->SetAttribute("id", static_cast<Int64>(i));

            setVec3(el, "p", position);
            setVec3(el, "n", normal);

            el->SetAttribute("color", legacyVertex.color);
            el->SetAttribute("u", legacyVertex.textureU);
            el->SetAttribute("v", legacyVertex.textureV);

            xVerts->InsertEndChild(el);
        }
        rootData->InsertEndChild(xVerts);

        XMLElement* xFaces = docData.NewElement("Faces");

        xFaces->SetAttribute("count", static_cast<Int64>(m_data.faces.size()));

        for (Index i = 0; i < m_data.faces.size(); ++i)
        {
            const auto& f = m_data.faces[i];
            XMLElement* el = docData.NewElement("F");

            el->SetAttribute("id", static_cast<Int64>(i));
            el->SetAttribute("tex", f.textureIndex);
            el->SetAttribute("flags", f.faceTypeFlags);

            el->SetAttribute("v1", f.vertexIndices[0]);
            el->SetAttribute("v2", f.vertexIndices[1]);
            el->SetAttribute("v3", f.vertexIndices[2]);

            el->SetAttribute("u1", f.textureU[0]); el->SetAttribute("v1", f.textureV[0]);
            el->SetAttribute("u2", f.textureU[1]); el->SetAttribute("v2", f.textureV[1]);
            el->SetAttribute("u3", f.textureU[2]); el->SetAttribute("v3", f.textureV[2]);

            xFaces->InsertEndChild(el);
        }

        rootData->InsertEndChild(xFaces);

        XMLElement* xGroups = docData.NewElement("Groups");

        for (Index i = 0; i < m_data.vertexGroups.size(); ++i)
        {
            const auto& g = m_data.vertexGroups[i];

            XMLElement* el = docData.NewElement("Group");

            el->SetAttribute("name", g.groupName.data());
            el->SetAttribute("root", g.originVertexIndex);

            if (i < m_data.groupVertexIndices.size())
            {
                String indicesStr;

                indicesStr.reserve(m_data.groupVertexIndices[i].size() * 6);

                for (const auto idx : m_data.groupVertexIndices[i])
                {
                     indicesStr += std::to_string(idx);
                     indicesStr += ' ';
                }

                if (!indicesStr.empty()) indicesStr.pop_back();

                el->SetText(indicesStr.c_str());
            }

            xGroups->InsertEndChild(el);
        }

        rootData->InsertEndChild(xGroups);


        XMLElement* xCloth = docData.NewElement("Cloth");

        for (const auto& cv : m_data.clothVertices)
        {
            XMLElement* el = docData.NewElement("CV");

            el->SetAttribute("ref", cv.meshVertexIndex);
            setVec3(el, "pos", cv.position);

            xCloth->InsertEndChild(el);
        }
        for (const auto& cs : m_data.clothSprings)
        {
            XMLElement* el = docData.NewElement("CS");

            el->SetAttribute("a", cs.startVertexIndex);
            el->SetAttribute("b", cs.endVertexIndex);
            el->SetAttribute("k", cs.stiffness);

            xCloth->InsertEndChild(el);
        }

        rootData->InsertEndChild(xCloth);

        docData.SaveFile((xmlDir / "Data.xml").string().c_str());
    }
}

Void ArxExporter::exportObjMtl() const
{
    const fs::path objDir = fs::path(m_baseOutputDirectory) / "OBJ";

    if (!fs::exists(objDir)) {
        fs::create_directories(objDir);
    }

    const String objPath = (objDir / "model.obj").string();
    const String mtlPath = (objDir / "model.mtl").string();

    constexpr Size bufSize = 64 * 1024;

    DynamicArray<Char8> objBuffer(bufSize);
    DynamicArray<Char8> mtlBuffer(bufSize);

    std::ofstream objFile(objPath);
    std::ofstream mtlFile(mtlPath);

    if (!objFile || !mtlFile)
    {
        return;
    }

    objFile.rdbuf()->pubsetbuf(objBuffer.data(), bufSize);
    mtlFile.rdbuf()->pubsetbuf(mtlBuffer.data(), bufSize);

    objFile.precision(6);
    objFile << std::fixed;

    mtlFile << "# ArxConverter Material File\n";

    for (Size i = 0; i < m_data.texturePaths.size(); ++i)
    {
        mtlFile << "newmtl Material_" << i << "\n";
        mtlFile << "Ka 1.0 1.0 1.0\nKd 1.0 1.0 1.0\nKs 0.0 0.0 0.0\n";
        mtlFile << "map_Kd " << fs::path(m_data.texturePaths[i]).filename().string() << "\n\n";
    }

    objFile << "# ArxConverter OBJ Export\n";
    objFile << "mtllib model.mtl\n";

    for (const auto& v : m_data.vertices)
    {
        objFile << "v " << v.position.x << " " << -v.position.y << " " << -v.position.z << "\n";
    }

    for (const auto& face : m_data.faces)
    {
        for (Int32 i = 0; i < 3; ++i) {
            objFile << "vt " << face.textureU[i] << " " << (1.0f - face.textureV[i]) << "\n";
        }
    }

    for (const auto& v : m_data.vertices)
    {
        objFile << "vn " << v.normal.x << " " << -v.normal.y << " " << -v.normal.z << "\n";
    }

    Int16 currentTextureIndex = -999;
    Size uvCounter = 1;

    for (const auto& face : m_data.faces)
    {
        if (face.textureIndex != currentTextureIndex)
        {
            currentTextureIndex = face.textureIndex;

            objFile << "usemtl ";

            if (currentTextureIndex >= 0 && currentTextureIndex < static_cast<Int16>(m_data.texturePaths.size()))
            {
                objFile << "Material_" << currentTextureIndex << "\n";
            }
            else
            {
                objFile << "Default\n";
            }
        }

        const UInt32 idx1 = face.vertexIndices[0] + 1;
        const UInt32 idx2 = face.vertexIndices[1] + 1;
        const UInt32 idx3 = face.vertexIndices[2] + 1;

        const UInt32 tIdx1 = uvCounter++;
        const UInt32 tIdx2 = uvCounter++;
        const UInt32 tIdx3 = uvCounter++;

        objFile << "f " << idx1 << "/" << tIdx1 << "/" << idx1 << " "
                        << idx2 << "/" << tIdx2 << "/" << idx2 << " "
                        << idx3 << "/" << tIdx3 << "/" << idx3 << "\n";
    }
}

Void ArxExporter::exportGltf() const
{
    const fs::path gltfDir   = fs::path(m_baseOutputDirectory) / "GLTF";
    const String filenameBase = "model";

    const fs::path gltfPath   = gltfDir / (filenameBase + ".gltf");
    const fs::path binPath    = gltfDir / (filenameBase + ".bin");

    if (!fs::exists(gltfDir)) {
        fs::create_directories(gltfDir);
    }

    DynamicArray<Float32> bufferPositions;
    DynamicArray<Float32> bufferNormals;

    bufferPositions.reserve(m_data.vertices.size() * 3);
    bufferNormals.reserve(m_data.vertices.size() * 3);

    std::array<Float64, 3> minPos = { 1e9, 1e9, 1e9 };
    std::array<Float64, 3> maxPos = { -1e9, -1e9, -1e9 };

    for (const auto& v : m_data.vertices)
    {
        const Float32 x = v.position.x;
        const Float32 y = -v.position.y;
        const Float32 z = -v.position.z;

        bufferPositions.push_back(x);
        bufferPositions.push_back(y);
        bufferPositions.push_back(z);

        if (x < minPos[0]) minPos[0] = x; if (x > maxPos[0]) maxPos[0] = x;
        if (y < minPos[1]) minPos[1] = y; if (y > maxPos[1]) maxPos[1] = y;
        if (z < minPos[2]) minPos[2] = z; if (z > maxPos[2]) maxPos[2] = z;

        bufferNormals.push_back(v.normal.x);
        bufferNormals.push_back(-v.normal.y);
        bufferNormals.push_back(-v.normal.z);
    }

    DynamicArray<Float32> bufferTexCoords(m_data.vertices.size() * 2, 0.0f);

    std::map<Int16, DynamicArray<UInt16>> materialIndices;

    for (const auto& face : m_data.faces)
    {
        for (Int32 i = 0; i < 3; ++i)
        {
            const Int32 vIdx = face.vertexIndices[i];

            bufferTexCoords[vIdx * 2 + 0] = face.textureU[i];
            bufferTexCoords[vIdx * 2 + 1] = face.textureV[i];
        }

        auto& indices = materialIndices[face.textureIndex];

        indices.push_back(face.vertexIndices[0]);
        indices.push_back(face.vertexIndices[1]);
        indices.push_back(face.vertexIndices[2]);
    }

    std::ofstream binFile(binPath, std::ios::binary);

    if (!binFile)
    {
        return;
    }

    DynamicArray<Char8> fileBuf(64 * 1024);

    binFile.rdbuf()->pubsetbuf(fileBuf.data(), fileBuf.size());


    auto writeChunk = [&](const void* data, Size size) -> Size
    {
        Size startOffset = static_cast<Size>(binFile.tellp());

        while (startOffset % 4 != 0)
        {
            binFile.put(0); startOffset++;
        }

        binFile.write(static_cast<const char*>(data), size);

        return startOffset;
    };

    const Size offPos = writeChunk(bufferPositions.data(), bufferPositions.size() * sizeof(Float32));
    const Size lenPos = bufferPositions.size() * sizeof(Float32);

    const Size offNorm = writeChunk(bufferNormals.data(), bufferNormals.size() * sizeof(Float32));
    const Size lenNorm = bufferNormals.size() * sizeof(Float32);

    const Size offUV = writeChunk(bufferTexCoords.data(), bufferTexCoords.size() * sizeof(Float32));
    const Size lenUV = bufferTexCoords.size() * sizeof(Float32);


    DynamicArray<std::pair<Size, Size>> sortedIndicesOffsets;

    sortedIndicesOffsets.reserve(materialIndices.size());

    std::map<Int16, Size> texIdToOffsetIndex;

    for (auto& [texId, indices] : materialIndices)
    {
        const Size len = indices.size() * sizeof(UInt16);
        const Size off = writeChunk(indices.data(), len);

        texIdToOffsetIndex[texId] = sortedIndicesOffsets.size();
        sortedIndicesOffsets.emplace_back(off, len);
    }

    while (static_cast<Size>(binFile.tellp()) % 4 != 0)
    {
        binFile.put(0);
    }

    const Size totalByteLength = binFile.tellp();

    binFile.close();

    json root;

    root["asset"] = { {"version", "2.0"}, {"generator", "ArxConverter"} };
    root["buffers"] = json::array({ { {"uri", filenameBase + ".bin"}, {"byteLength", totalByteLength} } });


    DynamicArray<json> bufferViews;

    bufferViews.push_back({ {"buffer", 0}, {"byteOffset", offPos}, {"byteLength", lenPos}, {"target", 34962} });
    bufferViews.push_back({ {"buffer", 0}, {"byteOffset", offNorm}, {"byteLength", lenNorm}, {"target", 34962} });
    bufferViews.push_back({ {"buffer", 0}, {"byteOffset", offUV}, {"byteLength", lenUV}, {"target", 34962} });


    Int32 currentIndexBufferViewId = 3;

    std::map<Int16, Int32> texIdToBufferViewId;

    for (const auto& [texId, idx] : texIdToOffsetIndex)
    {
        const auto& [fst, snd] = sortedIndicesOffsets[idx];

        bufferViews.push_back({ {"buffer", 0}, {"byteOffset", fst}, {"byteLength", snd}, {"target", 34963} });

        texIdToBufferViewId[texId] = currentIndexBufferViewId++;
    }

    root["bufferViews"] = std::move(bufferViews);


    DynamicArray<json> accessors;

    accessors.push_back({ {"bufferView", 0}, {"componentType", 5126}, {"count", bufferPositions.size()/3}, {"type", "VEC3"}, {"min", minPos}, {"max", maxPos} });
    accessors.push_back({ {"bufferView", 1}, {"componentType", 5126}, {"count", bufferNormals.size()/3}, {"type", "VEC3"} });
    accessors.push_back({ {"bufferView", 2}, {"componentType", 5126}, {"count", bufferTexCoords.size()/2}, {"type", "VEC2"} });


    std::map<Int16, Int32> texIdToAccessorId;

    Int32 currentAccessorId = 3;


    for (const auto& [texId, indices] : materialIndices)
    {
        const Int32 bvId = texIdToBufferViewId[texId];

        accessors.push_back({ {"bufferView", bvId}, {"componentType", 5123}, {"count", indices.size()}, {"type", "SCALAR"} });

        texIdToAccessorId[texId] = currentAccessorId++;
    }

    root["accessors"] = std::move(accessors);

    DynamicArray<json> images;
    DynamicArray<json> textures;
    DynamicArray<json> materials;

    std::map<Int16, Int32> texIdToMaterialId;

    for (Size i = 0; i < m_data.texturePaths.size(); ++i)
    {
        images.push_back({ {"uri", fs::path(m_data.texturePaths[i]).filename().string()} });

        textures.push_back({ {"source", i} });

        materials.push_back(
        {
            {"name", "Material_" + std::to_string(i)},
            {"pbrMetallicRoughness", { {"baseColorTexture", { {"index", i} }}, {"metallicFactor", 0.0}, {"roughnessFactor", 1.0} }},
            {"doubleSided", true}
        });

        texIdToMaterialId[static_cast<Int16>(i)] = static_cast<Int32>(i);
    }

    Int32 defaultMatId = -1;

    if (materialIndices.contains(-1) || materialIndices.size() > m_data.texturePaths.size())
    {
        materials.push_back({ {"name", "Default"}, {"pbrMetallicRoughness", { {"baseColorFactor", {0.8, 0.8, 0.8, 1.0}} }} });

        defaultMatId = static_cast<Int32>(materials.size() - 1);
    }

    if (!images.empty())
    {
        root["images"] = std::move(images);
    }
    if (!textures.empty())
    {
        root["textures"] = std::move(textures);
    }

    root["materials"] = std::move(materials);

    DynamicArray<json> primitives;

    for (const auto& texId: materialIndices | std::views::keys)
    {
        const Int32 matId = (texId >= 0 && texId < static_cast<Int16>(m_data.texturePaths.size())) ? texIdToMaterialId[texId] : defaultMatId;

        json prim = { {"attributes", { {"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2} }}, {"indices", texIdToAccessorId[texId]}, {"mode", 4} };

        if (matId != -1)
        {
            prim["material"] = matId;
        }

        primitives.push_back(std::move(prim));
    }

    root["meshes"] = json::array({ { {"name", String(m_headers.data3D.modelName.data())}, {"primitives", std::move(primitives)} } });


    DynamicArray<json> nodes;

    nodes.push_back({ {"name", "Mesh"}, {"mesh", 0} });

    for(const auto& ap : m_data.actionPoints)
    {
        const auto& v = m_data.vertices[ap.vertexIndex];

        nodes.push_back(
        {
            {"name", String(ap.actionName.data())},
            // ИСПРАВЛЕНИЕ: здесь тоже ставим минусы (-Y, -Z)
            {"translation", { v.position.x, -v.position.y, -v.position.z }},
            {"extras", { {"type", "ActionPoint"} }}
        });
    }

    DynamicArray<Int32> sceneNodes(nodes.size());

    std::iota(sceneNodes.begin(), sceneNodes.end(), 0);

    root["nodes"]  = std::move(nodes);
    root["scenes"] = json::array({ { {"nodes", sceneNodes} } });
    root["scene"]  = 0;


    if(std::ofstream gltfFile(gltfPath); gltfFile)
    {
         gltfFile.rdbuf()->pubsetbuf(fileBuf.data(), fileBuf.size());

         gltfFile << root.dump(4);
    }
}