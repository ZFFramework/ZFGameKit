/**
 * @file P2World.h
 * @brief 2d physics
 */

#ifndef _ZFI_P2World_h_
#define _ZFI_P2World_h_

#include "ZFGameKitDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

zfclassFwd P2Body;
zfclassFwd P2World;

/** @brief a dummy value indicates largest value in physics world */
inline zffloat P2_MAX(void) {
    return 999999;
}

/** @brief see #P2World */
#define P2FilterMaskAll() ((zfflags)-1)
/** @brief see #P2World */
#define P2FilterCategoryDefault() ((zfflags)0x80000000)
/** @brief see #P2World */
#define P2FilterMaskDefault() P2FilterMaskAll()

/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZFGameKit, zfflags, P2FilterMaskAll)
/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZFGameKit, zfflags, P2FilterCategoryDefault)
/** @brief see #P2World */
ZFEXPORT_VAR_READONLY_DECLARE(ZFLIB_ZFGameKit, zfflags, P2FilterMaskDefault)

// ============================================================
zfclassFwd _ZFP_P2ShapePrivate;
/** @brief see #P2World */
zfabstract ZFLIB_ZFGameKit P2Shape : zfextend ZFStyle {
    ZFOBJECT_DECLARE_ABSTRACT(P2Shape, ZFStyle)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_shapeId)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_density, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_density)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sensor)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sensorEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_sensorEnable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_contactEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_contactEnable)

    /** @brief see #P2World, what category this shape is */
    ZFPROPERTY_ASSIGN(zfflags, p2_filterCategory, P2FilterCategoryDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zfflags, p2_filterCategory)
    /** @brief see #P2World, what type should be contact with this shape */
    ZFPROPERTY_ASSIGN(zfflags, p2_filterMask, P2FilterMaskDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zfflags, p2_filterMask)
    /** @brief see #P2World, always contact (>0) or never contact (<0) within same group */
    ZFPROPERTY_ASSIGN(zfint, p2_filterGroup, 0)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfint, p2_filterGroup)

    /** @brief see #P2World, range in [0, 1] */
    ZFPROPERTY_ASSIGN(zffloat, p2_matFriction, 0.6f)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matFriction)
    /** @brief see #P2World, range in [0, 1], how much to bounce when contact */
    ZFPROPERTY_ASSIGN(zffloat, p2_matBounce, 0.05f)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matBounce)
    /** @brief see #P2World, range in [0, 1] */
    ZFPROPERTY_ASSIGN(zffloat, p2_matRotationResist)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matRotationResist)
    /**
     * @brief see #P2World
     *
     * -  0 : normal plain ground, won't be pushed by friction
     * -  [0, 1] : slightly pushed, e.g wind
     * -  [1, 10] : conveyor belt, from low to high speed
     * -  <0 : reverse
     */
    ZFPROPERTY_ASSIGN(zffloat, p2_matTangentSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_matTangentSpeed)

public:
    /**
     * @brief see #P2World, get world AABB, valid only after added to world,
     *   already applied with #P2Body::p2_bodyScale and all parent transforms
     */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABB)
    /**
     * @brief see #P2World, get local AABB, according to current property only,
     *   not applied with #P2Body::p2_bodyScale
     */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABBLocal)

public:
    /** @brief see #P2World, whether impl has attached to world */
    ZFMETHOD_DECLARE_0(zfbool, p2_implAttached)

protected:
    /** @brief for impl only */
    virtual ZFUIRect p2impl_AABBLocal(void) zfpurevirtual;
    /** @brief for impl only */
    virtual inline void p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale) {}
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void objectInfoImpl(ZF_IN_OUT zfstring &ret) {
        ZFObjectPrettyInfoT(ret, this);
    }

public:
    _ZFP_P2ShapePrivate *_ZFP_P2Shape_d;
    friend zfclassFwd _ZFP_P2ShapePrivate;
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2ShapeBox : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeBox, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUISize, p2_size, ZFUISizeCreate(1))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUISize, p2_size)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotation)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotation)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_radius)
protected:
    zfoverride
    virtual ZFUIRect p2impl_AABBLocal(void);
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale);
};
/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2ShapeCircle : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeCircle, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_radius)
protected:
    zfoverride
    virtual ZFUIRect p2impl_AABBLocal(void);
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale);
};
/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2ShapeCapsule : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapeCapsule, P2Shape)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position1)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position1)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_radius, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_radius)
protected:
    zfoverride
    virtual ZFUIRect p2impl_AABBLocal(void);
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale);
};
/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2ShapePolygon : zfextend P2Shape {
    ZFOBJECT_DECLARE(P2ShapePolygon, P2Shape)
public:
    /** @brief max vertex count of a #P2ShapePolygon */
    ZFMETHOD_DECLARE_STATIC_0(zfindex, p2_maxVertex)
    /**
     * @brief create polygon from points
     *
     * points must:
     * -  count must not exceeds #p2_maxVertex
     * -  ordered with clockwise
     * -  convex hull
     */
    ZFMETHOD_DECLARE_4(void, p2_polygon
            , ZFMP_IN(const ZFCoreArray<ZFUIPoint> &, points)
            , ZFMP_IN_OPT(const ZFUIPoint &, position, ZFUIPointZero())
            , ZFMP_IN_OPT(zffloat, rotation, 0)
            , ZFMP_IN_OPT(zffloat, radius, 0)
            )
protected:
    zfoverride
    virtual ZFUIRect p2impl_AABBLocal(void);
    zfoverride
    virtual void p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale);

public:
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(ZFCoreArray<ZFUIPoint>, p2_polygon_points)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_polygon_position)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(zffloat, p2_polygon_rotation)
    /** @brief use #p2_polygon to update, do not access manually */
    ZFPROPERTY_ASSIGN(zffloat, p2_polygon_radius)
};

// ============================================================
zfclassFwd _ZFP_P2JointPrivate;
/** @brief see #P2World */
zfabstract ZFLIB_ZFGameKit P2Joint : zfextend ZFStyle {
    ZFOBJECT_DECLARE_ABSTRACT(P2Joint, ZFStyle)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody0)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody1)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /**@brief see #P2World, joint's owner, maybe #P2World or #P2Body */
    ZFMETHOD_DECLARE_0(ZFObject *, p2_owner)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_removeFromParent)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_jointId)

    /** @brief see #P2World::p2_bodyFind, #P2Body::p2_bodyFind */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId0)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfstring, p2_bodyId0)
    /** @brief see #P2World::p2_bodyFind, #P2Body::p2_bodyFind */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfstring, p2_bodyId1)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_contactEnable)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_contactEnable)

public:
    /** @brief see #P2World, whether impl has attached to world */
    ZFMETHOD_DECLARE_0(zfbool, p2_implAttached)

protected:
    /** @brief init with body id */
    ZFOBJECT_ON_INIT_DECLARE_2(
            ZFMP_IN(const zfstring &, bodyId0)
            , ZFMP_IN(const zfstring &, bodyId1)
            )
protected:
    /** @brief for impl only */
    virtual inline void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {}
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void objectInfoImpl(ZF_IN_OUT zfstring &ret) {
        ZFObjectPrettyInfoT(ret, this);
    }

public:
    _ZFP_P2JointPrivate *_ZFP_P2Joint_d;
    friend zfclassFwd _ZFP_P2JointPrivate;
};

/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringHertzSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringDampingSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringHertzNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringDampingNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringHertzHard)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointSpringDampingHard)

/** @brief see #P2World */
zfabstract ZFLIB_ZFGameKit P2JointSpring : zfextend P2Joint {
    ZFOBJECT_DECLARE_ABSTRACT(P2JointSpring, P2Joint)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor1)

    /** @brief see #P2World, usually [0, 60], how many times to update spring for each second */
    ZFPROPERTY_ASSIGN(zffloat, p2_springHertz, 0)
    /** @brief see #P2World, [0, 1], from soft to hard, how effective the spring is */
    ZFPROPERTY_ASSIGN(zffloat, p2_springDamping, 0)

    /**
     * @brief see #P2World
     *
     * -  for positional motor: `N m/s`
     * -  for rotational motor: `N degree/s`
     */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorSpeed)
    /** @brief see #P2World, usually [50, 1000] */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce)
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointDistance : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointDistance, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distance, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distance)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_distanceCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin, -P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax, P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointRevolute : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointRevolute, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angular)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angular)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_angularCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularLimitMin, -P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularLimitMax, P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointPrismatic : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointPrismatic, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_axis, ZFUIPointCreate(1, 0))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_axis)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distance)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distance)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_distanceCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin, -P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax, P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointWheel : zfextend P2JointSpring {
    ZFOBJECT_DECLARE(P2JointWheel, P2JointSpring)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_axis)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_axis)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMin, -P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMin)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceLimitMax, P2_MAX())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceLimitMax)

    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_springDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorSpeed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointMotorFactorSoft)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointMotorFactorNormal)
/** @brief see #P2World */
ZFEXPORT_VAR_DECLARE(ZFLIB_ZFGameKit, zffloat, P2JointMotorFactorHard)

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointMotor : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointMotor, P2Joint)
public:
    /** @brief see #P2World, usually (0, 1), how hard the motor take effective */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorFactor, P2JointMotorFactorNormal())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorFactor)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce, 100)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorTorque, 100)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorTorque)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_distance)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_distance)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_distanceCur)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angular)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angular)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_angularCur)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointWeld : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointWeld, P2Joint)
public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor0)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor0)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_anchor1)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_anchor1)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularRef)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularRef)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceDamping)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularHertz)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularDamping)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2JointFilter : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointFilter, P2Joint)
protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

/**
 * @brief see #P2World
 *
 * note:
 * -  the body needs to be drag by mouse should be set to #P2Joint::p2_bodyId1
 * -  #P2Joint::p2_bodyId0 usually should be set to ground or a dummy static body
 */
zfclass ZFLIB_ZFGameKit P2JointMouse : zfextend P2Joint {
    ZFOBJECT_DECLARE(P2JointMouse, P2Joint)

public:
    /** @brief see #P2World, mouse position relative to world */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce, 1000)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

protected:
    zfoverride
    virtual void p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1);
};

// ============================================================
zfclassFwd _ZFP_P2ExtPrivate;
/** @brief see #P2Body::p2_ext */
zfclass ZFLIB_ZFGameKit P2Ext : zfextend ZFStyle {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(P2Ext, ZFStyle)

public:
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when attached to #P2Body::p2_ext,
     * param0 is the owner #P2Body
     */
    ZFEVENT(P2ExtAttach)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when detached from #P2Body::p2_ext,
     * param0 is the owner #P2Body
     */
    ZFEVENT(P2ExtDetach)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #P2Body::E_P2Update and this ext has attached,
     * param0 is the owner #P2Body
     */
    ZFEVENT(P2ExtUpdate)

public:
    /** @brief see #P2Body::p2_ext */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody)

protected:
    /** @brief see #P2Ext::E_P2ExtAttach */
    virtual void p2_extAttach(ZF_IN P2Body *owner, ZF_IN_OUT zfbool &listenOnUpdate) {
    }
    /** @brief see #P2Ext::E_P2ExtDetach */
    virtual void p2_extDetach(ZF_IN P2Body *owner) {
    }
    /**
     * @brief see #p2_extAttach, only called when listenOnUpdate set to true,
     *   or #P2Ext::E_P2ExtUpdate observer attached
     */
    virtual void p2_extUpdate(ZF_IN P2Body *owner) {
    }

    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId);
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId);

private:
    friend zfclassFwd _ZFP_P2ExtPrivate;
    P2Body *_ZFP_ownerBody;
protected:
    /** @cond ZFPrivateDoc */
    P2Ext(void) : _ZFP_ownerBody(zfnull) {}
    /** @endcond */
};

// ============================================================
/** @brief see #P2World */
ZFENUM_BEGIN(ZFLIB_ZFGameKit, P2BodyType)
    ZFENUM_VALUE(Static) /**< @brief see #P2World */
    ZFENUM_VALUE(Kinematic) /**< @brief see #P2World */
    ZFENUM_VALUE(Dynamic) /**< @brief see #P2World */
ZFENUM_SEPARATOR()
    ZFENUM_VALUE_REGISTER(Static)
    ZFENUM_VALUE_REGISTER(Kinematic)
    ZFENUM_VALUE_REGISTER(Dynamic)
ZFENUM_END(ZFLIB_ZFGameKit, P2BodyType)
ZFENUM_REG(ZFLIB_ZFGameKit, P2BodyType)

/** @brief see #P2Body::p2_toLocal */
zfclassPOD ZFLIB_ZFGameKit P2BodyTransform {
public:
    ZFUIPoint p2_position; /**< @brief see #P2Body::p2_toLocal */
    zffloat p2_rotation /**< @brief see #P2Body::p2_toLocal */;
    zffloat p2_bodyScale; /**< @brief see #P2Body::p2_toLocal */
};
ZFTYPEID_DECLARE(ZFLIB_ZFGameKit, P2BodyTransform, P2BodyTransform)
ZFTYPEID_REG(ZFLIB_ZFGameKit, P2BodyTransform, P2BodyTransform)
ZFOUTPUT_TYPE(P2BodyTransform, {P2BodyTransformToStringT(s, v);})
ZFCORE_POD_DECLARE(P2BodyTransform)

/** @brief convert transform from global to local, transforms ordered from specified body to root */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, void, P2BodyTransformToLocal
        , ZFMP_OUT(P2BodyTransform &, transformLocal)
        , ZFMP_IN(const P2BodyTransform &, transformGlobal)
        , ZFMP_IN(const ZFCoreArray<P2BodyTransform> &, transforms)
        )
/** @brief convert transform from local to global, transforms ordered from specified body to root */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, void, P2BodyTransformToGlobal
        , ZFMP_OUT(P2BodyTransform &, transformGlobal)
        , ZFMP_IN(const P2BodyTransform &, transformLocal)
        , ZFMP_IN(const ZFCoreArray<P2BodyTransform> &, transforms)
        )

/** @brief convert body's origin's position to body's mass center's position */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, ZFUIPoint, P2BodyMassPosFromOriginPos
        , ZFMP_IN(const ZFUIPoint &, originPos)
        , ZFMP_IN(const ZFUIPoint &, massCenter)
        , ZFMP_IN(zffloat, rotation)
        )
/** @brief convert body's mass center's position to body's origin's position */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, ZFUIPoint, P2BodyMassPosToOriginPos
        , ZFMP_IN(const ZFUIPoint &, massPos)
        , ZFMP_IN(const ZFUIPoint &, massCenter)
        , ZFMP_IN(zffloat, rotation)
        )

zfclassFwd _ZFP_P2BodyPrivate;
/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2Body : zfextend ZFStyle {
    ZFOBJECT_DECLARE(P2Body, ZFStyle)

public:
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when root body visibility changed by #P2World::p2_UIVisibleArea,
     * current visibility can be checked by #p2_bodyVisible
     */
    ZFEVENT(P2VisibilityUpdate)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2SensorEvent::p2_sensorEnterList,
     * and this body is the sensor\n
     * param0 is the visitor #P2Body
     */
    ZFEVENT(P2SensorEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2SensorEvent::p2_sensorExitList,
     * and this body is the sensor\n
     * param0 is the visitor #P2Body
     */
    ZFEVENT(P2SensorExit)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2SensorEvent::p2_sensorEnterList,
     * and this body is the visitor\n
     * param0 is the sensor #P2Body
     */
    ZFEVENT(P2SensorVisitorEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2SensorEvent::p2_sensorExitList,
     * and this body is the visitor\n
     * param0 is the sensor #P2Body
     */
    ZFEVENT(P2SensorVisitorExit)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called before contact, to filter and prevent further contact,
     * param0 is the other #P2Body,
     * param1 is the #P2ContactEventData
     */
    ZFEVENT(P2ContactFilter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2ContactEvent::p2_contactEnterList\n
     * param0 is the other #P2Body,
     * param1 is the #P2ContactEventData
     */
    ZFEVENT(P2ContactEnter)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape of this body got #P2ContactEvent::p2_contactExitList\n
     * param0 is the other #P2Body,
     * param1 is the #P2ContactEventData
     */
    ZFEVENT(P2ContactExit)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called during each simulation step, take care of performance
     */
    ZFEVENT(P2Update)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2Body *, p2_ownerBody)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(P2World *, p2_ownerWorld)
    /** @brief see #P2World, body's owner, maybe #P2World or #P2Body */
    ZFMETHOD_DECLARE_0(ZFObject *, p2_owner)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_removeFromParent)

public:
    /** @brief see #P2World, must be unique within same #P2World */
    ZFPROPERTY_ASSIGN(zfstring, p2_bodyId)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfstring, p2_bodyId)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(P2BodyType, p2_type)
    ZFPROPERTY_ON_UPDATE_DECLARE(P2BodyType, p2_type)

    /** @brief see #p2_toLocal, note changing scale is heavy task */
    ZFPROPERTY_ASSIGN(zffloat, p2_bodyScale, 1)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_bodyScale)
    /** @brief see #p2_toLocal, all of #p2_ownerBody's #p2_bodyScale, excluding this body */
    ZFMETHOD_DECLARE_0(zffloat, p2_bodyScaleInherit)
    /** @brief see #p2_toLocal, this body and all of #p2_ownerBody's #p2_bodyScale */
    ZFMETHOD_DECLARE_0(zffloat, p2_bodyScaleFixed)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_enable, zftrue)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_enable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_sleepEnable, zftrue)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_sleepEnable)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zfbool, p2_bullet)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_bullet)
    /** @brief see #P2World, whether keep this body's rotation */
    ZFPROPERTY_ASSIGN(zfbool, p2_rotationFixed)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfbool, p2_rotationFixed)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_gravityScale, 1.0f)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_gravityScale)

    /** @brief see #p2_toLocal, body's origin's position relative to parent */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_position)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_position)
    /** @brief see #p2_toLocal */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_positionVelocity)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_positionVelocity)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_positionDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_positionDamping)

    /** @brief see #p2_toLocal, range in [0, 360], note the rotation would around the body's origin */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotation)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotation)
    /** @brief see #p2_toLocal, note the rotation velocity would around the body's mass center */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotationVelocity)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotationVelocity)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_rotationDamping)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_rotationDamping)

public:
    /**
     * @brief see #P2World, world AABB, note: calculate each time called,
     *   already applied with #P2Body::p2_bodyScale and all parent transforms
     */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABB)
    /**
     * @brief see #P2World, local AABB, calculate by each shape's #P2Shape::p2_AABBLocal,
     *   not applied with #P2Body::p2_bodyScale
     *
     * origin point position would be (0-p2_AABBLocal.x, 0-p2_AABBLocal.y), relative to p2_AABBLocal's left bottom
     *
     * note: valid only after added to world
     */
    ZFMETHOD_DECLARE_0(ZFUIRect, p2_AABBLocal)
    /**
     * @brief see #P2World, relative to origin of #p2_AABBLocal, calculate and cached automatically
     *
     * not applied with #P2Body::p2_bodyScale
     *
     * note: valid only after added to world,
     * and only for dynamic body
     */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_massCenter)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zffloat, p2_mass)
    /**
     * @brief see #P2World
     *
     * value to descript how the body is easy to rotate:
     * -  0 : can not rotate on force
     * -  (0, 10] : smaller value means more easier to rotate
     */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationInertia)

    /** @brief see #p2_massCenter, the body's mass center's position in world */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_massPositionCur)
    /** @brief see #p2_massPositionCur, do nothing if body impl not created */
    ZFMETHOD_DECLARE_1(void, p2_massPositionCur
            , ZFMP_IN(const ZFUIPoint &, v)
            )

    /** @brief see #p2_toLocal, the body's origin's position in world */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_positionCur)
    /** @brief see #p2_toLocal, do nothing if body impl not created */
    ZFMETHOD_DECLARE_1(void, p2_positionCur
            , ZFMP_IN(const ZFUIPoint &, v)
            )
    /** @brief see #p2_toLocal */
    ZFMETHOD_DECLARE_0(ZFUIPoint, p2_positionVelocityCur)
    /** @brief see #p2_toLocal, do nothing if body impl not created */
    ZFMETHOD_DECLARE_1(void, p2_positionVelocityCur
            , ZFMP_IN(const ZFUIPoint &, v)
            )

    /** @brief see #p2_toLocal, note the rotation would around the body's origin */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationCur)
    /** @brief see #p2_toLocal, do nothing if body impl not created */
    ZFMETHOD_DECLARE_1(void, p2_rotationCur
            , ZFMP_IN(zffloat, v)
            )
    /** @brief see #p2_toLocal */
    ZFMETHOD_DECLARE_0(zffloat, p2_rotationVelocityCur)
    /** @brief see #p2_toLocal, do nothing if body impl not created */
    ZFMETHOD_DECLARE_1(void, p2_rotationVelocityCur
            , ZFMP_IN(zffloat, v)
            )

    /**
     * @brief util to update #p2_position and #p2_rotation with mass center position
     *
     * #p2_position and #p2_rotation is relative to body's origin,
     * which is not convenient to build complex body tree,
     * you may use this method to make things easier\n
     * \n
     * note #p2_shape and #p2_bodyScale should be prepared before calling this method,
     * since it depends on #p2_massCenter\n
     * \n
     * massPos and rotation can be #P2_MAX:
     * -  when both are #P2_MAX, do nothing
     * -  when massPos is #P2_MAX, update rotation but keep current mass center
     * -  when rotation is #P2_MAX, update mass center but keep current rotation
     */
    ZFMETHOD_DECLARE_2(void, p2_pos
            , ZFMP_IN(const ZFUIPoint &, massPos)
            , ZFMP_IN_OPT(zffloat, rotation, P2_MAX())
            )

    /**
     * @brief transform from global to local of this body
     *
     * #P2Body can holds child body, and child body would be applied with parent body's transform
     * when adding to physics world,
     * you may use #p2_toLocal and #p2_toGlobal to calculate transform
     *
     * -  methods with local transform:
     *   - #p2_position
     *   - #p2_rotation
     *   - #p2_bodyScale
     *   - #p2_massCenter
     * -  methods with global transform:
     *   - #p2_positionCur
     *   - #p2_positionVelocityCur
     *   - #p2_rotationCur
     *   - #p2_rotationVelocityCur
     *   - #p2_bodyScaleFixed
     *   - #p2_massPositionCur
     */
    ZFMETHOD_DECLARE_2(void, p2_toLocal
            , ZFMP_OUT(P2BodyTransform &, transformLocal)
            , ZFMP_IN(const P2BodyTransform &, transformGlobal)
            )
    /** @brief see #p2_toLocal */
    ZFMETHOD_DECLARE_2(void, p2_toGlobal
            , ZFMP_OUT(P2BodyTransform &, transformGlobal)
            , ZFMP_IN(const P2BodyTransform &, transformLocal)
            )
    /**
     * @brief util method to obtain all parents' transforms,
     *   including this body, ordered from this to root
     *
     * when no parent, only one element would return, which contains this body's transform
     */
    ZFMETHOD_DECLARE_0(ZFCoreArray<P2BodyTransform>, p2_transforms)

    /**
     * @brief see #P2World::p2_UIVisibleArea
     *
     * note: valid only for root body (bodies that added directly to #P2World::p2_body)
     */
    ZFMETHOD_DECLARE_0(zfbool, p2_bodyVisible)

public:
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfbool, p2_sleeping)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_sleeping
            , ZFMP_IN(zfbool, v)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_3(void, p2_applyPositionForceInWorld
            , ZFMP_IN(const ZFUIPoint &, force)
            , ZFMP_IN(const ZFUIPoint &, worldPosition)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyPositionForce
            , ZFMP_IN(const ZFUIPoint &, force)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyRotationForce
            , ZFMP_IN(zffloat, force)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_3(void, p2_applyPositionImpulseInWorld
            , ZFMP_IN(const ZFUIPoint &, impulse)
            , ZFMP_IN(const ZFUIPoint &, worldPosition)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyPositionImpulse
            , ZFMP_IN(const ZFUIPoint &, impulse)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_2(void, p2_applyRotationImpulse
            , ZFMP_IN(zffloat, impulse)
            , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
            )

public:
    // ============================================================
    // shape
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_shapeList, zfobj<ZFArray>())
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_shape
            , ZFMP_IN(P2Shape *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_shapeCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Shape>, p2_shapeRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Shape>, p2_shapeRemove
            , ZFMP_IN(P2Shape *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_shapeRemoveAll)

    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #p2_bodyList, breadth first, including this body\n
     * shapeId can be:
     * -  exact shapeId
     * -  chained bodyId and shapeId: `bodyId1.bodyId3...shapeId`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeFind
            , ZFMP_IN(const zfstring &, shapeId)
            )

    // ============================================================
    // child body
public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_bodyList, zfobj<ZFArray>())

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_body
            , ZFMP_IN(P2Body *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_bodyCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_bodyRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_bodyRemove
            , ZFMP_IN(P2Body *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_bodyRemoveAll)
    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #p2_bodyList, breadth first, including this body\n
     * bodyId can be:
     * -  exact bodyId
     * -  chained bodyId: `bodyId1.bodyId3...`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyFind
            , ZFMP_IN(const zfstring &, bodyId)
            )

    // ============================================================
    // body joint
public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_jointList, zfobj<ZFArray>())

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_joint
            , ZFMP_IN(P2Joint *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_jointCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemove
            , ZFMP_IN(P2Joint *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_jointRemoveAll)
    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #p2_jointList and all of #p2_bodyList, breadth first\n
     * jointId can be:
     * -  exact jointId
     * -  chained bodyId and jointId: `bodyId1.bodyId3...jointId`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointFind
            , ZFMP_IN(const zfstring &, jointId)
            )

    /**
     * @brief see #P2World
     *
     * all joints that referenced this body,
     * including joints in #P2Body::p2_jointList and #P2World::p2_jointList
     * must not manually modify,
     * valid only after added to world and first time updated
     */
    ZFMETHOD_DECLARE_0(ZFArray *, p2_refJointList)

    // ============================================================
    // body ext
public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_extList, zfobj<ZFArray>())

    /**
     * @brief attach extension for this body
     */
    ZFMETHOD_DECLARE_1(void, p2_ext
            , ZFMP_IN(P2Ext *, v)
            )
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_0(zfindex, p2_extCount)
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_1(P2Ext *, p2_extAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_1(zfautoT<P2Ext>, p2_extRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_1(zfautoT<P2Ext>, p2_extRemove
            , ZFMP_IN(P2Ext *, v)
            )
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_0(void, p2_extRemoveAll)
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_1(zfindex, p2_extCheck
            , ZFMP_IN(const ZFClass *, extCls)
            )
    /** @brief see #p2_ext */
    ZFMETHOD_DECLARE_1(P2Ext *, p2_extAccess
            , ZFMP_IN(const ZFClass *, extCls)
            )

public:
    /** @brief see #P2World, whether impl has attached to world */
    ZFMETHOD_DECLARE_0(zfbool, p2_implAttached)

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId);
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId);

public:
    _ZFP_P2BodyPrivate *_ZFP_P2Body_d;
};

// ============================================================
/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2BodyVisibilityEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2BodyVisibilityEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2Body *> p2_bodyEnterList;
    /** @brief see #P2World */
    ZFCoreArray<P2Body *> p2_bodyExitList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZFGameKit P2BodyMoveEventData {
public:
    /** @brief see #P2World */
    P2Body *p2_body;
    /** @brief see #P2World */
    ZFUIPoint p2_position;
    /** @brief see #P2World */
    zffloat p2_rotation;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZFGameKit, P2BodyMoveEventData, P2BodyMoveEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZFGameKit, P2BodyMoveEventData, P2BodyMoveEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZFGameKit, P2BodyMoveEventData)
ZFCORE_POD_DECLARE(P2BodyMoveEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2BodyMoveEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2BodyMoveEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2BodyMoveEventData> p2_moveEventList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZFGameKit P2SensorEventData {
public:
    /** @brief the sensor shape */
    P2Shape *p2_shape0;
    /** @brief the visitor shape */
    P2Shape *p2_shape1;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZFGameKit, P2SensorEventData, P2SensorEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZFGameKit, P2SensorEventData, P2SensorEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZFGameKit, P2SensorEventData)
ZFCORE_POD_DECLARE(P2SensorEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2SensorEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2SensorEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2SensorEventData> p2_sensorEnterList;
    /** @brief see #P2World */
    ZFCoreArray<P2SensorEventData> p2_sensorExitList;
};

/** @brief see #P2World */
zfclassPOD ZFLIB_ZFGameKit P2ContactEventData {
public:
    /** @brief the shape */
    P2Shape *p2_shape0;
    /** @brief the shape */
    P2Shape *p2_shape1;

    /**
     * @brief contact normal vector from p2_shape0 to p2_shape1
     *
     * for #P2ContactEvent::p2_contactEnterList and #P2Body::E_P2ContactFilter only
     */
    ZFUIPoint p2_normal;

    /**
     * @brief whether enable contact, true by default
     *
     * for #P2Body::E_P2ContactFilter only
     */
    zfbool p2_contactEnable;
};
ZFTYPEID_ACCESS_ONLY_DECLARE(ZFLIB_ZFGameKit, P2ContactEventData, P2ContactEventData)
ZFTYPEID_ACCESS_ONLY_REG(ZFLIB_ZFGameKit, P2ContactEventData, P2ContactEventData)
ZFOUTPUT_TYPE_DECLARE(ZFLIB_ZFGameKit, P2ContactEventData)
ZFCORE_POD_DECLARE(P2ContactEventData)

/** @brief see #P2World */
zfclass ZFLIB_ZFGameKit P2ContactEvent : zfextend ZFObject {
    ZFOBJECT_DECLARE(P2ContactEvent, ZFObject)
public:
    /** @brief see #P2World */
    ZFCoreArray<P2ContactEventData> p2_contactEnterList;
    /** @brief see #P2World */
    ZFCoreArray<P2ContactEventData> p2_contactExitList;
};

// ============================================================
/** @brief see #P2World */
zfclassNotPOD P2WorldImpl {
public:
    virtual ~P2WorldImpl(void) {}
public:
    /** @brief called when body added to world, before #bodyMoveEvent */
    virtual void bodyAdd(ZF_IN P2World *world, ZF_IN P2Body *body) zfpurevirtual;
    /** @brief called when body removed from world */
    virtual void bodyRemove(ZF_IN P2World *world, ZF_IN P2Body *body) zfpurevirtual;
    /** @brief called to update body position */
    virtual void bodyMoveEvent(ZF_IN P2World *world, ZF_IN P2BodyMoveEvent *event) zfpurevirtual;
    /**
     * @brief called when #P2World::p2_UIOffset or #P2World::p2_UISize or #P2World::p2_UIScale changed,
     *   called only once after current simulation step
     */
    virtual void UIUpdate(ZF_IN P2World *world) zfpurevirtual;
};

zfclassFwd _ZFP_P2WorldPrivate;
/**
 * @brief base physics world
 *
 * note: this module is more or less a packaging of box2d,
 * you should be familiar with box2d before using this module\n
 *
 * main classes:
 * -  #P2World : manage the physics world
 * -  #P2Body : base physics body, one body usually should have one or more shapes,
 *   a body can have child bodies and joints,
 *   child bodies' position and rotation would be relative to its parent body
 * -  #P2Shape : base shape
 * -  #P2Joint : joint to connect two bodies, it can:
 *   belong to #P2World (to control two bodies in world which not belongs to same root body),
 *   or belong to #P2Body (to describe connection within the same root body)
 *
 * you can use #P2World in two different way:
 * -  create a #P2World, then implement #P2World::p2impl,
 *   to implement render and body management
 * -  use #P2WorldView,
 *   and use any #ZFUIView type to act as #P2Body
 *   (#ZFUIView already marked as `ZFCLASS_EXTEND(ZFUIView, P2Body)`),
 *   and the #P2WorldView would help you to manage your views as #P2Body
 *
 * NOTE:
 * -  for #P2World, origin point is at left down zero point,
 *   and rotation is clockwise
 * -  for #ZFUIView, origin point is at left top zero point,
 *   and rotation is clockwise
 * -  when using #P2WorldView,
 *   you should always use methods declared in #P2World (all method name starts with `p2_`)
 *   to manage children/body/shapes
 */
zfclass ZFLIB_ZFGameKit P2World : zfextend ZFStyle {
    ZFOBJECT_DECLARE(P2World, ZFStyle)

public:
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #P2Body::p2_implAttached attached, param0 is the #P2Body
     */
    ZFEVENT(P2BodyAttach)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #P2Body::p2_implAttached detached, param0 is the #P2Body
     */
    ZFEVENT(P2BodyDetach)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #P2Joint::p2_implAttached attached, param0 is the #P2Joint
     */
    ZFEVENT(P2JointAttach)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #P2Joint::p2_implAttached detached, param0 is the #P2Joint
     */
    ZFEVENT(P2JointDetach)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called before each simulation step
     */
    ZFEVENT(P2UpdatePrev)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called before each simulation step
     */
    ZFEVENT(P2UpdatePost)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any root body's visibility changed during each simulation step,
     * param0 is #P2BodyVisibilityEvent
     */
    ZFEVENT(P2BodyVisibilityEvent)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any body moved during each simulation step,
     * param0 is #P2BodyMoveEvent
     */
    ZFEVENT(P2BodyMoveEvent)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape fired sensor event during each simulation step,
     * param0 is #P2SensorEvent
     */
    ZFEVENT(P2SensorEvent)
    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when any child shape fired contact event during each simulation step,
     * param0 is #P2ContactEvent
     */
    ZFEVENT(P2ContactEvent)

    /**
     * @brief see #ZFObject::observerNotify
     *
     * called when #p2_UIOffset or #p2_UISize or #p2_UIScale changed, or #p2_UIUpdateRequest called
     */
    ZFEVENT(P2UIUpdate)

public:
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_gravity, ZFUIPointCreate(0, -10))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_gravity)

public:
    /** @brief see #P2World, positive value means item moves to right top, in P2's size unit */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_UIOffset)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIPoint, p2_UIOffset)
    /** @brief see #P2World, in P2's size unit */
    ZFPROPERTY_ASSIGN(ZFUISize, p2_UISize)
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUISize, p2_UISize)
    /** @brief see #P2World, `p2_UIScale = UI_size_unit / P2_size_unit` */
    ZFPROPERTY_ASSIGN(zffloat, p2_UIScale, 50)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_UIScale)
    /**
     * @brief explicitly request update UI
     *
     * #P2WorldImpl::UIUpdate would only get called if #p2_UIOffset or #p2_UISize or #p2_UIScale really changed,
     * you may use this method to explicitly request update UI,
     * typically because of renderer size changed
     */
    ZFMETHOD_DECLARE_0(void, p2_UIUpdateRequest)

    /** @brief see #P2World, extra margin to calculate proper #p2_UIVisibleArea, in P2's size unit */
    ZFPROPERTY_ASSIGN(ZFUIMargin, p2_UIVisibleAreaMargin, ZFUIMarginCreate(-1))
    ZFPROPERTY_ON_UPDATE_DECLARE(ZFUIMargin, p2_UIVisibleAreaMargin)
    /** @brief see #P2World, visible area according to #p2_UIOffset and #p2_UISize */
    ZFMETHOD_DECLARE_0(const ZFUIRect &, p2_UIVisibleArea)

    /**
     * @brief whether #p2_UIOffset has changed, reset after each #E_P2UIUpdate event
     */
    ZFMETHOD_DECLARE_0(zfbool, p2_UIOffsetChanged)
    /**
     * @brief whether #p2_UISize has changed, reset after each #E_P2UIUpdate event
     */
    ZFMETHOD_DECLARE_0(zfbool, p2_UISizeChanged)
    /**
     * @brief whether #p2_UIScale has changed, reset after each #E_P2UIUpdate event
     */
    ZFMETHOD_DECLARE_0(zfbool, p2_UIScaleChanged)
    /**
     * @brief whether #p2_UIVisibleArea has changed, reset after each #E_P2UIUpdate event
     */
    ZFMETHOD_DECLARE_0(zfbool, p2_UIVisibleAreaChanged)

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_start)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_stop)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfbool, p2_started)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_manualUpdate)

public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_bodyList, zfobj<ZFArray>())

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_body
            , ZFMP_IN(P2Body *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_bodyCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_bodyRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Body>, p2_bodyRemove
            , ZFMP_IN(P2Body *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_bodyRemoveAll)
    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #p2_bodyList, breadth first\n
     * bodyId can be:
     * -  exact bodyId
     * -  chained bodyId: `bodyId1.bodyId3...`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Body *, p2_bodyFind
            , ZFMP_IN(const zfstring &, bodyId)
            )
    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #p2_bodyList, breadth first\n
     * shapeId can be:
     * -  exact shapeId
     * -  chained bodyId and shapeId: `bodyId1.bodyId3...shapeId`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Shape *, p2_shapeFind
            , ZFMP_IN(const zfstring &, shapeId)
            )

public:
    /** @brief see #P2World, must not manually modify */
    ZFPROPERTY_RETAIN_READONLY(zfanyT<ZFArray>, p2_jointList, zfobj<ZFArray>())

    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(void, p2_joint
            , ZFMP_IN(P2Joint *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(zfindex, p2_jointCount)
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemoveAt
            , ZFMP_IN(zfindex, index)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_1(zfautoT<P2Joint>, p2_jointRemove
            , ZFMP_IN(P2Joint *, v)
            )
    /** @brief see #P2World */
    ZFMETHOD_DECLARE_0(void, p2_jointRemoveAll)
    /**
     * @brief see #P2World, note this method is expensive
     *
     * recursively search in #P2World::p2_jointList and #P2Body::p2_jointFind, breadth first\n
     * jointId can be:
     * -  exact jointId
     * -  chained bodyId and jointId: `bodyId1.bodyId3...shapeId`,
     *   no need to specify all bodies of each depth
     */
    ZFMETHOD_DECLARE_1(P2Joint *, p2_jointFind
            , ZFMP_IN(const zfstring &, jointId)
            )

public:
    /**
     * @brief see #P2World
     *
     * callback's param0 would be #P2Shape that was overlapped\n
     * you may:
     * -  set #ZFArgs::eventFiltered to true to stop test
     * -  set #ZFArgs::result, the last result would be returned as this method's result
     */
    ZFMETHOD_DECLARE_4(zfauto, p2_overlapTest
            , ZFMP_IN(const ZFUIRect &, rect)
            , ZFMP_IN(const ZFListener &, callback)
            , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
            , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
            )
    /**
     * @brief see #P2World
     *
     * callback's param0 would be #P2Shape that was hit by ray test\n
     * you may:
     * -  set #ZFArgs::eventFiltered to true to stop test,
     *   and it's initialized as true,
     *   so that it's stopped for first hit
     * -  set #ZFArgs::result, the last result would be returned as this method's result
     */
    ZFMETHOD_DECLARE_5(zfauto, p2_rayTest
            , ZFMP_IN(const ZFUIPoint &, src)
            , ZFMP_IN(const ZFUIPoint &, direction)
            , ZFMP_IN(const ZFListener &, callback)
            , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
            , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
            )

public:
    /** @brief the impl */
    P2WorldImpl *p2impl;

protected:
    zfoverride
    virtual void objectOnInit(void);
    zfoverride
    virtual void objectOnDealloc(void);
    zfoverride
    virtual void objectOnDeallocPrepare(void);
    zfoverride
    virtual void observerOnAdd(ZF_IN zfidentity eventId);
    zfoverride
    virtual void observerOnRemove(ZF_IN zfidentity eventId);

public:
    _ZFP_P2WorldPrivate *_ZFP_P2World_d;
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2World_h_

