#include "P2STG.h"
#include "P2GamePad.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(P2MoverForDPad)

ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForDPad, zffloat, p2_velocity) {
    if(propertyValue <= 0) {
        propertyValue = 5;
    }
}

ZFMETHOD_DEFINE_2(P2MoverForDPad, void, p2_move
        , ZFMP_IN(zffloat, x)
        , ZFMP_IN(zffloat, y)
        ) {
    _x = x;
    _y = y;
}
void P2MoverForDPad::p2_extAttach(ZF_IN P2Body *owner, ZF_IN_OUT zfbool &listenOnUpdate) {
    zfsuper::p2_extAttach(owner, listenOnUpdate);
    listenOnUpdate = zftrue;
}
void P2MoverForDPad::p2_extUpdate(ZF_IN P2Body *owner) {
    zfsuper::p2_extUpdate(owner);
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

    zfint fadeFrame = (zfint)this->p2_fadeFrame();
    if(x != 0 || y != 0) {
        _xPrev = x;
        _yPrev = y;
        if(fadeFrame != 0) {
            if(_fade < 0) {
                if(_fade > -fadeFrame) {
                    _fade = fadeFrame + _fade;
                }
                else {
                    _fade = 1;
                }
            }
            else if(_fade < fadeFrame) {
                ++_fade;
            }
        }
        zffloat v = this->p2_velocity() * this->p2_ownerBody()->p2_bodyScaleFixed();
        if(_fade < fadeFrame) {
            v = v * _fade / fadeFrame;
        }
        zffloat t = v / zfm_sqrt(x * x + y * y);
        this->p2_ownerBody()->p2_positionVelocity(ZFUIPointCreate(x * t, y * t));
    }
    else {
        if(fadeFrame != 0) {
            if(_fade < 0) {
                if(_fade > -fadeFrame) {
                    --_fade;
                }
            }
            else {
                if(_fade < fadeFrame) {
                    _fade = _fade - fadeFrame;
                }
                else {
                    _fade = -1;
                }
            }
        }
        if(_fade > -fadeFrame && (_xPrev || _yPrev)) {
            zffloat v = this->p2_velocity() * this->p2_ownerBody()->p2_bodyScaleFixed() * (1 + (zffloat)_fade / fadeFrame);
            x = _xPrev;
            y = _yPrev;
            zffloat t = v / zfm_sqrt(x * x + y * y);
            this->p2_ownerBody()->p2_positionVelocity(ZFUIPointCreate(x * t, y * t));
        }
        else {
            _fade = 0;
            this->p2_ownerBody()->p2_positionVelocity(ZFUIPointZero());
        }
    }
}

ZF_NAMESPACE_GLOBAL_END

