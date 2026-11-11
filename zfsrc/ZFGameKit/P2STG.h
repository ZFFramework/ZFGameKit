/**
 * @file P2STG.h
 * @brief shooting game util
 */

#ifndef _ZFI_P2STG_h_
#define _ZFI_P2STG_h_

#include "P2World.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief helper to move #P2Unit with velocity
 */
zfclass ZFLIB_ZFGameKit P2MoverForVelocity : zfextend P2Ext {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(P2MoverForVelocity, P2Ext)

public:
    /** @brief velocity when moving unit */
    ZFPROPERTY_ASSIGN(zffloat, p2_velocity, 10)
    ZFPROPERTY_ON_UPDATE_DECLARE(zffloat, p2_velocity)
    /**
     * @brief use which game pad to control, zfindexMax to disable
     *
     * when disabled, you may use #move to manually move
     */
    ZFPROPERTY_ASSIGN(zfindex, p2_gamePadIndex, 0)
    /** @brief frame to fade when changing velocity */
    ZFPROPERTY_ASSIGN(zfuint, p2_fadeFrame, 5)

public:
    /**
     * @brief manually move, top right is positive
     *
     * after called, unit would keep moving with (x, y) direction of #velocity,
     * ignoring #p2_gamePadIndex,
     * until manually call move(0, 0) to stop
     */
    ZFMETHOD_DECLARE_2(void, p2_move
            , ZFMP_IN(zffloat, x)
            , ZFMP_IN(zffloat, y)
            )

protected:
    zfoverride
    virtual void p2_extOnAttach(ZF_IN P2Unit *owner, ZF_IN_OUT zfbool &listenOnUpdate);
    zfoverride
    virtual void p2_extOnUpdate(ZF_IN P2Unit *owner);

private:
    zffloat _x;
    zffloat _y;
    zffloat _xPrev;
    zffloat _yPrev;
    zfint _fade;
protected:
    /** @cond ZFPrivateDoc */
    P2MoverForVelocity(void) : _x(0), _y(0), _xPrev(0), _yPrev(0), _fade(0) {}
    /** @endcond */
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2STG_h_

