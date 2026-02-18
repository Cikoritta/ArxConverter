module;

export module ArxConverter.ArxHeaders;


import ArxConverter.Types;
import ArxConverter.Container;


#pragma pack(push, 4)

// ============================================================================
// Primary FTL File Header
// ============================================================================

/// Primary header at the beginning of every FTL file
/// Contains file signature and format version
export struct FtlPrimaryHeader
{
    /// File signature, must be "FTL" (4 bytes with null terminator)
    Array<Char8, 4> identifier = {};

    /// FTL format version number
    Float32 version = 0.0f;
};


// ============================================================================
// Secondary Header - Offset Table
// ============================================================================

/// Secondary header containing byte offsets to all data sections
/// A value of -1 indicates the section is not present in the file
export struct FtlSecondaryHeader
{
    /// Byte offset to 3D geometry data section
    Int32 offset3dData = -1;

    /// Byte offset to cylinder collision data section (legacy, often unused)
    Int32 offsetCylinder = -1;

    /// Byte offset to progressive mesh (LOD) data section
    Int32 offsetProgressiveData = -1;

    /// Byte offset to cloth simulation data section
    Int32 offsetClothesData = -1;

    /// Byte offset to collision spheres data section
    Int32 offsetCollisionSpheres = -1;

    /// Byte offset to physics bounding box data section (legacy, often unused)
    Int32 offsetPhysicsBox = -1;
};


// ============================================================================
// Section Headers
// ============================================================================

/// Header for the 3D geometry data section
export struct Ftl3dDataHeader
{
    /// Total number of vertices in the mesh
    Int32 vertexCount = 0;

    /// Total number of triangular faces in the mesh
    Int32 faceCount = 0;

    /// Total number of texture file paths
    Int32 textureCount = 0;

    /// Total number of vertex groups (bones/skeleton parts)
    Int32 groupCount = 0;

    /// Total number of action points on the model
    Int32 actionCount = 0;

    /// Total number of vertex selections (named sets)
    Int32 selectionCount = 0;

    /// Index of the vertex serving as the model's origin point
    Int32 originVertexIndex = 0;

    /// Internal name of the model (null-terminated string)
    Array<Char8, 256> modelName = {};
};

/// Header for progressive mesh (LOD) data section
export struct FtlProgressiveDataHeader
{
    /// Number of vertices with progressive mesh data
    Int32 vertexCount = 0;
};

/// Header for cloth simulation data section
export struct FtlClothesDataHeader
{
    /// Number of cloth vertices
    Int32 clothVertexCount = 0;

    /// Number of spring constraints between cloth vertices
    Int32 springCount = 0;
};

/// Header for collision spheres data section
export struct FtlCollisionSpheresHeader
{
    /// Number of collision spheres
    Int32 sphereCount = 0;
};


// ============================================================================
// 3D Geometry Structures
// ============================================================================

/// 3D vector used for positions, normals, velocities, and forces
export struct Vector3D
{
    Float32 x = 0.0f; ///< X coordinate
    Float32 y = 0.0f; ///< Y coordinate
    Float32 z = 0.0f; ///< Z coordinate
};

/// Legacy DirectX transformed and lit vertex structure
/// Contains screen-space coordinates, color, and texture coordinates
export struct LegacyD3DVertex
{
    Float32 screenX = 0.0f;     ///< Screen X coordinate
    Float32 screenY = 0.0f;     ///< Screen Y coordinate
    Float32 screenZ = 0.0f;     ///< Screen Z coordinate (depth)
    Float32 rhw = 0.0f;         ///< Reciprocal of homogeneous W
    UInt32 color = 0;           ///< Vertex color (ARGB format)
    UInt32 specular = 0;        ///< Specular color (ARGB format)
    Float32 textureU = 0.0f;    ///< Texture U coordinate
    Float32 textureV = 0.0f;    ///< Texture V coordinate
};

/// Single mesh vertex with position and normal
export struct MeshVertex
{
    /// Legacy DirectX vertex data (often unused in modern code)
    LegacyD3DVertex legacyVertex = {};

    /// Vertex position in model-space coordinates
    Vector3D position = {};

    /// Vertex normal vector for lighting calculations
    Vector3D normal = {};
};

/// Single triangular face with texture mapping and lighting data
export struct MeshFace
{
    /// Face type flags (transparency, double-sided, etc.)
    Int32 faceTypeFlags = 0;

    /// Vertex colors for each of the 3 triangle vertices (ARGB format)
    Array<UInt32, 3> vertexColors = {};

    /// Indices of the 3 vertices forming this triangle
    Array<UInt16, 3> vertexIndices = {};

    /// Index into the texture array (-1 if untextured)
    Int16 textureIndex = -1;

    /// Texture U coordinates for each vertex (floating-point)
    Array<Float32, 3> textureU = {};

    /// Texture V coordinates for each vertex (floating-point)
    Array<Float32, 3> textureV = {};

    /// Legacy integer texture U coordinates (obsolete)
    Array<Int16, 3> legacyTextureU = {};

    /// Legacy integer texture V coordinates (obsolete)
    Array<Int16, 3> legacyTextureV = {};

    /// Transparency value for the entire face
    Float32 transparency = 0.0f;

    /// Face normal vector (perpendicular to the triangle plane)
    Vector3D faceNormal = {};

    /// Per-vertex normal vectors for smooth shading
    Array<Vector3D, 3> vertexNormals = {};

    /// Temporary field used by the engine for intermediate calculations
    Float32 tempValue = 0.0f;
};


// ============================================================================
// Skeleton and Animation Structures
// ============================================================================

/// Vertex group representing a bone or body part in the skeleton
export struct VertexGroup
{
    /// Name of the group/bone
    Array<Char8, 256> groupName = {};

    /// Index of the root vertex for this group (bone attachment point)
    Int32 originVertexIndex = 0;

    /// Number of vertices belonging to this group
    Int32 vertexCount = 0;

    /// Legacy offset field (unused, indices stored separately)
    Int32 legacyIndicesOffset = 0;

    /// Bounding sphere radius for this group (used for rough collision detection)
    Float32 boundingRadius = 0.0f;
};

/// Action point on the model (weapon attachment, particle source, etc.)
export struct ActionPoint
{
    /// Name of the action point
    Array<Char8, 256> actionName = {};

    /// Index of the vertex this action point is attached to
    Int32 vertexIndex = 0;

    /// Action type flags (bitfield)
    Int32 actionTypeFlags = 0;

    /// Associated sound effect ID
    Int32 soundEffectId = 0;
};

/// Named selection of vertices (used by editors or scripts)
export struct VertexSelection
{
    /// Name of the selection
    Array<Char8, 64> selectionName = {};

    /// Number of vertices in this selection
    Int32 vertexCount = 0;

    /// Legacy offset field (unused, indices stored separately)
    Int32 legacyIndicesOffset = 0;
};


// ============================================================================
// Collision Structures
// ============================================================================

/// Collision sphere following a vertex for approximate collision detection
export struct CollisionSphere
{
    /// Index of the vertex this sphere is attached to (sphere center)
    Int16 vertexIndex = 0;

    /// Behavior flags for collision handling
    Int16 behaviorFlags = 0;

    /// Radius of the collision sphere
    Float32 radius = 0.0f;
};


// ============================================================================
// Progressive Mesh (LOD) Structures
// ============================================================================

/// Progressive mesh data for a single vertex (Level of Detail system)
/// Allows dynamic simplification of the mesh based on distance
export struct ProgressiveMeshVertex
{
    /// Index of the vertex this one collapses into during simplification
    Int16 collapseTargetIndex = -1;

    /// Flag indicating if this vertex needs recomputation
    Int16 needsRecomputation = 0;

    /// Distance threshold at which this vertex collapses
    Float32 collapseDistanceRatio = 0.0f;

    /// Error metric - cost of removing this vertex
    Float32 collapseCost = 0.0f;

    /// Next vertex candidate for collapse
    Int16 nextCollapseCandidate = -1;

    /// Padding byte for alignment (unused)
    Int16 padding = 0;
};


// ============================================================================
// Cloth Simulation Structures
// ============================================================================

/// Cloth vertex using Verlet integration for physics simulation
export struct ClothVertex
{
    /// Index of the corresponding mesh vertex
    Int16 meshVertexIndex = 0;

    /// Behavior flags (e.g., pinned, fixed)
    Int8 behaviorFlags = 0;

    /// Collision participation flag
    Int8 collisionEnabled = 0;

    /// Current position in world space
    Vector3D position = {};

    /// Current velocity vector
    Vector3D velocity = {};

    /// Accumulated force (gravity, wind, constraints, etc.)
    Vector3D force = {};

    /// Mass of the vertex (0.0 = infinite mass, vertex is fixed)
    Float32 mass = 0.0f;

    /// Temporary position buffer (next integration step)
    Vector3D tempPosition = {};

    /// Temporary velocity buffer (next integration step)
    Vector3D tempVelocity = {};

    /// Temporary force buffer (next integration step)
    Vector3D tempForce = {};

    /// Position in the previous simulation step (for Verlet integration)
    Vector3D previousPosition = {};
};

/// Spring constraint between two cloth vertices
/// Maintains distance and provides structural integrity to cloth
export struct ClothSpring
{
    /// Index of the first cloth vertex
    Int16 startVertexIndex = 0;

    /// Index of the second cloth vertex
    Int16 endVertexIndex = 0;

    /// Rest length - natural distance the spring tries to maintain
    Float32 restLength = 0.0f;

    /// Spring stiffness constant (higher = stiffer cloth)
    Float32 stiffness = 0.0f;

    /// Damping coefficient (reduces oscillation)
    Float32 damping = 0.0f;

    /// Spring type (structural, shear, bend)
    Int32 springType = 0;
};

#pragma pack(pop)


// ============================================================================
// Aggregate Header Structure
// ============================================================================

/// Complete collection of all FTL file headers
export struct FtlHeaders
{
    FtlPrimaryHeader          primary              = {};   ///< Primary file header

    FtlSecondaryHeader        secondary            = {};   ///< Offset table

    FtlProgressiveDataHeader  progressiveData      = {};   ///< Progressive mesh header

    FtlClothesDataHeader      clothesData          = {};   ///< Cloth simulation header

    FtlCollisionSpheresHeader collisionSpheresData = {};   ///< Collision spheres header

    Ftl3dDataHeader           data3D               = {};   ///< 3D geometry header
};


// ============================================================================
// Complete FTL File Data
// ============================================================================

/// Complete parsed FTL file data containing all sections
export struct FtlFileData
{
    // 3D Geometry
    DynamicArray<MeshVertex>          vertices               = {}; ///< All mesh vertices

    DynamicArray<MeshFace>            faces                  = {}; ///< All triangular faces

    DynamicArray<String>              texturePaths           = {}; ///< Texture file paths

    DynamicArray<VertexGroup>         vertexGroups           = {}; ///< Skeleton groups/bones

    DynamicArray<DynamicArray<Int32>> groupVertexIndices     = {}; ///< Vertex indices per group

    DynamicArray<ActionPoint>         actionPoints           = {}; ///< Action points

    DynamicArray<VertexSelection>     vertexSelections       = {}; ///< Named vertex selections

    DynamicArray<DynamicArray<Int32>> selectionVertexIndices = {}; ///< Vertex indices per selection

    // Collision
    DynamicArray<CollisionSphere> collisionSpheres = {}; ///< Collision detection spheres

    // Progressive Mesh (LOD)
    DynamicArray<ProgressiveMeshVertex> progressiveMeshData = {}; ///< LOD data per vertex

    // Cloth Simulation
    DynamicArray<ClothVertex> clothVertices       = {}; ///< Cloth simulation vertices

    DynamicArray<ClothVertex> clothBackupVertices = {}; ///< Backup cloth state

    DynamicArray<ClothSpring> clothSprings        = {}; ///< Spring constraints
};