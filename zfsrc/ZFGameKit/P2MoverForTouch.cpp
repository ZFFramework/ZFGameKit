#include "P2MoverForTouch.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFOBJECT_REGISTER(P2MoverForTouch)

ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_motorForce) {
    if(_ctrl) {
        _ctrl->p2_motorForce(propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_motorTorque) {
    if(_ctrl) {
        _ctrl->p2_motorTorque(propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_distanceHertz) {
    if(_ctrl) {
        _ctrl->p2_distanceHertz(propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_distanceDamping) {
    if(_ctrl) {
        _ctrl->p2_distanceDamping(propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_angularHertz) {
    if(_ctrl) {
        _ctrl->p2_angularHertz(propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2MoverForTouch, zffloat, p2_angularDamping) {
    if(_ctrl) {
        _ctrl->p2_angularDamping(propertyValue);
    }
}

ZFMETHOD_DEFINE_1(P2MoverForTouch, void, p2_move
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    _move = v;
}
ZFMETHOD_DEFINE_0(P2MoverForTouch, const ZFUIPoint &, p2_move) {
    return _move;
}

void P2MoverForTouch::p2_extAttach(ZF_IN P2Body *owner, ZF_IN_OUT zfbool &listenOnUpdate) {
    zfsuper::p2_extAttach(owner, listenOnUpdate);
    P2World *world = owner->p2_ownerWorld();
    ZFCoreAssert(world);

    zfself *ownerExt = this;
    ZFLISTENER_1(onTouch
            , zfweakT<zfself>, ownerExt
            ) {
    } {
        zfself *t = ownerExt;
        P2World *world = t->p2_ownerWorld();
        ZFUIMouseEvent *event = zfargs.param0();
        switch(event->mouseAction) {
            case v_ZFUIMouseAction::e_Down:
            case v_ZFUIMouseAction::e_Move: {
                event->eventResolved(zftrue);
                t->_targetType = t->p2_ownerBody()->p2_type();
                if(t->_targetType != v_P2BodyType::e_Dynamic) {
                    t->p2_ownerBody()->p2_type(v_P2BodyType::e_Dynamic);
                }
                t->p2_ownerBody()->p2_sleeping(zffalse);
                if(t->_helper == zfnull) {
                    t->_helper = zfobj<ZFUIView>();
                    t->_helper->p2_privateMark();
                    world->p2_body(t->_helper);
                    t->_helper->p2_type(v_P2BodyType::e_Kinematic);
                }
                if(t->_ctrl == zfnull) {
                    t->_ctrl = zfobj<P2JointMotor>();
                    t->_ctrl->p2_privateMark();
                    world->p2_joint(t->_ctrl);
                    t->_ctrl->p2_body0(t->_helper);
                    t->_ctrl->p2_body1(t->p2_ownerBody());
                    t->_ctrl->p2_motorForce(t->p2_motorForce());
                    t->_ctrl->p2_motorTorque(t->p2_motorTorque());
                    t->_ctrl->p2_distanceHertz(t->p2_distanceHertz());
                    t->_ctrl->p2_distanceDamping(t->p2_distanceDamping());
                    t->_ctrl->p2_angularHertz(t->p2_angularHertz());
                    t->_ctrl->p2_angularDamping(t->p2_angularDamping());
                }
                if(event->mouseAction == v_ZFUIMouseAction::e_Down) {
                    t->_downTargetPos = t->p2_ownerBody()->p2_positionCur();
                    t->_downTouchPos = event->mousePoint;
                }
                ZFUIPoint target;
                const ZFUIPoint &offset = t->p2_offset();
                if(t->_move.x != P2_MAX() && t->_move.y != P2_MAX()) {
                    target.x = t->_move.x + offset.x;
                    target.y = t->_move.y + offset.y;
                }
                else if(t->p2_relativeMode()) {
                    target.x = t->_downTargetPos.x + (event->mousePoint.x - t->_downTouchPos.x) / world->p2_UIScale() + offset.x;
                    target.y = t->_downTargetPos.y - (event->mousePoint.y - t->_downTouchPos.y) / world->p2_UIScale() + offset.y;
                }
                else {
                    target.x = event->mousePoint.x / world->p2_UIScale() - world->p2_UIOffset().x + offset.x;
                    target.y = (zfcast(ZFUIView *, world)->height() - event->mousePoint.y) / world->p2_UIScale() - world->p2_UIOffset().y + offset.y;
                }
                t->_helper->p2_position(target);
                break;
            }
            case v_ZFUIMouseAction::e_Up:
            case v_ZFUIMouseAction::e_Cancel: {
                event->eventResolved(zftrue);
                if(t->_ctrl) {
                    t->_ctrl->p2_removeFromParent();
                    t->_ctrl = zfnull;
                }
                if(t->_helper) {
                    t->_helper->p2_removeFromParent();
                    t->_helper = zfnull;
                }
                if(t->_targetType != v_P2BodyType::e_Dynamic) {
                    t->p2_ownerBody()->p2_type(t->_targetType);
                }
                break;
            }
            default:
                break;
        }
    } ZFLISTENER_END()
    _onTouch = onTouch;
    world->observerAdd(ZFUIView::E_ViewOnMouseEvent(), _onTouch);
}
void P2MoverForTouch::p2_extDetach(ZF_IN P2Body *owner) {
    if(_onTouch) {
        owner->p2_ownerWorld()->observerRemove(ZFUIView::E_ViewOnMouseEvent(), _onTouch);
        _onTouch = zfnull;
    }
    if(_ctrl) {
        _ctrl->p2_removeFromParent();
        _ctrl = zfnull;
    }
    if(_helper) {
        _helper->p2_removeFromParent();
        _helper = zfnull;
    }
    if(_targetType != v_P2BodyType::e_Dynamic) {
        owner->p2_type(_targetType);
    }
    zfsuper::p2_extDetach(owner);
}

ZF_NAMESPACE_GLOBAL_END

