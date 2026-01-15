#include "ArxFile.hpp"
#include "ExportOBJ.hpp"


FTL_HEADERS parseHeaders(const std::vector<std::byte>& data)
{
    FTL_HEADERS headers = {};

    std::size_t pos = 0ULL;


    auto readStruct = [&]<typename T0>(T0& out)
    {
        using T = std::decay_t<T0>;

        if (pos + sizeof(T) > data.size())
        {
            throw std::runtime_error("Unexpected EOF while reading structure");
        }

        std::memcpy(&out, data.data() + pos, sizeof(T));

        pos += sizeof(T);
    };


    // ARX_FTL_PRIMARY_HEADER
    readStruct(headers.primary);

    if (headers.primary.ident[0ULL] != 'F' || headers.primary.ident[1ULL] != 'T' || headers.primary.ident[2ULL] != 'L')
    {
        throw std::runtime_error("FTL signature not detected");
    }

    fmt::print("Primary Header:\n");
    fmt::print("  Signature: \"{}\"\n  Version: \"{}\"\n", headers.primary.ident.data(), headers.primary.version);


    // Increases offset by checksum size
    pos += 512ULL;


    // ARX_FTL_SECONDARY_HEADER
    readStruct(headers.secondary);

    fmt::print("Secondary Header:\n");
    fmt::print("  3D data offset: {}\n",            headers.secondary.offset_3Ddata);
    fmt::print("  Collision spheres offset: {}\n",  headers.secondary.offset_collision_spheres);
    fmt::print("  Progressive data offset: {}\n",   headers.secondary.offset_progressive_data);
    fmt::print("  Clothes data offset: {}\n",       headers.secondary.offset_clothes_data);

    // ARX_FTL_3D_DATA_HEADER
    if (headers.secondary.offset_3Ddata != -1I32)
    {
        pos = headers.secondary.offset_3Ddata;

        readStruct(headers.data3D);

        fmt::print("3D Data Header:\n");
        fmt::print("  Vertices: {}\n",      headers.data3D.nb_vertex);
        fmt::print("  Faces: {}\n",         headers.data3D.nb_faces);
        fmt::print("  Maps: {}\n",          headers.data3D.nb_maps);
        fmt::print("  Groups: {}\n",        headers.data3D.nb_groups);
        fmt::print("  Actions: {}\n",       headers.data3D.nb_action);
        fmt::print("  Selections: {}\n",    headers.data3D.nb_selections);
    }

    // ARX_FTL_COLLISION_SPHERES_DATA_HEADER
    if (headers.secondary.offset_collision_spheres != -1I32)
    {
        pos = headers.secondary.offset_collision_spheres;

        readStruct(headers.collisionSpheresData);

        fmt::print("Collision Spheres Data:\n");
        fmt::print("  Number of spheres: {}\n", headers.collisionSpheresData.nb_spheres);
    }

    // ARX_FTL_PROGRESSIVE_DATA_HEADER
    if (headers.secondary.offset_progressive_data != -1I32)
    {
        pos = headers.secondary.offset_progressive_data;

        readStruct(headers.progressiveData);

        fmt::print("Progressive Data:\n");
        fmt::print("  Number of vertices: {}\n", headers.progressiveData.nb_vertex);
    }

    // ARX_FTL_CLOTHES_DATA_HEADER
    if (headers.secondary.offset_clothes_data != -1I32)
    {
        pos = headers.secondary.offset_clothes_data;

        readStruct(headers.clothesData);

        fmt::print("Clothes Data:\n");
        fmt::print("  Number of coverts: {}\n", headers.clothesData.nb_cvert);
        fmt::print("  Number of springs: {}\n", headers.clothesData.nb_springs);
    }

    return headers;
}

FTL_DATA parseData(const std::vector<std::byte>& data, const FTL_HEADERS& headers)
{
    FTL_DATA out = {};

    auto check = [&](std::size_t pos, std::size_t size)
    {
        if (pos + size > data.size())
        {
            throw std::runtime_error("Unexpected EOF while parsing FTL data");
        }
    };

    if (headers.secondary.offset_3Ddata != -1I32)
    {
        std::size_t pos = headers.secondary.offset_3Ddata;

        pos += sizeof(ARX_FTL_3D_DATA_HEADER);

        out.vertices.resize(headers.data3D.nb_vertex);

        for (auto& v : out.vertices)
        {
            check(pos, sizeof(EERIE_OLD_VERTEX));

            std::memcpy(&v, data.data() + pos, sizeof(EERIE_OLD_VERTEX));

            pos += sizeof(EERIE_OLD_VERTEX);
        }

        out.faces.resize(headers.data3D.nb_faces);

        for (auto& f : out.faces)
        {
            check(pos, sizeof(EERIE_FACE_FTL));

            std::memcpy(&f, data.data() + pos, sizeof(EERIE_FACE_FTL));

            pos += sizeof(EERIE_FACE_FTL);
        }

        out.textures.resize(headers.data3D.nb_maps);

        for (auto& tex : out.textures)
        {
            check(pos, 256);
            const char* str = reinterpret_cast<const char*>(data.data() + pos);
            tex = std::string(str);
            pos += 256;
        }

        out.groups.resize(headers.data3D.nb_groups);

        for (auto& g : out.groups)
        {
            check(pos, sizeof(EERIE_GROUPLIST));

            std::memcpy(&g, data.data() + pos, sizeof(EERIE_GROUPLIST));

            pos += sizeof(EERIE_GROUPLIST);
        }

        out.groupIndices.resize(out.groups.size());

        for (std::size_t i = 0; i < out.groups.size(); ++i)
        {
            if (const auto count = out.groups[i].nb_index; count > 0)
            {
                check(pos, sizeof(std::int32_t) * count);

                out.groupIndices[i].resize(count);

                std::memcpy(out.groupIndices[i].data(), data.data() + pos, sizeof(std::int32_t) * count);

                pos += sizeof(std::int32_t) * count;
            }
        }

        out.actions.resize(headers.data3D.nb_action);

        if (!out.actions.empty())
        {
            check(pos, sizeof(EERIE_ACTIONLIST) * out.actions.size());

            std::memcpy(out.actions.data(), data.data() + pos, sizeof(EERIE_ACTIONLIST) * out.actions.size());

            pos += sizeof(EERIE_ACTIONLIST) * out.actions.size();
        }

        out.selections.resize(headers.data3D.nb_selections);

        if (!out.selections.empty())
        {
            check(pos, sizeof(EERIE_SELECTIONS) * out.selections.size());

            std::memcpy(out.selections.data(), data.data() + pos, sizeof(EERIE_SELECTIONS) * out.selections.size());

            pos += sizeof(EERIE_SELECTIONS) * out.selections.size();
        }

        out.selectionIndices.resize(out.selections.size());

        for (std::size_t i = 0; i < out.selections.size(); ++i)
        {
            if (const auto count = out.selections[i].nb_selected; count > 0)
            {
                check(pos, sizeof(std::int32_t) * count);

                out.selectionIndices[i].resize(count);

                std::memcpy(out.selectionIndices[i].data(), data.data() + pos, sizeof(std::int32_t) * count);

                pos += sizeof(std::int32_t) * count;
            }
        }
    }

    if (headers.secondary.offset_collision_spheres != -1I32)
    {
        std::size_t pos = headers.secondary.offset_collision_spheres;

        pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);

        out.collisionSpheres.resize(headers.collisionSpheresData.nb_spheres);

        if (!out.collisionSpheres.empty())
        {
            check(pos, sizeof(COLLISION_SPHERE) * out.collisionSpheres.size());

            std::memcpy(out.collisionSpheres.data(), data.data() + pos, sizeof(COLLISION_SPHERE) * out.collisionSpheres.size());
        }
    }

    if (headers.secondary.offset_progressive_data != -1I32)
    {
        std::size_t pos = headers.secondary.offset_progressive_data;

        pos += sizeof(ARX_FTL_PROGRESSIVE_DATA_HEADER);

        out.progressive.resize(headers.progressiveData.nb_vertex);

        if (!out.progressive.empty())
        {
            check(pos, sizeof(PROGRESSIVE_DATA) * out.progressive.size());

            std::memcpy(out.progressive.data(), data.data() + pos, sizeof(PROGRESSIVE_DATA) * out.progressive.size());
        }
    }

    if (headers.secondary.offset_clothes_data != -1I32)
    {
        std::size_t pos = headers.secondary.offset_clothes_data;

        pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);

        out.clothesVertices.resize(headers.clothesData.nb_cvert);
        out.clothesBackup.resize(headers.clothesData.nb_cvert);

        if (!out.clothesVertices.empty())
        {
            check(pos, sizeof(CLOTHESVERTEX) * out.clothesVertices.size());

            std::memcpy(out.clothesVertices.data(), data.data() + pos, sizeof(CLOTHESVERTEX) * out.clothesVertices.size());

            std::memcpy(out.clothesBackup.data(), data.data() + pos, sizeof(CLOTHESVERTEX) * out.clothesBackup.size());

            pos += sizeof(CLOTHESVERTEX) * out.clothesVertices.size();
        }

        out.springs.resize(headers.clothesData.nb_springs);

        if (!out.springs.empty())
        {
            check(pos, sizeof(EERIE_SPRINGS) * out.springs.size());

            std::memcpy(out.springs.data(), data.data() + pos, sizeof(EERIE_SPRINGS) * out.springs.size());
        }
    }

    return out;
}


std::int32_t main(const std::int32_t argc, char* argv[])
{
    try
    {
        if (argc != 2I32 && argc != 3I32)
        {
            throw std::runtime_error("Usage: ArxConverter.exe <file.ftl> [output.obj]");
        }

        system("cls");

        std::string inputPath = argv[1ULL];
        std::string outputPath;

        if (argc == 3I32)
        {
            outputPath = argv[2ULL];
        }
        else
        {
            std::filesystem::path p(inputPath);

            outputPath = p.parent_path().string() + "/" + p.stem().string() + ".obj";
        }

        fmt::print("Input file: \"{}\"\n", inputPath);
        fmt::print("Output file: \"{}\"\n\n", outputPath);

        const auto compressed = readFile(inputPath);
        const auto decompressed = decompressFile(compressed);

        const FTL_HEADERS headers = parseHeaders(decompressed);
        const FTL_DATA data = parseData(decompressed, headers);


        fmt::print("\n--- Starting export to OBJ ---\n");

        exportToOBJ(outputPath, data, headers);
    }
    catch (const std::exception& e)
    {
        fmt::print("Error: {}\n", e.what());

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}