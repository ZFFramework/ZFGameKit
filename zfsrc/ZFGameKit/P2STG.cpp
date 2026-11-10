#include "P2STG.h"
#include "P2GamePad.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(P2MoverForVelocity)

ZFMETHOD_DEFINE_2(P2MoverForVelocity, void, p2_move
        , ZFMP_IN(zffloat, x)
        , ZFMP_IN(zffloat, y)
        ) {
    if(this->p2_ownerUnit() && this->p2_ownerUnit()->p2_body()) {
        if(x == 0 && y == 0) {
            this->p2_ownerUnit()->p2_body()->p2_positionVelocity(ZFUIPointZero());
        }
        else {
            zffloat t = this->p2_velocity() / zfm_sqrt(x * x + y * y);
            this->p2_ownerUnit()->p2_body()->p2_positionVelocity(ZFUIPointCreate(x * t, y * t));
        }
    }
}

static void _ZFP_P2MoverForVelocity_gamePadUpdate(
        ZF_IN P2MoverForVelocity *mover
        , ZF_IN zfindex indexOld
        , ZF_IN zfindex indexNew
        ) {
    zfstring key = "_ZFP_P2MoverForVelocityCb";

    if(indexOld != zfindexMax()) {
        zfautoT<v_ZFListener> t = mover->objectTagRemoveAndGet(key);
        if(t) {
            P2GamePadObserver(indexOld).observerRemove(ZFGlobalEvent::E_P2GamePadButtonUpdate(), t->zfv);
        }
    }

    if(indexNew != zfindexMax()) {
        ZFLISTENER_1(impl
                , P2MoverForVelocity *, mover
                ) {
            P2GamePadButton btn = zfargs.param0().to<v_P2GamePadButton *>()->zfv();
            switch(btn) {
                case v_P2GamePadButton::e_DPadL:
                case v_P2GamePadButton::e_DPadT:
                case v_P2GamePadButton::e_DPadR:
                case v_P2GamePadButton::e_DPadB: {
                    zffloat x = 0;
                    zffloat y = 0;
                    if(P2GamePadPressed(v_P2GamePadButton::e_DPadL)) {
                        if(!P2GamePadPressed(v_P2GamePadButton::e_DPadR)) {
                            x = -1;
                        }
                    }
                    else if(P2GamePadPressed(v_P2GamePadButton::e_DPadR)) {
                        x = 1;
                    }
                    if(P2GamePadPressed(v_P2GamePadButton::e_DPadT)) {
                        if(!P2GamePadPressed(v_P2GamePadButton::e_DPadB)) {
                            y = 1;
                        }
                    }
                    else if(P2GamePadPressed(v_P2GamePadButton::e_DPadB)) {
                        y = -1;
                    }
                    mover->p2_move(x, y);
                    break;
                }
                case v_P2GamePadButton::e_LStickX:
                case v_P2GamePadButton::e_LStickY:
                    mover->p2_move(
                            P2GamePadValue(v_P2GamePadButton::e_LStickX)
                            , P2GamePadValue(v_P2GamePadButton::e_LStickY)
                            );
                    break;
                default:
                    break;
            }
        } ZFLISTENER_END()
        mover->objectTag(key, zfobj<v_ZFListener>(impl));
        P2GamePadObserver(indexNew).observerAdd(ZFGlobalEvent::E_P2GamePadButtonUpdate(), impl);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForVelocity, zfindex, p2_gamePadIndex) {
    _ZFP_P2MoverForVelocity_gamePadUpdate(this, propertyValueOld, propertyValue);
}
void P2MoverForVelocity::p2_extOnAttach(ZF_IN P2Unit *owner) {
    zfsuper::p2_extOnAttach(owner);
    _ZFP_P2MoverForVelocity_gamePadUpdate(this, this->p2_gamePadIndex(), this->p2_gamePadIndex());
}
void P2MoverForVelocity::p2_extOnDetach(ZF_IN P2Unit *owner) {
    _ZFP_P2MoverForVelocity_gamePadUpdate(this, this->p2_gamePadIndex(), zfindexMax());
    zfsuper::p2_extOnDetach(owner);
}

ZF_NAMESPACE_GLOBAL_END

