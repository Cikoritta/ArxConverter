module;

#include <cstring>
#include <string_view>

module ArxConverter.ArxParser;


template <typename Type>
const Type* ArxParser::mapRaw(Size offset) const
{
    if (offset + sizeof(Type) > m_data.size())
    {
        m_logger.print<LogLevel::Error>("Unexpected EOF mapping structure at offset {}", offset);
    }

    return reinterpret_cast<const Type*>(m_data.data() + offset);
}


ArxParser::ArxParser(const DynamicArray<Byte>& data, Logger& logger) noexcept : m_data{ data }, m_logger{ logger }
{
    parse();
}

const FtlHeaders& ArxParser::getHeaders() const noexcept
{
    return m_headers;
}

const FtlFileData& ArxParser::getData() const noexcept
{
    return m_fileData;
}


Void ArxParser::parse()
{
    if (m_data.empty())
    {
        m_logger.print<LogLevel::Error>("Attempted to parse empty data.");
    }

    Size position = 0ULL;

    parseHeaders(position);

    parseData(position);
}

Void ArxParser::parseHeaders(Size& pos)
{
    if (auto* ptr = mapRaw<FtlPrimaryHeader>(pos))
    {
        m_headers.primary = *ptr;

        pos += sizeof(FtlPrimaryHeader);
    }

    if (m_headers.primary.identifier[0] != 'F' ||
        m_headers.primary.identifier[1] != 'T' ||
        m_headers.primary.identifier[2] != 'L')
    {
        m_logger.print<LogLevel::Error>("FTL signature not detected.");
    }


    m_logger.print<LogLevel::Info>("Signature: \"{}{}{}\"",static_cast<Char8>(m_headers.primary.identifier[0]), static_cast<Char8>(m_headers.primary.identifier[1]), static_cast<Char8>(m_headers.primary.identifier[2]));

    m_logger.print<LogLevel::Info>("Version:   \"{}\"", m_headers.primary.version);

    pos += 512ULL;


    if (auto* ptr = mapRaw<FtlSecondaryHeader>(pos))
    {
        m_headers.secondary = *ptr;

        pos += sizeof(FtlSecondaryHeader);
    }


    auto readSectionHeader = [&]<typename T0>(Int32 offset, T0& targetStruct)
	{
        if (offset != -1)
        {
        	const Size sectionPos = static_cast<Size>(offset);

            if (auto* ptr = mapRaw<std::remove_reference_t<T0>>(sectionPos))
            {
                targetStruct = *ptr;
            }
        }
    };

    readSectionHeader(m_headers.secondary.offset3dData, m_headers.data3D);

    readSectionHeader(m_headers.secondary.offsetCollisionSpheres, m_headers.collisionSpheresData);

    readSectionHeader(m_headers.secondary.offsetProgressiveData, m_headers.progressiveData);

    readSectionHeader(m_headers.secondary.offsetClothesData, m_headers.clothesData);
}

Void ArxParser::parseData(Size& pos)
{
    auto readArray = [&]<typename Type>(DynamicArray<Type>& vec, Int32 count, Size& currentPos)
    {
        if (count <= 0)
        {
        	return;
        }

        const Size elemCount   = static_cast<Size>(count);

        const Size sizeInBytes = sizeof(Type) * elemCount;

        if (currentPos + sizeInBytes > m_data.size())
        {
            m_logger.print<LogLevel::Error>("Unexpected EOF while reading array at offset {}", currentPos);
        }

        vec.resize(elemCount);

        std::memcpy(vec.data(), m_data.data() + currentPos, sizeInBytes);

        currentPos += sizeInBytes;
    };


    if (m_headers.secondary.offset3dData != -1)
    {
        pos = static_cast<Size>(m_headers.secondary.offset3dData) + sizeof(Ftl3dDataHeader);


        readArray(m_fileData.vertices, m_headers.data3D.vertexCount, pos);

        readArray(m_fileData.faces, m_headers.data3D.faceCount, pos);


		if (const Int32 texCount = m_headers.data3D.textureCount; texCount > 0)
        {
            m_fileData.texturePaths.resize(static_cast<Size>(texCount));

            const Byte* rawData  = m_data.data();

            const Size totalSize = m_data.size();

            for (auto& texPath : m_fileData.texturePaths)
            {
                if (pos + 256ULL > totalSize)
                {
                    m_logger.print<LogLevel::Error>("Unexpected EOF reading textures");
                }

                const Char8* strStart = reinterpret_cast<const Char8*>(rawData + pos);

                texPath = String(strStart);

                pos += 256ULL;
            }
        }

        readArray(m_fileData.vertexGroups, m_headers.data3D.groupCount, pos);

        if (!m_fileData.vertexGroups.empty())
        {
            m_fileData.groupVertexIndices.resize(m_fileData.vertexGroups.size());

            for (Index i = 0; i < m_fileData.vertexGroups.size(); ++i)
            {
				if (const Int32 count = m_fileData.vertexGroups[i].vertexCount; count > 0)
                {
                    readArray(m_fileData.groupVertexIndices[i], count, pos);
                }
            }
        }

        readArray(m_fileData.actionPoints, m_headers.data3D.actionCount, pos);

        readArray(m_fileData.vertexSelections, m_headers.data3D.selectionCount, pos);


        if (!m_fileData.vertexSelections.empty())
        {
            m_fileData.selectionVertexIndices.resize(m_fileData.vertexSelections.size());

            for (Index i = 0; i < m_fileData.vertexSelections.size(); ++i)
            {
				if (const Int32 count = m_fileData.vertexSelections[i].vertexCount; count > 0)
                {
                    readArray(m_fileData.selectionVertexIndices[i], count, pos);
                }
            }
        }
    }

    if (m_headers.secondary.offsetCollisionSpheres != -1)
    {
        pos = static_cast<Size>(m_headers.secondary.offsetCollisionSpheres) + sizeof(FtlCollisionSpheresHeader);

        readArray(m_fileData.collisionSpheres, m_headers.collisionSpheresData.sphereCount, pos);
    }

    if (m_headers.secondary.offsetProgressiveData != -1)
    {
        pos = static_cast<Size>(m_headers.secondary.offsetProgressiveData) + sizeof(FtlProgressiveDataHeader);

        readArray(m_fileData.progressiveMeshData, m_headers.progressiveData.vertexCount, pos);
    }

    if (m_headers.secondary.offsetClothesData != -1)
    {
        pos = static_cast<Size>(m_headers.secondary.offsetClothesData) + sizeof(FtlClothesDataHeader);

		if (const Int32 vertCount = m_headers.clothesData.clothVertexCount; vertCount > 0)
        {
            readArray(m_fileData.clothVertices, vertCount, pos);

            readArray(m_fileData.clothBackupVertices, vertCount, pos);
        }

        readArray(m_fileData.clothSprings, m_headers.clothesData.springCount, pos);
    }
}