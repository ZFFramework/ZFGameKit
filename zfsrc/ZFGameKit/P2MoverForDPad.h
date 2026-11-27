/**
 * @file P2MoverForDPad.h
 * @brief P2Ext to move #P2Body with DPad
 */

#ifndef _ZFI_P2MoverForDPad_h_
#define _ZFI_P2MoverForDPad_h_

#include "P2World.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief P2Ext to move #P2Body with DPad
 */
zfclass ZFLIB_ZFGameKit P2MoverForDPad : zfextend P2Ext {
    ZFOBJECT_DECLARE_WITH_CUSTOM_CTOR(P2MoverForDPad, P2Ext)

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
     * until manually call p2_move(ZFUIPointZero()) to stop
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
    virtual void p2_extUpdate(ZF_IN P2Body *owner);

private:
    ZFUIPoint _move;
    ZFUIPoint _prev;
    zfint _fade;
protected:
    /** @cond ZFPrivateDoc */
    P2MoverForDPad(void) : _move(), _prev(), _fade(0) {}
    /** @endcond */
};

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2MoverForDPad_h_

