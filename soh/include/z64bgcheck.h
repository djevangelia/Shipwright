#ifndef Z_BGCHECK_H
#define Z_BGCHECK_H

struct PlayState;
struct Actor;
struct DynaPolyActor;

#define COLPOLY_NORMAL_FRAC (1.0f / SHT_MAX)
#define COLPOLY_SNORMAL(x) ((s16)((x) * SHT_MAX))
#define COLPOLY_GET_NORMAL(n) ((n)*COLPOLY_NORMAL_FRAC)
#define COLPOLY_VIA_FLAG_TEST(vIA, flags) ((vIA) & (((flags)&7) << 13))
#define COLPOLY_VTX_INDEX(vI) ((vI)&0x1FFF)

#define DYNAPOLY_INVALIDATE_LOOKUP (1 << 0)

#define BGACTOR_NEG_ONE -1
#define BG_ACTOR_MAX 50
#define BGCHECK_SCENE BG_ACTOR_MAX
#define BGCHECK_Y_MIN -32000.0f
#define BGCHECK_XYZ_ABSMAX 32760.0f
#define BGCHECK_SUBDIV_OVERLAP 50
#define BGCHECK_SUBDIV_MIN 150.0f

#define SurfaceType_GetFloorProperty_RESPAWN 5
#define SurfaceType_GetFloorProperty_MOUNT_WALL 6
#define SurfaceType_GetFloorProperty_STOP 8
#define SurfaceType_GetFloorProperty_VOID_OUT 12

#define WATERBOX_ROOM(p) ((p >> 13) & 0x3F)

typedef struct {
    Vec3f scale;
    Vec3s rot;
    Vec3f pos;
} ScaleRotPos;

typedef struct {
    /* 0x00 */ u16 type;
    union {
        u16 vtxData[3];
        struct {
            /* 0x02 */ u16 flags_vIA; // 0xE000 is poly exclusion flags (xpFlags), 0x1FFF is vtxId
            /* 0x04 */ u16 flags_vIB; // 0xE000 is flags, 0x1FFF is vtxId
                                      // 0x2000 = poly IsConveyor surface
            /* 0x06 */ u16 vIC;
        };
    };
    /* 0x08 */ Vec3s normal; // Unit normal vector
                             // Value ranges from -0x7FFF to 0x7FFF, representing -1.0 to 1.0; 0x8000 is invalid

    /* 0x0E */ s16 dist; // Plane distance from origin along the normal
} CollisionPoly; // size = 0x10

typedef struct {
    /* 0x00 */ u16 cameraSType;
    /* 0x02 */ s16 numCameras;
    /* 0x04 */ Vec3s* camPosData;
} CamData;

typedef struct {
    /* 0x00 */ s16 xMin;
    /* 0x02 */ s16 ySurface;
    /* 0x04 */ s16 zMin;
    /* 0x06 */ s16 xLength;
    /* 0x08 */ s16 zLength;
    /* 0x0C */ u32 properties;

    // 0x0008_0000 = ?
    // 0x0007_E000 = Room Index, 0x3F = all rooms
    // 0x0000_1F00 = Lighting Settings Index
    // 0x0000_00FF = CamData index
} WaterBox; // size = 0x10

typedef enum FloorType {
    /*  0 */ FLOOR_TYPE_0,
    /*  1 */ FLOOR_TYPE_1,
    /*  2 */ FLOOR_TYPE_2, // Damage
    /*  3 */ FLOOR_TYPE_3, // Damage
    /*  4 */ FLOOR_TYPE_4, // Shallow sand
    /*  5 */ FLOOR_TYPE_5,
    /*  6 */ FLOOR_TYPE_6, // No fall damage (ex. Gerudo Fortress exterior walking areas)
    /*  7 */ FLOOR_TYPE_7, // Quicksand (Haunted Wasteland ditch)
    /*  8 */ FLOOR_TYPE_8, // Jabu surface
    /*  9 */ FLOOR_TYPE_9,
    /* 10 */ FLOOR_TYPE_10,
    /* 11 */ FLOOR_TYPE_11, // Exit grotto (force look up)
    /* 12 */ FLOOR_TYPE_12 // Quicksand (Haunted Wasteland below bombchu salesman)
} FloorType;

typedef enum WallType {
    /*  0 */ WALL_TYPE_0,
    /*  1 */ WALL_TYPE_1,
    /*  2 */ WALL_TYPE_2,
    /*  3 */ WALL_TYPE_3,
    /*  4 */ WALL_TYPE_4,
    /*  5 */ WALL_TYPE_5,
    /*  6 */ WALL_TYPE_6,
    /*  7 */ WALL_TYPE_7,
    /*  8 */ WALL_TYPE_8,
    /*  9 */ WALL_TYPE_9,
    /* 10 */ WALL_TYPE_10,
    /* 11 */ WALL_TYPE_11,
    /* 12 */ WALL_TYPE_12,
    /* 32 */ WALL_TYPE_MAX = 32
} WallType;

#define WALL_FLAG_0 (1 << 0)
#define WALL_FLAG_1 (1 << 1) // Ladder
#define WALL_FLAG_2 (1 << 2) // Top of ladder
#define WALL_FLAG_3 (1 << 3) // Climbable
#define WALL_FLAG_CRAWLSPACE_1 (1 << 4)
#define WALL_FLAG_CRAWLSPACE_2 (1 << 5)
#define WALL_FLAG_6 (1 << 6) // Grabbable dynapoly
#define WALL_FLAG_CRAWLSPACE (WALL_FLAG_CRAWLSPACE_1 | WALL_FLAG_CRAWLSPACE_2)

typedef enum FloorProperty {
    /*  0 */ FLOOR_PROPERTY_0, // Normal floor property
    /*  5 */ FLOOR_PROPERTY_5 = 5, // Trigger respawn. Used in Dampe's grave, Spirit Temple, DMT.
    /*  6 */ FLOOR_PROPERTY_6, // Force grabbing ledge, no jump or fall. See: Volvagia boss platform edges.
    /*  7 */ FLOOR_PROPERTY_7, // Prevent walking off edges. Only used in Chamber of Sages.
    /*  8 */ FLOOR_PROPERTY_8, // Prevent XZ movement. Used on Spirit Temple statue and in Shooting Gallery + Bombchu Bowling.
    /*  9 */ FLOOR_PROPERTY_9, // Force falling, no jump. However, the wall can be grabbed if climbable (func_8083A6AC). Also set for Hover Boots. See: Pot room mini-walls between windows.
    /* 11 */ FLOOR_PROPERTY_11 = 11, // Able to jumpdive off ledge
    /* 12 */ FLOOR_PROPERTY_12 // Trigger voidout. Used in Haunted Wasteland, Shadow Temple, Ice Cavern among others.
} FloorProperty;

typedef enum SurfaceSfxOffset {
    /*  0 */ SURFACE_SFX_OFFSET_DIRT,
    /*  1 */ SURFACE_SFX_OFFSET_SAND,
    /*  2 */ SURFACE_SFX_OFFSET_STONE,
    /*  3 */ SURFACE_SFX_OFFSET_JABU,
    /*  4 */ SURFACE_SFX_OFFSET_WATER_SHALLOW,
    /*  5 */ SURFACE_SFX_OFFSET_WATER_DEEP,
    /*  6 */ SURFACE_SFX_OFFSET_TALL_GRASS,
    /*  7 */ SURFACE_SFX_OFFSET_LAVA, // MAGMA?
    /*  8 */ SURFACE_SFX_OFFSET_GRASS,
    /*  9 */ SURFACE_SFX_OFFSET_CARPET,
    /* 10 */ SURFACE_SFX_OFFSET_WOOD,
    /* 11 */ SURFACE_SFX_OFFSET_BRIDGE, // WOOD_PLANK?
    /* 12 */ SURFACE_SFX_OFFSET_VINE,
    /* 13 */ SURFACE_SFX_OFFSET_IRON_BOOTS,
    /* 14 */ SURFACE_SFX_OFFSET_UNUSED,
    /* 15 */ SURFACE_SFX_OFFSET_ICE
} SurfaceSfxOffset;

typedef enum SurfaceMaterial {
    /*  0 */ SURFACE_MATERIAL_DIRT,
    /*  1 */ SURFACE_MATERIAL_SAND,
    /*  2 */ SURFACE_MATERIAL_STONE,
    /*  3 */ SURFACE_MATERIAL_JABU,
    /*  4 */ SURFACE_MATERIAL_WATER_SHALLOW,
    /*  5 */ SURFACE_MATERIAL_WATER_DEEP,
    /*  6 */ SURFACE_MATERIAL_TALL_GRASS,
    /*  7 */ SURFACE_MATERIAL_LAVA, // MAGMA?
    /*  8 */ SURFACE_MATERIAL_GRASS,
    /*  9 */ SURFACE_MATERIAL_BRIDGE, // WOOD_PLANK?
    /* 10 */ SURFACE_MATERIAL_WOOD,
    /* 11 */ SURFACE_MATERIAL_DIRT_SOFT,
    /* 12 */ SURFACE_MATERIAL_ICE,
    /* 13 */ SURFACE_MATERIAL_CARPET,
    /* 14 */ SURFACE_MATERIAL_MAX
} SurfaceMaterial;

typedef enum FloorEffect {
    /*  0 */ FLOOR_EFFECT_0,
    /*  1 */ FLOOR_EFFECT_1, // Sliding
    /*  2 */ FLOOR_EFFECT_2 // Transition
} FloorEffect;

typedef enum ConveyorSpeed {
    /*  0 */ CONVEYOR_SPEED_DISABLED,
    /*  1 */ CONVEYOR_SPEED_SLOW,
    /*  2 */ CONVEYOR_SPEED_MEDIUM,
    /*  3 */ CONVEYOR_SPEED_FAST,
    /*  4 */ CONVEYOR_SPEED_MAX
} ConveyorSpeed;

#define CONVEYOR_DIRECTION_TO_BINANG(conveyorDirection) ((conveyorDirection) * (0x10000 / 64))
#define CONVEYOR_DIRECTION_FROM_BINANG(conveyorDirectionBinang) ((conveyorDirectionBinang) / (0x10000 / 64))

#define SURFACETYPE0(bgCamIndex, exitIndex, floorType, unk18, wallType, floorProperty, isSoft, isHorseBlocked) \
    ((((bgCamIndex)     & 0xFF) <<  0) | \
     (((exitIndex)      & 0x1F) <<  8) | \
     (((floorType)      & 0x1F) << 13) | \
     (((unk18)          & 0x07) << 18) | \
     (((wallType)       & 0x1F) << 21) | \
     (((floorProperty)  & 0x0F) << 26) | \
     (((isSoft)         &    1) << 30) | \
     (((isHorseBlocked) &    1) << 31))

#define SURFACETYPE1(material, floorEffect, lightSetting, echo, canHookshot, conveyorSpeed, conveyorDirection, unk27) \
    ((((material)          & 0x0F) <<  0) | \
     (((floorEffect)       & 0x03) <<  4) | \
     (((lightSetting)      & 0x1F) <<  6) | \
     (((echo)              & 0x3F) << 11) | \
     (((canHookshot)       &    1) << 17) | \
     (((conveyorSpeed)     & 0x07) << 18) | \
     (((conveyorDirection) & 0x3F) << 21) | \
     (((unk27)             &    1) << 27))

typedef struct {
    u32 data[2];

    // Type 1
    // 0x0800_0000 = wall damage
} SurfaceType;

typedef struct {
    /* 0x00 */ Vec3s minBounds; // minimum coordinates of poly bounding box
    /* 0x06 */ Vec3s maxBounds; // maximum coordinates of poly bounding box
    /* 0x0C */ u16 numVertices;
    /* 0x10 */ Vec3s* vtxList;
    /* 0x14 */ u16 numPolygons;
    /* 0x18 */ CollisionPoly* polyList;
    /* 0x1C */ SurfaceType* surfaceTypeList;
    /* 0x20 */ CamData* cameraDataList;
    /* 0x24 */ u16 numWaterBoxes;
    /* 0x28 */ WaterBox* waterBoxes;
    size_t cameraDataListLen; // OTRTODO: Added to allow for bounds checking the cameraDataList.
} CollisionHeader; // original name: BGDataInfo

typedef struct {
    s16 polyId;
    u16 next; // next SSNode index
} SSNode;

typedef struct {
    u16 head; // first SSNode index
} SSList;

typedef struct {
    /* 0x00 */ u16 max;          // original name: short_slist_node_size
    /* 0x02 */ u16 count;        // original name: short_slist_node_last_index
    /* 0x04 */ SSNode* tbl;      // original name: short_slist_node_tbl
    /* 0x08 */ u8* polyCheckTbl; // points to an array of bytes, one per static poly. Zero initialized when starting a
                                 // bg check, and set to 1 if that poly has already been tested.
} SSNodeList;

typedef struct {
    SSNode* tbl;
    s32 count;
    s32 max;
} DynaSSNodeList;

typedef struct {
    SSList floor;
    SSList wall;
    SSList ceiling;
} StaticLookup;

typedef struct {
    u16 polyStartIndex;
    SSList ceiling;
    SSList wall;
    SSList floor;
} DynaLookup;

typedef struct {
    /* 0x00 */ struct Actor* actor;
    /* 0x04 */ CollisionHeader* colHeader;
    /* 0x08 */ DynaLookup dynaLookup;
    /* 0x10 */ u16 vtxStartIndex;
    /* 0x14 */ ScaleRotPos prevTransform;
    /* 0x34 */ ScaleRotPos curTransform;
    /* 0x54 */ Sphere16 boundingSphere;
    /* 0x5C */ f32 minY;
    /* 0x60 */ f32 maxY;
} BgActor; // size = 0x64

typedef struct {
    /* 0x0000 */ u8 bitFlag;
    /* 0x0004 */ BgActor bgActors[BG_ACTOR_MAX];
    /* 0x138C */ u16 bgActorFlags[BG_ACTOR_MAX]; // & 0x0008 = no dyna ceiling
    /* 0x13F0 */ CollisionPoly* polyList;
    /* 0x13F4 */ Vec3s* vtxList;
    /* 0x13F8 */ DynaSSNodeList polyNodes;
    /* 0x1404 */ s32 polyNodesMax;
    /* 0x1408 */ s32 polyListMax;
    /* 0x140C */ s32 vtxListMax;
} DynaCollisionContext; // size = 0x1410

typedef struct CollisionContext {
    /* 0x00 */ CollisionHeader* colHeader; // scene's static collision
    /* 0x04 */ Vec3f minBounds;            // minimum coordinates of collision bounding box
    /* 0x10 */ Vec3f maxBounds;            // maximum coordinates of collision bounding box
    /* 0x1C */ Vec3i subdivAmount;         // x, y, z subdivisions of the scene's static collision
    /* 0x28 */ Vec3f subdivLength;         // x, y, z subdivision worldspace lengths
    /* 0x34 */ Vec3f subdivLengthInv;      // inverse of subdivision length
    /* 0x40 */ StaticLookup* lookupTbl;    // 3d array of length subdivAmount
    /* 0x44 */ SSNodeList polyNodes;
    /* 0x50 */ DynaCollisionContext dyna;
    /* 0x1460 */ u32 memSize; // Size of all allocated memory plus CollisionContext
} CollisionContext; // size = 0x1464

typedef struct {
    /* 0x00 */ struct PlayState* play;
    /* 0x04 */ struct CollisionContext* colCtx;
    /* 0x08 */ u16 xpFlags;
    /* 0x0C */ CollisionPoly** resultPoly;
    /* 0x10 */ f32 yIntersect;
    /* 0x14 */ Vec3f* pos;
    /* 0x18 */ s32* bgId;
    /* 0x1C */ struct Actor* actor;
    /* 0x20 */ u32 unk_20;
    /* 0x24 */ f32 chkDist;
    /* 0x28 */ DynaCollisionContext* dyna;
    /* 0x2C */ SSList* ssList;
} DynaRaycast;

typedef struct {
    /* 0x00 */ struct CollisionContext* colCtx;
    /* 0x04 */ u16 xpFlags;
    /* 0x08 */ DynaCollisionContext* dyna;
    /* 0x0C */ SSList* ssList;
    /* 0x10 */ Vec3f* posA;
    /* 0x14 */ Vec3f* posB;
    /* 0x18 */ Vec3f* posResult;
    /* 0x1C */ CollisionPoly** resultPoly;
    /* 0x20 */ s32 chkOneFace; // bccFlags & 0x8
    /* 0x24 */ f32* distSq;    // distance from posA to poly squared
    /* 0x28 */ f32 chkDist;    // distance from poly
} DynaLineTest;

#endif
