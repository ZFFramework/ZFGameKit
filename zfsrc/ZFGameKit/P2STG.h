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
    ZFOBJECT_DECLARE(P2MoverForVelocity, P2Ext)

public:
    /** @brief velocity when moving unit */
    ZFPROPERTY_ASSIGN(zffloat, p2_velocity, 5)
    /**
     * @brief use which game pad to control, zfindexMax to disable
     *
     * when disabled, you may use #move to manually move
     */
    ZFPROPERTY_ASSIGN(zfindex, p2_gamePadIndex, 0)
    ZFPROPERTY_ON_UPDATE_DECLARE(zfindex, p2_gamePadIndex)

public:
    /**
     * @brief manually move, top right is positive
     *
     * after called, unit would keep moving with (x, y) direction of #velocity,
     * must manually call move(0, 0) to stop moving
     */
    ZFMETHOD_DECLARE_2(void, p2_move
            , ZFMP_IN(zffloat, x)
            , ZFMP_IN(zffloat, y)
            )

protected:
    zfoverride
    virtual void p2_extOnAttach(ZF_IN P2Unit *owner);
    zfoverride
    virtual void p2_extOnDetach(ZF_IN P2Unit *owner);
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2STG_h_

