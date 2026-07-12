#ifndef Z64_ANIMATION_H
#define Z64_ANIMATION_H

#include <libultraship/libultra.h>
#include "z64dma.h"
#include "z64math.h"

struct PlayState;
struct Actor;
struct SkelAnime;

#define LINK_ANIMATION_OFFSET(addr, offset) \
    (((uintptr_t)_link_animetionSegmentRomStart) + ((uintptr_t)addr) - ((uintptr_t)_link_animetionSegmentStart) + ((uintptr_t)offset))
#define LIMB_DONE 0xFF
#define ANIMATION_ENTRY_MAX 50
#define ANIM_FLAG_UPDATEY (1 << 1)
#define ANIM_FLAG_NOMOVE (1 << 4)

#define SKELANIME_TYPE_NORMAL   0
#define SKELANIME_TYPE_FLEX     1
#define SKELANIME_TYPE_CURVE    2

typedef enum {
    /* 0 */ ANIMMODE_LOOP,
    /* 1 */ ANIMMODE_LOOP_INTERP,
    /* 2 */ ANIMMODE_ONCE,
    /* 3 */ ANIMMODE_ONCE_INTERP,
    /* 4 */ ANIMMODE_LOOP_PARTIAL,
    /* 5 */ ANIMMODE_LOOP_PARTIAL_INTERP
} AnimationMode;

typedef enum { 
    /* -1 */ ANIMTAPER_DECEL = -1, 
    /*  0 */ ANIMTAPER_NONE, 
    /*  1 */ ANIMTAPER_ACCEL
} AnimationTapers;

// This flag seems like it was intended to be paired with `ANIM_FLAG_UPDATE_Y` to control
// XZ movement based on the current animation.
// However, this flag is not checked by the Skelanime system. XZ movement will always occur
// regardless of the current state of this flag, as long as the ActorMovement Anim Task is in use.
// The name of this flag is speculative based on its usage in Player and in other actors.
//
// In practice, this flag only affects the scaling of Player's XZ position based on age.
#define ANIM_FLAG_UPDATE_XZ (1 << 0)

// Enables the movement of an actor in the Y-axis based on the current animation.
// This only has an effect if the ActorMovement Anim Task is in use.
//
// This animation-driven movement does not replace "normal" movement from other sources
// such as speed/velocity and collisions. The actor should stop updating other sources of movement
// as required if they are preventing the animation from playing as intended.
// An option is to implement and use `ANIM_FLAG_OVERRIDE_MOVEMENT`.
#define ANIM_FLAG_UPDATE_Y (1 << 1)

// When this flag is set, Player's root limb position adjustment as child is disabled.
// Many of Player's animations are originally created for Adult Link. When playing those
// animations as Child Link without any adjustment, he will appear to be floating in the air.
// To fix this, Child Link's root position is scaled down by default to fit his smaller size.
// However, if an animation is created specifically for Child Link, it is desirable to disable
// this scaling of the root position by using this flag.
// Note that this flag will be ignored if `ANIM_FLAG_UPDATE_XZ` or `ANIM_FLAG_UPDATE_Y` are also
// set. The adjustment will be applied in this case regardless of this flag being enabled.
#define ANIM_FLAG_DISABLE_CHILD_ROOT_ADJUSTMENT (1 << 2)

// When this flag is set, ActorMovement tasks will be queued.
//
// Note that individual actors are responsible for implementing the functionality of this flag.
// In practice, Player is the only actor who implements this flag.
// It is possible to bypass the need for this flag by manually calling `AnimTaskQueue_AddActorMovement`
// when it is needed.
#define ANIM_FLAG_ENABLE_MOVEMENT (1 << 3)

// When this flag is set, movement in all axes will not be applied for one frame. The flag
// is unset automatically after one use, so movement can resume. The intent is for this flag to be used
// when changing between two different animations.
// In some contexts, disabling the first frame of movement is necessary for a seamless transition.
//
// Depending on specific implementations, an actor may choose to reset `prevTransl` to `baseTransl` when
// starting a new animation. This is helpful when an animation's translation data starts at the "origin"
// (in this case, the origin refers to `baseTransl`, in model space).
// Some animations have translation data that does not begin at the "origin". This is common when a
// longer sequence of animation is broken up into different parts as separate animations.
// In this case, when one animation starts its translation at the same position where a different animation
// left off, resetting `prevTransl` is not desirable. This will cause the actor's position to noticeably change
// when the translation data from the first frame of the new animation is applied.
//
// When this flag is used during a transition between two animations, the first frame of movement is not applied.
// This allows the actor's world position to stay at the same location as where the previous animation ended.
// Because translations are calculated as a difference from the current and previous frame, all subsequent
// frames have their translation occur relative to this new starting point.
//
// Note that for Player, this flag is only relevant when transitioning from an animation that was also using
// ActorMovement. This is because of how `prevTransl` gets reset in `Player_StartAnimMovement`.
#define ANIM_FLAG_ADJUST_STARTING_POS (1 << 4)

// Disables "normal" movement from sources like speed/velocity and collisions, which allows the
// animation to have full control over the actor's movement.
//
// Note that individual actors are responsible for implementing the functionality of this flag.
// In practice, Player is the only actor who implements this flag.
#define ANIM_FLAG_OVERRIDE_MOVEMENT (1 << 7)

typedef struct {
    /* 0x00 */ Vec3s jointPos; // Root is position in model space, children are relative to parent
    /* 0x06 */ u8 child;
    /* 0x07 */ u8 sibling;
    /* 0x08 */ Gfx* dList;
} StandardLimb; // size = 0xC

typedef struct {
    /* 0x00 */ Vec3s jointPos; // Root is position in model space, children are relative to parent
    /* 0x06 */ u8 child;
    /* 0x07 */ u8 sibling;
    /* 0x08 */ Gfx* dLists[2]; // Near and far
} LodLimb; // size = 0x10

typedef struct LegacyLimb {
    /* 0x000 */ Gfx* dList;
    /* 0x004 */ Vec3f trans;
    /* 0x010 */ Vec3s rot;
    /* 0x018 */ struct LegacyLimb* sibling;
    /* 0x01C */ struct LegacyLimb* child;
} LegacyLimb; // size = 0x20

// Model has limbs with only rigid meshes
typedef struct {
    /* 0x00 */ void** segment;
    /* 0x04 */ u8 limbCount;
               u8 skeletonType;
} SkeletonHeader; // size = 0x8

// Model has limbs with flexible meshes
typedef struct {
    /* 0x00 */ SkeletonHeader sh;
    /* 0x08 */ u8 dListCount;
} FlexSkeletonHeader; // size = 0xC

// Index into the frame data table. 
typedef struct {
    /* 0x00 */ u16 x;
    /* 0x02 */ u16 y;
    /* 0x04 */ u16 z;
} JointIndex; // size = 0x06

typedef struct {
    /* 0x00 */ s16 frameCount;
} AnimationHeaderCommon;

typedef struct {
    /* 0x00 */ AnimationHeaderCommon common;
    /* 0x04 */ void* segment;
} LinkAnimationHeader; // size = 0x8

typedef struct {
    /* 0x00 */ AnimationHeaderCommon common;
    /* 0x04 */ s16* frameData; // "tbl"
    /* 0x08 */ JointIndex* jointIndices; // "ref_tbl"
    /* 0x0C */ u16 staticIndexMax;
} AnimationHeader; // size = 0x10

// Unused
typedef struct {
    /* 0x00 */ s16 xMax;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 yMax;
    /* 0x06 */ s16 y;
    /* 0x08 */ s16 zMax;
    /* 0x0A */ s16 z;
} JointKey; // size = 0x0C

// Unused
typedef struct {
    /* 0x00 */ s16 frameCount;
    /* 0x02 */ s16 limbCount;
    /* 0x04 */ s16* frameData;
    /* 0x08 */ JointKey* jointKey;
} LegacyAnimationHeader; // size = 0xC

typedef s32 (*OverrideLimbDrawOpa)(struct PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                   void*);

typedef void (*PostLimbDrawOpa)(struct PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void*);

typedef s32 (*OverrideLimbDraw)(struct PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                                void*, Gfx** gfx);

typedef void (*PostLimbDraw)(struct PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void*, Gfx** gfx);

typedef enum {
    ANIMENTRY_LOADFRAME,
    ANIMENTRY_COPYALL,
    ANIMENTRY_INTERP,
    ANIMENTRY_COPYTRUE,
    ANIMENTRY_COPYFALSE,
    ANIMENTRY_MOVEACTOR
} AnimationType;

typedef struct {
    /* 0x000 */ DmaRequest req;
    /* 0x020 */ OSMesgQueue msgQueue;
    /* 0x038 */ OSMesg msg;
} AnimEntryLoadFrame; // size = 0x3C

typedef struct {
    /* 0x000 */ u8 queueFlag;
    /* 0x001 */ u8 vecCount;
    /* 0x004 */ Vec3s* dst;
    /* 0x008 */ Vec3s* src;
} AnimEntryCopyAll; // size = 0xC

typedef struct {
    /* 0x000 */ u8 queueFlag;
    /* 0x001 */ u8 vecCount;
    /* 0x004 */ Vec3s* base;
    /* 0x008 */ Vec3s* mod;
    /* 0x00C */ f32 weight;
} AnimEntryInterp; // size = 0x10

typedef struct {
    /* 0x000 */ u8 queueFlag;
    /* 0x001 */ u8 vecCount;
    /* 0x004 */ Vec3s* dst;
    /* 0x008 */ Vec3s* src;
    /* 0x00C */ u8* copyFlag;
} AnimEntryCopyTrue; // size = 0x10

typedef struct {
    /* 0x000 */ u8 queueFlag;
    /* 0x001 */ u8 vecCount;
    /* 0x004 */ Vec3s* dst;
    /* 0x008 */ Vec3s* src;
    /* 0x00C */ u8* copyFlag;
} AnimEntryCopyFalse; // size = 0x10

typedef struct {
    /* 0x000 */ struct Actor* actor;
    /* 0x004 */ struct SkelAnime* skelAnime;
    /* 0x008 */ f32 unk_08;
} AnimEntryMoveActor; // size = 0xC

typedef union {
    AnimEntryLoadFrame load;
    AnimEntryCopyAll copy;
    AnimEntryInterp interp;
    AnimEntryCopyTrue copy1;
    AnimEntryCopyFalse copy0;
    AnimEntryMoveActor move;
} AnimationEntryData; // size = 0x3C

typedef struct {
    /* 0x00 */ u8 type;
    /* 0x04 */ AnimationEntryData data;
} AnimationEntry; // size = 0x40

typedef struct AnimationContext {
    s16 animationCount;
    AnimationEntry entries[ANIMATION_ENTRY_MAX];
} AnimationContext; // size = 0xC84

typedef void (*AnimationEntryCallback)(struct PlayState* play, AnimationEntryData* data);

// fcurve_skelanime structs
typedef struct {
    /* 0x0000 */ u16 unk_00; // appears to be flags
    /* 0x0002 */ s16 unk_02;
    /* 0x0004 */ s16 unk_04;
    /* 0x0006 */ s16 unk_06;
    /* 0x0008 */ f32 unk_08;
} TransformData; // size = 0xC

typedef struct {
    /* 0x0000 */ u8* refIndex;
    /* 0x0004 */ TransformData* transformData;
    /* 0x0008 */ s16* copyValues;
    /* 0x000C */ s16 unk_0C;
    /* 0x000E */ s16 unk_0E;
} TransformUpdateIndex; // size = 0x10

typedef struct {
    /* 0x0000 */ u8 firstChildIdx;
    /* 0x0001 */ u8 nextLimbIdx;
    /* 0x0004 */ Gfx* dList[2];
} SkelCurveLimb; // size = 0xC

typedef struct {
    /* 0x0000 */ SkelCurveLimb** limbs;
    /* 0x0004 */ u8 limbCount;
} SkelCurveLimbList; // size = 0x8

typedef struct {
    /* 0x0000 */ Vec3s scale;
    /* 0x0006 */ Vec3s rot;
    /* 0x000C */ Vec3s pos;
} LimbTransform; // size = 0x12

typedef struct {
    /* 0x0000 */ u8 limbCount;
    /* 0x0004 */ SkelCurveLimb** limbList;
    /* 0x0008 */ TransformUpdateIndex* transUpdIdx;
    /* 0x000C */ f32 unk_0C; // seems to be unused
    /* 0x0010 */ f32 animFinalFrame;
    /* 0x0014 */ f32 animSpeed;
    /* 0x0018 */ f32 animCurFrame;
    /* 0x001C */ LimbTransform* transforms;
} SkelAnimeCurve; // size = 0x20

typedef s32 (*OverrideCurveLimbDraw)(struct PlayState* play, SkelAnimeCurve* skelCurve, s32 limbIndex, void*);
typedef void (*PostCurveLimbDraw)(struct PlayState* play, SkelAnimeCurve* skelCurve, s32 limbIndex, void*);

typedef s32 (*AnimUpdateFunc)();

typedef struct SkelAnime {
    /* 0x00 */ u8 limbCount; // Number of limbs in the skeleton
    /* 0x01 */ u8 mode; // See `AnimationMode`
    /* 0x02 */ u8 dListCount; // Number of display lists in a flexible skeleton
    /* 0x03 */ s8 taper; // Tapering to use when morphing between animations. Only used by Door_Warp1.
    /* 0x04 */ void** skeleton; // An array of pointers to limbs. Can be StandardLimb, LodLimb, or SkinLimb.
    /* 0x08 */ void* animation; // Can be an AnimationHeader or LinkAnimationHeader.
    /* 0x0C */ f32 startFrame; // In mode ANIMMODE_LOOP_PARTIAL*, start of partial loop.
    /* 0x10 */ f32 endFrame; // In mode ANIMMODE_ONCE*, Update returns true when curFrame is equal to this. In mode ANIMMODE_LOOP_PARTIAL*, end of partial loop.
    /* 0x14 */ f32 animLength; // Total number of frames in the current animation.
    /* 0x18 */ f32 curFrame; // Current frame in the animation
    /* 0x1C */ f32 playSpeed; // Multiplied by R_UPDATE_RATE / 3 to get the animation's frame rate.
    /* 0x20 */ Vec3s* jointTable; // Current translation of model and rotations of all limbs
    /* 0x24 */ Vec3s* morphTable; // Table of values used to morph between animations
    /* 0x28 */ f32 morphWeight; // Weight of the current animation morph as a fraction in [0,1]
    /* 0x2C */ f32 morphRate; // Reciprocal of the number of frames in the morph
    /* 0x30 */ union {
        s32 (*normal)(struct SkelAnime*); // Can be Loop, Partial loop, Play once, Morph, or Tapered morph
        s32 (*link)(struct PlayState*, struct SkelAnime*); // Can be Loop, Play once, or Morph
    } update;
    /* 0x34 */ s8 initFlags; // Flags used when initializing Link's skeleton
    /* 0x35 */ u8 movementFlags; // Flags used for animations that move the actor in worldspace.
    /* 0x36 */ s16 prevRot; // Previous rotation in worldspace.
    /* 0x38 */ Vec3s prevTransl; // Previous modelspace translation.
    /* 0x3E */ Vec3s baseTransl; // Base modelspace translation.
               SkeletonHeader* skeletonHeader;
} SkelAnime; // size = 0x44

#endif
