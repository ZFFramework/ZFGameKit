#include "P2STG.h"
#include "P2GamePad.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(P2MoverForVelocity)

ZFMETHOD_DEFINE_2(P2MoverForVelocity, void, p2_move
        , ZFMP_IN(zffloat, x)
        , ZFMP_IN(zffloat, y)
        ) {
    _x = x;
    _y = y;
}
void P2MoverForVelocity::p2_extOnAttach(ZF_IN P2Unit *owner, ZF_IN_OUT zfbool &listenOnUpdate) {
    zfsuper::p2_extOnAttach(owner, listenOnUpdate);
    listenOnUpdate = zftrue;
}
void P2MoverForVelocity::p2_extOnUpdate(ZF_IN P2Unit *owner) {
    zfsuper::p2_extOnUpdate(owner);
    zffloat x = 0;
    zffloat y = 0;
    if(_x != 0 || _y != 0) {
        x = _x;
        y = _y;
    }
    else {
        zfindex padIndex = this->p2_gamePadIndex();
        x = P2GamePadValue(v_P2GamePadButton::e_LStickX, padIndex);
        if(P2GamePadPressed(v_P2GamePadButton::e_DPadL, padIndex)) {
            if(!P2GamePadPressed(v_P2GamePadButton::e_DPadR, padIndex)) {
                x = -1;
            }
        }
        else if(P2GamePadPressed(v_P2GamePadButton::e_DPadR, padIndex)) {
            x = 1;
        }
        y = P2GamePadValue(v_P2GamePadButton::e_LStickY, padIndex);
        if(P2GamePadPressed(v_P2GamePadButton::e_DPadB, padIndex)) {
            if(!P2GamePadPressed(v_P2GamePadButton::e_DPadT, padIndex)) {
                y = -1;
            }
        }
        else if(P2GamePadPressed(v_P2GamePadButton::e_DPadT, padIndex)) {
            y = 1;
        }
    }

    if(x != 0 || y != 0) {
        zffloat t = this->p2_velocity() / zfm_sqrt(x * x + y * y);
        this->p2_ownerUnit()->p2_body()->p2_positionVelocity(ZFUIPointCreate(x * t, y * t));
    }
    else {
        this->p2_ownerUnit()->p2_body()->p2_positionVelocity(ZFUIPointZero());
    }
}

ZF_NAMESPACE_GLOBAL_END

