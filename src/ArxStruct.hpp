#pragma once

#include <array>
#include <cstdint>
#include <d3dtypes.h>


#pragma pack(push, 4)

struct ARX_FTL_PRIMARY_HEADER
{
    std::array<char, 4ULL> ident    = {};

    float version                   = 0.0f;
};

struct ARX_FTL_SECONDARY_HEADER
{
    std::int32_t offset_3Ddata              = 0I32;
    std::int32_t offset_cylinder            = 0I32;
    std::int32_t offset_progressive_data    = 0I32;
    std::int32_t offset_clothes_data        = 0I32;
    std::int32_t offset_collision_spheres   = 0I32;
    std::int32_t offset_physics_box         = 0I32;
};

struct ARX_FTL_PROGRESSIVE_DATA_HEADER
{
    std::int32_t nb_vertex  = 0I32;
};

struct ARX_FTL_CLOTHES_DATA_HEADER
{
    std::int32_t nb_cvert   = 0I32;
    std::int32_t nb_springs = 0I32;
};

struct ARX_FTL_COLLISION_SPHERES_DATA_HEADER
{
    std::int32_t nb_spheres = 0I32;
};

struct ARX_FTL_3D_DATA_HEADER
{
    std::int32_t nb_vertex      = 0I32;
    std::int32_t nb_faces       = 0I32;
    std::int32_t nb_maps        = 0I32;
    std::int32_t nb_groups      = 0I32;
    std::int32_t nb_action      = 0I32;
    std::int32_t nb_selections  = 0I32;
    std::int32_t origin         = 0I32;

    char name[256ULL]           = {};
};

struct EERIE_3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct EERIE_OLD_VERTEX
{
    D3DTLVERTEX vert    = {};
    EERIE_3D v          = {};
    EERIE_3D norm       = {};
};

#define IOPOLYVERT 3

struct EERIE_FACE_FTL
{
    std::int32_t facetype               = 0I32;

    D3DCOLOR rgb[IOPOLYVERT]            = {};
    std::uint16_t vid[IOPOLYVERT]       = {};

    std::int16_t texid                  = 0I16;

    float u[IOPOLYVERT]                 = {};
    float v[IOPOLYVERT]                 = {};

    std::int16_t ou[IOPOLYVERT]         = {};
    std::int16_t ov[IOPOLYVERT]         = {};

    float transval                      = 0.0f;

    EERIE_3D norm                       = {};
    EERIE_3D nrmls[IOPOLYVERT]          = {};

    float temp                          = 0.0f;
};

struct EERIE_GROUPLIST
{
    char name[256ULL]           = {};

    std::int32_t origin         = 0I32;
    std::int32_t nb_index       = 0I32;
    std::int32_t indexes_offset = 0I32;

    float siz                   = 0.0f;
};

struct EERIE_ACTIONLIST
{
    char name[256ULL]   = {};

    std::int32_t idx    = 0I32;
    std::int32_t act    = 0I32;
    std::int32_t sfx    = 0I32;
};

struct EERIE_SELECTIONS
{
    char name[64ULL]                = {};

    std::int32_t nb_selected        = 0I32;
    std::int32_t selected_offset    = 0I32;
};

struct COLLISION_SPHERE
{
    std::int16_t idx    = 0I16;
    std::int16_t flags  = 0I16;

    float radius        = 0.0f;
};

struct PROGRESSIVE_DATA
{
    std::int16_t actual_collapse    = 0I16;
    std::int16_t need_computing     = 0I16;

    float collapse_ratio            = 0.0f;
    float collapse_cost             = 0.0f;

    std::int16_t collapse_candidate = 0I16;
    std::int16_t padd               = 0I16;
};

struct CLOTHESVERTEX
{
    std::int16_t idx    = 0I16;
    std::int8_t flags   = 0I8;
    std::int8_t coll    = 0I8;

    EERIE_3D pos        = {};
    EERIE_3D velocity   = {};
    EERIE_3D force      = {};

    float mass          = 0.0f;

    EERIE_3D t_pos      = {};
    EERIE_3D t_velocity = {};
    EERIE_3D t_force    = {};
    EERIE_3D lastpos    = {};
};

struct EERIE_SPRINGS
{
    std::int16_t startidx   = 0I16;
    std::int16_t endidx     = 0I16;

    float restlength        = 0.0f;
    float constant          = 0.0f;
    float damping           = 0.0f;

    std::int32_t type       = 0I32;
};

#pragma pack(pop)


struct FTL_HEADERS
{
    ARX_FTL_PRIMARY_HEADER                  primary                 = {};
    ARX_FTL_SECONDARY_HEADER                secondary               = {};
    ARX_FTL_PROGRESSIVE_DATA_HEADER         progressiveData         = {};
    ARX_FTL_CLOTHES_DATA_HEADER             clothesData             = {};
    ARX_FTL_COLLISION_SPHERES_DATA_HEADER   collisionSpheresData    = {};
    ARX_FTL_3D_DATA_HEADER                  data3D                  = {};
};

struct FTL_DATA
{
    // 3D
    std::vector<EERIE_OLD_VERTEX>           vertices            = {};
    std::vector<EERIE_FACE_FTL>             faces               = {};
    std::vector<std::string>                textures            = {};
    std::vector<EERIE_GROUPLIST>            groups              = {};
    std::vector<std::vector<std::int32_t>>  groupIndices        = {};
    std::vector<EERIE_ACTIONLIST>           actions             = {};
    std::vector<EERIE_SELECTIONS>           selections          = {};
    std::vector<std::vector<std::int32_t>>  selectionIndices    = {};

    // Collision
    std::vector<COLLISION_SPHERE>           collisionSpheres    = {};

    // Progressive
    std::vector<PROGRESSIVE_DATA>           progressive         = {};

    // Clothes
    std::vector<CLOTHESVERTEX>              clothesVertices     = {};
    std::vector<CLOTHESVERTEX>              clothesBackup       = {};
    std::vector<EERIE_SPRINGS>              springs             = {};
};