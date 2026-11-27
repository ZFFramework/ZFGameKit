/**
 * @file P2MoverForTouch.h
 * @brief P2Ext to move #P2Body with touch
 */

#ifndef _ZFI_P2MoverForTouch_h_
#define _ZFI_P2MoverForTouch_h_

#include "P2World.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief P2Ext to move #P2Body with touch
 */
zfclass ZFLIB_ZFGameKit P2MoverForTouch : zfextend P2Ext {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(P2MoverForTouch, P2Ext)

public:
    /**
     * @brief whether relative move mode
     *
     * -  false: body would be moved to touch position
     * -  true: body would be moved when touch move, relative to touch down position
     */
    ZFPROPERTY_ASSIGN(zfbool, p2_relativeMode)

    /** @brief extra offset to fix move position */
    ZFPROPERTY_ASSIGN(ZFUIPoint, p2_offset, ZFUIPointCreate(0, 2))

public:
    /** @brief motor force when moving unit */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorForce, P2JointForceHard())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorForce)

    /** @brief motor force when moving unit */
    ZFPROPERTY_ASSIGN(zffloat, p2_motorTorque, 0)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_motorTorque)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceHertz, P2JointHertzDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_distanceDamping, 2)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_distanceDamping)

    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularHertz, P2JointHertzDefault())
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularHertz)
    /** @brief see #P2World */
    ZFPROPERTY_ASSIGN(zffloat, p2_angularDamping, 2)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_angularDamping)

public:
    /**
     * @brief manually move, top right is positive
     *
     * after called, unit would keep moving to position (x, y),
     * ignoring touch events,
     * until manually call p2_move(ZFUIPointCreate(P2_MAX(), P2_MAX())) to stop
     */
    ZFMETHOD_DECLARE_1(void, p2_move
            , ZFMP_IN(const ZFUIPoint &, v)
            )
    /** @brief see #p2_move */
    ZFMETHOD_DECLARE_0(const ZFUIPoint &, p2_move)

protected:
    zfoverride
    virtual void p2_extAttach(ZF_IN P2Body *owner, ZF_IN_OUT zfbool &listenOnUpdate);
    zfoverride
    virtual void p2_extDetach(ZF_IN P2Body *owner);

private:
    ZFUIPoint _move;
    P2BodyType _targetType;
    ZFUIPoint _downTargetPos;
    ZFUIPoint _downTouchPos;
    ZFListener _onTouch;
    zfautoT<P2JointMotor> _ctrl;
    zfautoT<P2Body> _helper;
protected:
    /** @cond ZFPrivateDoc */
    P2MoverForTouch(void)
    : _move(ZFUIPointCreate(P2_MAX(), P2_MAX()))
    , _targetType()
    , _downTargetPos()
    , _downTouchPos()
    , _onTouch()
    , _ctrl()
    , _helper()
    {}
    /** @endcond */
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2MoverForTouch_h_

