#include "P2World.h"
#include "../../zfsrc_ext/ZFImpl/_repo/box2d/box2d/box2d.h"
#include "ZFCore/ZFSTLWrapper/zfstlhashmap.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFMETHOD_FUNC_USER_REGISTER_FOR_FUNC_0(zffloat, P2_MAX)

ZF_STATIC_REGISTER_INIT(P2EnvSetup) {
    b2SetAssertFcn(b2fn_assert);
}
private:
    static int b2fn_assert(const char *condition, const char *fileName, int lineNumber) {
        ZFCoreCriticalMessageTrim(
                "[%s (%s)] box2d assert fail: %s"
                , fileName
                , lineNumber
                , condition
                );
        return 0;
    }
ZF_STATIC_REGISTER_END(P2EnvSetup)

// ============================================================
static ZFUIPoint b2Vec2ToZF(ZF_IN const b2Vec2 &v) {
    return ZFUIPointCreate(v.x, v.y);
}
static b2Vec2 b2Vec2FromZF(ZF_IN const ZFUIPoint &v) {
    b2Vec2 ret;
    ret.x = v.x;
    ret.y = v.y;
    return ret;
}
static zffloat b2RadianNormalize(ZF_IN zffloat v) {
    if(v < 0 || v >= zfm_PI() * 2) {
        v = zfm_fmod(zfm_fmod(v, zfm_PI() * 2) + zfm_PI() * 2, zfm_PI() * 2);
    }
    if(zfmAbs(v) <= zfm_PI() / 180 / 5) {
        return 0;
    }
    else {
        return v;
    }
}
static zffloat b2DegreeNormalize(ZF_IN zffloat v) {
    if(v < 0 || v >= 360.0f) {
        v = zfm_fmod(zfm_fmod(v, 360.0f) + 360.0f, 360.0f);
    }
    if(zfmAbs(v) <= 1 / 5) {
        return 0;
    }
    else {
        return v;
    }
}
static zffloat b2RotToZF(ZF_IN const b2Rot &v) {
    return b2DegreeNormalize(0.0f - (zfm_atan2(v.s, v.c) * 180 / zfm_PI()));
}
static b2Rot b2RotFromZF(ZF_IN zffloat v) {
    return b2MakeRot(b2RadianNormalize((0.0f - v) * zfm_PI() / 180));
}
static zffloat b2RadToZF(ZF_IN float v) {
    return 0.0f - (v * 180 / zfm_PI());
}
static float b2RadFromZF(ZF_IN zffloat v) {
    return (0.0f - v) * zfm_PI() / 180;
}
static zffloat b2AngularVelocityToZF(ZF_IN float v) {
    return 0.0f - (v * 180 / zfm_PI());
}
static float b2AngularVelocityFromZF(ZF_IN zffloat v) {
    return 0.0f - v * zfm_PI() / 180;
}
static ZFUIRect b2AABBToZF(ZF_IN const b2AABB &v) {
    return ZFUIRectCreate(
            v.lowerBound.x
            , v.lowerBound.y
            , v.upperBound.x - v.lowerBound.x
            , v.upperBound.y - v.lowerBound.y
            );
}
static b2AABB b2AABBFromZF(ZF_IN const ZFUIRect &v) {
    b2AABB ret;
    ret.lowerBound.x = v.x;
    ret.lowerBound.y = v.y;
    ret.upperBound.x = v.x + v.width;
    ret.upperBound.y = v.y + v.height;
    return ret;
}

// ============================================================
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterMaskAll, P2FilterMaskAll())
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterCategoryDefault, P2FilterCategoryDefault())
ZFEXPORT_VAR_READONLY_DEFINE(zfflags, P2FilterMaskDefault, P2FilterMaskDefault())

// ============================================================
static void _ZFP_P2ShapeAttach(ZF_IN P2Body *owner, ZF_IN P2Shape *shape);
static void _ZFP_P2ShapeDetach(ZF_IN P2Body *owner, ZF_IN P2Shape *shape);
static void _ZFP_P2JointAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Body *ownerBody, ZF_IN P2Joint *joint);
static void _ZFP_P2JointDetach(ZF_IN P2Joint *joint);
static void _ZFP_P2BodyAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Body *ownerBody, ZF_IN P2Body *body);
static void _ZFP_P2BodyDetach(ZF_IN P2Body *body);
static void _ZFP_P2ExtAttach(ZF_IN P2Body *owner, ZF_IN P2Ext *ext);
static void _ZFP_P2ExtDetach(ZF_IN P2Ext *ext);

static void _ZFP_P2BodyImplMassUpdateRequest(ZF_IN P2Body *body);
static void _ZFP_P2WorldImplUpdate(ZF_IN P2World *world);

zfclassNotPOD _ZFP_P2ShapePrivate {
public:
    b2ShapeId implShapeId;
public:
    _ZFP_P2ShapePrivate(void)
    : implShapeId(b2_nullShapeId)
    {
    }
public:
    static ZFUIRect shapeAABBLocal(ZF_IN P2Shape *shape) {
        return shape->p2impl_AABBLocal();
    }
    static void shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN P2Shape *shape, ZF_IN zffloat shapeScale) {
        shape->p2impl_shapeCreate(ownerBody, shapeScale);
    }
};

zfclassNotPOD _ZFP_P2JointPrivate {
public:
    b2JointId implJointId;
    P2World *ownerWorld; // null if belong to body, even if already added to world
    P2Body *ownerBody;
public:
    _ZFP_P2JointPrivate(void)
    : implJointId(b2_nullJointId)
    , ownerWorld(zfnull)
    , ownerBody(zfnull)
    {
    }
public:
    static void jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1, ZF_IN P2Joint *joint) {
        joint->p2impl_jointCreate(ownerBody0, ownerBody1);
    }
    static zffloat jointScale(ZF_IN P2Joint *joint) {
        if(joint->_ZFP_P2Joint_d->ownerBody) {
            return joint->_ZFP_P2Joint_d->ownerBody->p2_bodyScaleFixed();
        }
        else {
            return 1;
        }
    }
};

zfclassNotPOD _ZFP_P2ExtPrivate {
public:
    static void attach(ZF_IN P2Body *owner, ZF_IN P2Ext *ext) {
        ext->_ZFP_ownerBody = owner;
        zfbool listenOnUpdate = zffalse;
        ext->p2_extAttach(owner, listenOnUpdate);
        ext->observerNotify(P2Ext::E_P2ExtAttach(), ZFArgs()
                .param0(owner)
                );
        if(listenOnUpdate) {
            ZFLISTENER_1(impl
                    , P2Ext *, ext
                    ) {
                if(ext->p2_ownerBody()) {
                    ext->p2_extUpdate(ext->p2_ownerBody());
                }
            } ZFLISTENER_END()
            ext->objectTag(zftext("_ZFP_P2ExtListenOnUpdate"), zfobj<v_ZFListener>(impl));
            owner->observerAdd(P2Body::E_P2Update(), impl);
        }
        _ZFP_P2ExtPrivate::observerAttach(owner, ext);
    }
    static void detach(ZF_IN P2Ext *ext) {
        P2Body *owner = ext->_ZFP_ownerBody;
        ext->_ZFP_ownerBody = zfnull;
        _ZFP_P2ExtPrivate::observerDetach(owner, ext);
        {
            zfautoT<v_ZFListener> t = ext->objectTagRemoveAndGet(zftext("_ZFP_P2ExtListenOnUpdate"));
            if(t) {
                owner->observerRemove(P2Body::E_P2Update(), t->zfv);
            }
        }
        ext->observerNotify(P2Ext::E_P2ExtDetach(), ZFArgs()
                .param0(owner)
                );
        ext->p2_extDetach(owner);
    }
    static void observerAttach(ZF_IN P2Body *owner, ZF_IN P2Ext *ext) {
        if(ext->observerHasAdd(P2Ext::E_P2ExtUpdate())) {
            ZFLISTENER_1(impl
                    , P2Ext *, ext
                    ) {
                if(ext->p2_ownerBody()) {
                    ext->observerNotify(P2Ext::E_P2ExtUpdate(), ZFArgs()
                            .param0(ext->p2_ownerBody())
                            );
                }
            } ZFLISTENER_END()
            ext->objectTag(zftext("_ZFP_P2ExtUpdate"), zfobj<v_ZFListener>(impl));
            owner->observerAdd(P2Body::E_P2Update(), impl);
        }
    }
    static void observerDetach(ZF_IN P2Body *owner, ZF_IN P2Ext *ext) {
        zfautoT<v_ZFListener> t = ext->objectTagRemoveAndGet(zftext("_ZFP_P2ExtUpdate"));
        if(t) {
            owner->observerRemove(P2Body::E_P2Update(), t->zfv);
        }
    }
};

void P2Ext::observerOnAdd(ZF_IN zfidentity eventId) {
    zfsuper::observerOnAdd(eventId);
    if(eventId == P2Ext::E_P2ExtUpdate()) {
        if(this->p2_ownerBody()) {
            _ZFP_P2ExtPrivate::observerAttach(this->p2_ownerBody(), this);
        }
    }
}
void P2Ext::observerOnRemove(ZF_IN zfidentity eventId) {
    if(eventId == P2Ext::E_P2ExtUpdate()) {
        if(this->p2_ownerBody()) {
            _ZFP_P2ExtPrivate::observerDetach(this->p2_ownerBody(), this);
        }
    }
    zfsuper::observerOnRemove(eventId);
}

zfclassNotPOD _ZFP_P2BodyPrivate {
public:
    b2BodyId implBodyId;
    P2World *ownerWorld; // null if belong to another body, even if already added to world
    P2Body *ownerBody;
    zfobj<ZFArray> bodyRefJointList;
    ZFUIRect bodyAABBLocal; // relative to origin point of body
    ZFUIPoint massCenter; // relative to origin point of body

    enum {
        stateFlag_bodyPosUpdateRequested = 1 << 0,
        stateFlag_bodyScaleUpdateRequested = 1 << 1,
        stateFlag_massUpdateRequested = 1 << 2,
        stateFlag_bodyVisible = 1 << 3,
        stateFlag_E_P2VisibilityUpdate = 1 << 4,
        stateFlag_E_P2SensorEnter = 1 << 5,
        stateFlag_E_P2SensorExit = 1 << 6,
        stateFlag_E_P2SensorVisitorEnter = 1 << 7,
        stateFlag_E_P2SensorVisitorExit = 1 << 8,
        stateFlag_E_P2ContactFilter = 1 << 9,
        stateFlag_E_P2ContactEnter = 1 << 10,
        stateFlag_E_P2ContactExit = 1 << 11,
        stateFlag_E_P2Update = 1 << 12,
    };
    zfuint stateFlag;

public:
    _ZFP_P2BodyPrivate(void)
    : implBodyId(b2_nullBodyId)
    , ownerWorld(zfnull)
    , ownerBody(zfnull)
    , bodyRefJointList()
    , massCenter()
    , stateFlag(0
            | stateFlag_bodyPosUpdateRequested
            | stateFlag_bodyScaleUpdateRequested
            | stateFlag_massUpdateRequested
            )
    {
    }
public:
    static void updatePreSolveCfg(ZF_IN P2Body *body) {
        if(body) {
            zfbool enable = ZFBitTest(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter);
            for(zfindex i = body->p2_shapeCount() - 1; i != zfindexMax(); --i) {
                P2Shape *shape = body->p2_shapeAt(i);
                if(B2_IS_NON_NULL(shape->_ZFP_P2Shape_d->implShapeId)) {
                    b2Shape_EnablePreSolveEvents(shape->_ZFP_P2Shape_d->implShapeId, enable);
                }
            }
        }
    }
public:
    void bodyPosUpdate(void) {
        ZFBitUnset(this->stateFlag, stateFlag_bodyPosUpdateRequested);
        if(B2_IS_NULL(this->implBodyId)) {
            this->bodyAABBLocal = ZFUIRectZero();
            this->massCenter = ZFUIPointZero();
        }
        else {
            P2Body *body = (P2Body *)b2Body_GetUserData(this->implBodyId);
            ZFCoreAssert(body);
            {
                bodyAABBLocal = ZFUIRectZero();
                for(zfindex i = body->p2_shapeCount() - 1; i != zfindexMax(); --i) {
                    ZFUIRect shapeAABB = _ZFP_P2ShapePrivate::shapeAABBLocal(body->p2_shapeAt(i));
                    bodyAABBLocal = ZFUIRectUnion(bodyAABBLocal, shapeAABB);
                }
            }

            this->massCenter = ZFUIPointApplyScaleReversely(
                    b2Vec2ToZF(b2Body_GetLocalCenterOfMass(this->implBodyId))
                    , body->p2_bodyScaleFixed()
                    );
        }
    }
    void massUpdate(void) {
        if(ZFBitTest(stateFlag, stateFlag_massUpdateRequested)) {
            // updating mass may modify linearVelocity (because of massCenter changed)
            b2Vec2 t = b2Body_GetLinearVelocity(this->implBodyId);
            b2Body_ApplyMassFromShapes(this->implBodyId);
            b2Body_SetLinearVelocity(this->implBodyId, t);
            ZFBitUnset(stateFlag, stateFlag_massUpdateRequested);
        }
    }
};

zfclassNotPOD _ZFP_P2WorldPrivate {
public:
    b2WorldId implWorldId;
    ZFListener implTimer;
    zfimplhashmap<P2Body *, zfbool> pendingBody; // bodies needs to create or update mass or update shape scale
    zfimplhashmap<P2Joint *, zfbool> pendingJoint; // joints needs to create or update body
    zfimplhashmap<P2Body *, zfbool> pendingBodyMove;
    P2BodyMoveEvent *bodyMoveEvent;
    P2SensorEvent *sensorEvent;
    P2ContactEvent *contactEvent;
    P2BodyVisibilityEvent *bodyVisibilityEvent;
    ZFUIRect visibleArea;
    zfimplhashmap<P2Body *, zfbool> visibleBody;
    zfimplhashmap<P2Body *, zfbool> visibleBodyPrev;
    zfimplhashmap<P2Body *, zfbool> customUpdateBody;

    enum {
        stateFlag_updateRunning = 1 << 0,
        stateFlag_UIUpdateFlag = 1 << 1,
        stateFlag_UIOffsetChanged = 1 << 2,
        stateFlag_UISizeChanged = 1 << 3,
        stateFlag_UIScaleChanged = 1 << 4,
        stateFlag_UIVisibleAreaChanged = 1 << 5,
        stateFlag_UIChangedMask = 0
            | stateFlag_UIOffsetChanged
            | stateFlag_UISizeChanged
            | stateFlag_UIScaleChanged
            | stateFlag_UIVisibleAreaChanged
            ,
        stateFlag_E_P2BodyAttach = 1 << 6,
        stateFlag_E_P2BodyDetach = 1 << 7,
        stateFlag_E_P2JointAttach = 1 << 8,
        stateFlag_E_P2JointDetach = 1 << 9,
        stateFlag_E_P2UpdatePrev = 1 << 10,
        stateFlag_E_P2UpdatePost = 1 << 11,
        stateFlag_E_P2BodyVisibilityEvent = 1 << 12,
        stateFlag_E_P2BodyMoveEvent = 1 << 13,
        stateFlag_E_P2SensorEvent = 1 << 14,
        stateFlag_E_P2ContactEvent = 1 << 15,
        stateFlag_E_P2UIUpdate = 1 << 16,
    };
    zfuint stateFlag;

public:
    _ZFP_P2WorldPrivate(void)
    : implWorldId(b2_nullWorldId)
    , implTimer()
    , pendingBody()
    , pendingJoint()
    , pendingBodyMove()
    , bodyMoveEvent(zfobjAlloc(P2BodyMoveEvent))
    , sensorEvent(zfobjAlloc(P2SensorEvent))
    , contactEvent(zfobjAlloc(P2ContactEvent))
    , bodyVisibilityEvent(zfobjAlloc(P2BodyVisibilityEvent))
    , visibleArea(ZFUIRectZero())
    , visibleBody()
    , visibleBodyPrev()
    , stateFlag()
    {
    }
    ~_ZFP_P2WorldPrivate(void) {
        zfobjRelease(bodyVisibilityEvent);
        zfobjRelease(contactEvent);
        zfobjRelease(sensorEvent);
        zfobjRelease(bodyMoveEvent);
    }

public:
    static void bodyTreeAttach(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFCoreQueuePOD<P2Body *> toUpdate;
        toUpdate.add(body);
        while(!toUpdate.isEmpty()) {
            body = toUpdate.take();
            {
                ZFArray *l = body->p2_bodyList();
                for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                    toUpdate.add(l->get(i));
                }
            }
            world->_ZFP_P2World_d->pendingBody[body] = zftrue;
            ZFArray *l = body->p2_jointList();
            for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                world->_ZFP_P2World_d->pendingJoint[l->get(i)] = zftrue;
            }
        }
    }
};

static zfuint _ZFP_P2Body_stateFlag = 0;
ZF_GLOBAL_INITIALIZER_INIT_WITH_LEVEL(P2Body_stateFlag, ZFLevelZFFrameworkStatic) {
    _ZFP_P2Body_stateFlag = 0;
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2VisibilityUpdate(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2SensorEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2SensorExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2SensorVisitorEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2SensorVisitorExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2ContactEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter);
    ZFGlobalObserver().observerHasAddStateAttach(P2Body::E_P2ContactExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit);
}
ZF_GLOBAL_INITIALIZER_DESTROY(P2Body_stateFlag) {
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2VisibilityUpdate(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2SensorEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2SensorExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2SensorVisitorEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2SensorVisitorExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2ContactEnter(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter);
    ZFGlobalObserver().observerHasAddStateDetach(P2Body::E_P2ContactExit(), &_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit);
}
ZF_GLOBAL_INITIALIZER_END(P2Body_stateFlag)
void P2Body::observerOnAdd(ZF_IN zfidentity eventId) {
    zfsuper::observerOnAdd(eventId);
    if(zffalse) {}
    else if(eventId == P2Body::E_P2VisibilityUpdate()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate);}
    else if(eventId == P2Body::E_P2SensorEnter()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter);}
    else if(eventId == P2Body::E_P2SensorExit()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit);}
    else if(eventId == P2Body::E_P2SensorVisitorEnter()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter);}
    else if(eventId == P2Body::E_P2SensorVisitorExit()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit);}
    else if(eventId == P2Body::E_P2ContactEnter()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter);}
    else if(eventId == P2Body::E_P2ContactExit()) {ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit);}

    else if(eventId == P2Body::E_P2ContactFilter()) {
        ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter);
        _ZFP_P2BodyPrivate::updatePreSolveCfg(this);
    }
    else if(eventId == P2Body::E_P2Update()) {
        ZFBitSet(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2Update);
        if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
            this->p2_ownerWorld()->_ZFP_P2World_d->customUpdateBody[this] = zftrue;
        }
    }
}
void P2Body::observerOnRemove(ZF_IN zfidentity eventId) {
    zfsuper::observerOnRemove(eventId);
    if(zffalse) {}
    else if(eventId == P2Body::E_P2VisibilityUpdate()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate);}
    else if(eventId == P2Body::E_P2SensorEnter()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter);}
    else if(eventId == P2Body::E_P2SensorExit()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit);}
    else if(eventId == P2Body::E_P2SensorVisitorEnter()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter);}
    else if(eventId == P2Body::E_P2SensorVisitorExit()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit);}
    else if(eventId == P2Body::E_P2ContactEnter()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter);}
    else if(eventId == P2Body::E_P2ContactExit()) {ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit);}

    else if(eventId == P2Body::E_P2ContactFilter()) {
        ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter);
        _ZFP_P2BodyPrivate::updatePreSolveCfg(this);
    }
    else if(eventId == P2Body::E_P2Update()) {
        ZFBitUnset(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2Update);
        if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
            this->p2_ownerWorld()->_ZFP_P2World_d->customUpdateBody.erase(this);
        }
    }
}
ZFEVENT_REGISTER(P2Body, P2VisibilityUpdate)
ZFEVENT_REGISTER(P2Body, P2SensorEnter)
ZFEVENT_REGISTER(P2Body, P2SensorExit)
ZFEVENT_REGISTER(P2Body, P2SensorVisitorEnter)
ZFEVENT_REGISTER(P2Body, P2SensorVisitorExit)
ZFEVENT_REGISTER(P2Body, P2ContactFilter)
ZFEVENT_REGISTER(P2Body, P2ContactEnter)
ZFEVENT_REGISTER(P2Body, P2ContactExit)
ZFEVENT_REGISTER(P2Body, P2Update)

static zfuint _ZFP_P2World_stateFlag = 0;
ZF_GLOBAL_INITIALIZER_INIT_WITH_LEVEL(P2World_stateFlag, ZFLevelZFFrameworkStatic) {
    _ZFP_P2World_stateFlag = 0;
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2BodyAttach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2BodyDetach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2JointAttach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2JointDetach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2UpdatePrev(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2UpdatePost(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2BodyVisibilityEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2BodyMoveEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2SensorEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2ContactEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent);
    ZFGlobalObserver().observerHasAddStateAttach(P2World::E_P2UIUpdate(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate);
}
ZF_GLOBAL_INITIALIZER_DESTROY(P2World_stateFlag) {
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2BodyAttach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2BodyDetach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2JointAttach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2JointDetach(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2UpdatePrev(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2UpdatePost(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2BodyVisibilityEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2BodyMoveEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2SensorEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2ContactEvent(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent);
    ZFGlobalObserver().observerHasAddStateDetach(P2World::E_P2UIUpdate(), &_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate);
}
ZF_GLOBAL_INITIALIZER_END(P2World_stateFlag)
void P2World::observerOnAdd(ZF_IN zfidentity eventId) {
    zfsuper::observerOnAdd(eventId);
    if(zffalse) {}
    else if(eventId == P2World::E_P2BodyAttach()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach);}
    else if(eventId == P2World::E_P2BodyDetach()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach);}
    else if(eventId == P2World::E_P2JointAttach()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach);}
    else if(eventId == P2World::E_P2JointDetach()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach);}
    else if(eventId == P2World::E_P2UpdatePrev()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev);}
    else if(eventId == P2World::E_P2UpdatePost()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost);}
    else if(eventId == P2World::E_P2BodyVisibilityEvent()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent);}
    else if(eventId == P2World::E_P2BodyMoveEvent()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent);}
    else if(eventId == P2World::E_P2SensorEvent()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent);}
    else if(eventId == P2World::E_P2ContactEvent()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent);}
    else if(eventId == P2World::E_P2UIUpdate()) {ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate);}
}
void P2World::observerOnRemove(ZF_IN zfidentity eventId) {
    zfsuper::observerOnRemove(eventId);
    if(zffalse) {}
    else if(eventId == P2World::E_P2BodyAttach()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach);}
    else if(eventId == P2World::E_P2BodyDetach()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach);}
    else if(eventId == P2World::E_P2JointAttach()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach);}
    else if(eventId == P2World::E_P2JointDetach()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach);}
    else if(eventId == P2World::E_P2UpdatePrev()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev);}
    else if(eventId == P2World::E_P2UpdatePost()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost);}
    else if(eventId == P2World::E_P2BodyVisibilityEvent()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent);}
    else if(eventId == P2World::E_P2BodyMoveEvent()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent);}
    else if(eventId == P2World::E_P2SensorEvent()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent);}
    else if(eventId == P2World::E_P2ContactEvent()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent);}
    else if(eventId == P2World::E_P2UIUpdate()) {ZFBitUnset(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate);}
}
ZFEVENT_REGISTER(P2World, P2BodyAttach)
ZFEVENT_REGISTER(P2World, P2BodyDetach)
ZFEVENT_REGISTER(P2World, P2JointAttach)
ZFEVENT_REGISTER(P2World, P2JointDetach)
ZFEVENT_REGISTER(P2World, P2UpdatePrev)
ZFEVENT_REGISTER(P2World, P2UpdatePost)
ZFEVENT_REGISTER(P2World, P2BodyVisibilityEvent)
ZFEVENT_REGISTER(P2World, P2BodyMoveEvent)
ZFEVENT_REGISTER(P2World, P2SensorEvent)
ZFEVENT_REGISTER(P2World, P2ContactEvent)
ZFEVENT_REGISTER(P2World, P2UIUpdate)

// ============================================================
ZFOBJECT_REGISTER(P2Shape)

ZFMETHOD_DEFINE_0(P2Shape, P2Body *, p2_ownerBody) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2BodyId implBodyId = b2Shape_GetBody(_ZFP_P2Shape_d->implShapeId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Shape, P2World *, p2_ownerWorld) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2WorldId implWorldId = b2Shape_GetWorld(_ZFP_P2Shape_d->implShapeId);
        ZFCoreAssert(B2_IS_NON_NULL(implWorldId));
        return (P2World *)b2World_GetUserData(implWorldId);
    }
    else {
        return zfnull;
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_density) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_SetDensity(_ZFP_P2Shape_d->implShapeId, propertyValue, zffalse);
        _ZFP_P2BodyImplMassUpdateRequest(this->p2_ownerBody());
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfbool, p2_sensorEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_EnableSensorEvents(_ZFP_P2Shape_d->implShapeId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfbool, p2_contactEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Shape_EnableContactEvents(_ZFP_P2Shape_d->implShapeId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfflags, p2_filterCategory) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.categoryBits = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfflags, p2_filterMask) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.maskBits = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zfint, p2_filterGroup) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2Filter implFilter = b2Shape_GetFilter(_ZFP_P2Shape_d->implShapeId);
        implFilter.groupIndex = propertyValue;
        b2Shape_SetFilter(_ZFP_P2Shape_d->implShapeId, implFilter);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matFriction) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.friction = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matBounce) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.restitution = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matRotationResist) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.rollingResistance = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Shape, zffloat, p2_matTangentSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId)) {
        b2SurfaceMaterial implMat = b2Shape_GetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId);
        implMat.tangentSpeed = propertyValue;
        b2Shape_SetSurfaceMaterial(_ZFP_P2Shape_d->implShapeId, implMat);
    }
}

ZFMETHOD_DEFINE_0(P2Shape, ZFUIRect, p2_AABB) {
    if(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)) {
        return ZFUIRectZero();
    }
    else {
        return b2AABBToZF(b2Shape_GetAABB(_ZFP_P2Shape_d->implShapeId));
    }
}
ZFMETHOD_DEFINE_0(P2Shape, ZFUIRect, p2_AABBLocal) {
    if(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)) {
        return ZFUIRectZero();
    }
    else {
        return this->p2impl_AABBLocal();
    }
}

ZFMETHOD_DEFINE_0(P2Shape, zfbool, p2_implAttached) {
    return B2_IS_NON_NULL(_ZFP_P2Shape_d->implShapeId);
}

void P2Shape::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Shape_d = zfpoolNew(_ZFP_P2ShapePrivate);
}
void P2Shape::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId));
    zfpoolDelete(_ZFP_P2Shape_d);
    zfsuper::objectOnDealloc();
}

static void _ZFP_P2Shape_implShapeDef(ZF_IN_OUT b2ShapeDef &cfg, ZF_IN P2Shape *owner, ZF_IN P2Body *ownerBody) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(owner->_ZFP_P2Shape_d->implShapeId)
            , "shape already attached to body, shape: %s, body: %s, world: %s, adding to body: %d"
            , owner
            , owner->p2_ownerBody()
            , owner->p2_ownerWorld()
            , ownerBody
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody->_ZFP_P2Body_d->implBodyId)
            , "shape owner body not attached to world, shape: %s, adding to body: %s"
            , owner
            , ownerBody
            );

    cfg.userData = owner;
    cfg.material.friction = owner->p2_matFriction();
    cfg.material.restitution = owner->p2_matBounce();
    cfg.material.rollingResistance = owner->p2_matRotationResist();
    cfg.material.tangentSpeed = owner->p2_matTangentSpeed();
    cfg.density = owner->p2_density();
    cfg.filter.categoryBits = (uint64_t)owner->p2_filterCategory();
    cfg.filter.maskBits = (uint64_t)owner->p2_filterMask();
    cfg.filter.groupIndex = (int)owner->p2_filterGroup();
    cfg.isSensor = owner->p2_sensor();
    cfg.enableSensorEvents = owner->p2_sensorEnable();
    cfg.enableContactEvents = owner->p2_contactEnable();
    if(ZFBitTest(ownerBody->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter)) {
        cfg.enablePreSolveEvents = zftrue;
    }
    cfg.invokeContactCreation = zffalse;
    cfg.updateBodyMass = zffalse;
}

// ============================================================
ZFOBJECT_REGISTER(P2ShapeBox)
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeBox, ZFUIPoint, p2_position) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeBox, ZFUISize, p2_size) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeBox, zffloat, p2_rotation) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
    propertyValue = b2DegreeNormalize(propertyValue);
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeBox, zffloat, p2_radius) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
static b2Polygon _ZFP_P2ShapeBox_implDef(ZF_IN P2ShapeBox *shape, ZF_IN zffloat shapeScale) {
    const ZFUISize &size = shape->p2_size();
    return shape->p2_position() == ZFUIPointZero() && shape->p2_rotation() == 0
        ? shape->p2_radius() == 0
            ? b2MakeBox(
                    size.width * shapeScale / 2
                    , size.height * shapeScale / 2
                    )
            : b2MakeRoundedBox(
                    size.width * shapeScale / 2
                    , size.height * shapeScale / 2
                    , shape->p2_radius() * shapeScale
                    )
        : shape->p2_radius() == 0
            ? b2MakeOffsetBox(
                    size.width * shapeScale / 2
                    , size.height * shapeScale / 2
                    , b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_position(), shapeScale))
                    , b2RotFromZF(shape->p2_rotation())
                    )
            : b2MakeOffsetRoundedBox(
                    size.width * shapeScale / 2
                    , size.height * shapeScale / 2
                    , b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_position(), shapeScale))
                    , b2RotFromZF(shape->p2_rotation())
                    , shape->p2_radius() * shapeScale
                    )
        ;
}
ZFUIRect P2ShapeBox::p2impl_AABBLocal(void) {
    b2Polygon impl = _ZFP_P2ShapeBox_implDef(this, 1);
    return b2AABBToZF(b2ComputePolygonAABB(&impl, b2Transform_identity));
}
void P2ShapeBox::p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale) {
    zfsuper::p2impl_shapeCreate(ownerBody, shapeScale);
    ZFCoreAssert(!ZFUISizeIsEmpty(this->p2_size()));

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Polygon implPolygon = _ZFP_P2ShapeBox_implDef(this, shapeScale);
    _ZFP_P2Shape_d->implShapeId = b2CreatePolygonShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implPolygon
            );
}

ZFOBJECT_REGISTER(P2ShapeCircle)
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeCircle, ZFUIPoint, p2_position) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeCircle, zffloat, p2_radius) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
static b2Circle _ZFP_P2ShapeCircle_implDef(ZF_IN P2ShapeCircle *shape, ZF_IN zffloat shapeScale) {
    b2Circle implCircle;
    implCircle.center = b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_position(), shapeScale));
    implCircle.radius = shape->p2_radius() * shapeScale;
    return implCircle;
}
ZFUIRect P2ShapeCircle::p2impl_AABBLocal(void) {
    b2Circle impl = _ZFP_P2ShapeCircle_implDef(this, 1);
    return b2AABBToZF(b2ComputeCircleAABB(&impl, b2Transform_identity));
}
void P2ShapeCircle::p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale) {
    zfsuper::p2impl_shapeCreate(ownerBody, shapeScale);
    ZFCoreAssert(this->p2_radius() > 0);

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Circle implCircle = _ZFP_P2ShapeCircle_implDef(this, shapeScale);
    _ZFP_P2Shape_d->implShapeId = b2CreateCircleShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implCircle
            );
}

ZFOBJECT_REGISTER(P2ShapeCapsule)
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeCapsule, ZFUIPoint, p2_position0) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeCapsule, ZFUIPoint, p2_position1) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2ShapeCapsule, zffloat, p2_radius) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "must not be changed after added to world"
            );
}
static b2Capsule _ZFP_P2ShapeCapsule_implDef(ZF_IN P2ShapeCapsule *shape, ZF_IN zffloat shapeScale) {
    b2Capsule implCapsule;
    implCapsule.center1 = b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_position0(), shapeScale));
    implCapsule.center2 = b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_position1(), shapeScale));
    implCapsule.radius = shape->p2_radius() * shapeScale;
    return implCapsule;
}
ZFUIRect P2ShapeCapsule::p2impl_AABBLocal(void) {
    b2Capsule impl = _ZFP_P2ShapeCapsule_implDef(this, 1);
    return b2AABBToZF(b2ComputeCapsuleAABB(&impl, b2Transform_identity));
}
void P2ShapeCapsule::p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale) {
    zfsuper::p2impl_shapeCreate(ownerBody, shapeScale);
    ZFCoreAssert(this->p2_radius() > 0);

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Capsule implCapsule = _ZFP_P2ShapeCapsule_implDef(this, shapeScale);
    _ZFP_P2Shape_d->implShapeId = b2CreateCapsuleShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implCapsule
            );
}

ZFOBJECT_REGISTER(P2ShapePolygon)
ZFMETHOD_DEFINE_0(P2ShapePolygon, zfindex, p2_maxVertex) {
    return B2_MAX_POLYGON_VERTICES;
}
ZFMETHOD_DEFINE_4(P2ShapePolygon, void, p2_polygon
        , ZFMP_IN(const ZFCoreArray<ZFUIPoint> &, points)
        , ZFMP_IN_OPT(const ZFUIPoint &, position, ZFUIPointZero())
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(zffloat, radius, 0)
        ) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId)
            , "[P2ShapePolygon] must not be changed after added to world: %s::p2_polygon(%s, %s, %s, %s)"
            , this
            , points
            , position
            , rotation
            , radius
            );
    ZFCoreAssertWithMessageTrim(points.count() >= 3 && points.count() <= B2_MAX_POLYGON_VERTICES
            , "[P2ShapePolygon] points count must in range [3, %s]: %s::p2_polygon(%s, %s, %s, %s)"
            , (zfint)B2_MAX_POLYGON_VERTICES
            , this
            , points
            , position
            , rotation
            , radius
            );
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Shape_d->implShapeId));
    ZFCoreAssert(!points.isEmpty());
    this->p2_polygon_points(points);
    this->p2_polygon_position(position);
    this->p2_polygon_rotation(rotation);
    this->p2_polygon_radius(radius);
}
static b2Polygon _ZFP_P2ShapePolygon_implDef(ZF_IN P2ShapePolygon *shape, ZF_IN zffloat shapeScale) {
    b2Hull implHull;
    if(shapeScale != 1) {
        ZFCoreArray<ZFUIPoint> points;
        points.resize(shape->p2_polygon_points().count());
        for(zfindex i = points.count() - 1; i != zfindexMax(); --i) {
            points[i] = ZFUIPointApplyScale(shape->p2_polygon_points()[i], shapeScale);
        }
        implHull = b2ComputeHull((const b2Vec2 *)(points.arrayBuf()), (int)points.count());
    }
    else {
        ZFCoreArray<ZFUIPoint> points;
        points.resize(shape->p2_polygon_points().count());
        for(zfindex i = points.count() - 1; i != zfindexMax(); --i) {
            points[i] = shape->p2_polygon_points()[i];
        }
        implHull = b2ComputeHull((const b2Vec2 *)(points.arrayBuf()), (int)points.count());
    }
    ZFCoreAssertWithMessageTrim(implHull.count > 0
            , "invalid polygon points, shape: %s"
            , shape->p2_polygon_points().objectInfoOfContent()
            , shape
            );
    return shape->p2_polygon_position() == ZFUIPointZero() && shape->p2_polygon_rotation() == 0
        ? b2MakePolygon(
                &implHull
                , shape->p2_polygon_radius() * shapeScale
                )
        : b2MakeOffsetRoundedPolygon(
                &implHull
                , b2Vec2FromZF(ZFUIPointApplyScale(shape->p2_polygon_position(), shapeScale))
                , b2RotFromZF(shape->p2_polygon_rotation())
                , shape->p2_polygon_radius() * shapeScale
                )
        ;
}
ZFUIRect P2ShapePolygon::p2impl_AABBLocal(void) {
    b2Polygon impl = _ZFP_P2ShapePolygon_implDef(this, 1);
    return b2AABBToZF(b2ComputePolygonAABB(&impl, b2Transform_identity));
}
void P2ShapePolygon::p2impl_shapeCreate(ZF_IN P2Body *ownerBody, ZF_IN zffloat shapeScale) {
    zfsuper::p2impl_shapeCreate(ownerBody, shapeScale);
    ZFCoreAssert(!this->p2_polygon_points().isEmpty());

    b2ShapeDef implShapeDef = b2DefaultShapeDef();
    _ZFP_P2Shape_implShapeDef(implShapeDef, this, ownerBody);
    b2Polygon implPolygon = _ZFP_P2ShapePolygon_implDef(this, shapeScale);
    _ZFP_P2Shape_d->implShapeId = b2CreatePolygonShape(
            ownerBody->_ZFP_P2Body_d->implBodyId
            , &implShapeDef
            , &implPolygon
            );
}

// ============================================================
ZFOBJECT_REGISTER(P2Joint)

ZFMETHOD_DEFINE_0(P2Joint, P2Body *, p2_ownerBody0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2BodyId implBodyId = b2Joint_GetBodyA(_ZFP_P2Joint_d->implJointId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Joint, P2Body *, p2_ownerBody1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2BodyId implBodyId = b2Joint_GetBodyB(_ZFP_P2Joint_d->implJointId);
        ZFCoreAssert(B2_IS_NON_NULL(implBodyId));
        return (P2Body *)b2Body_GetUserData(implBodyId);
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Joint, P2World *, p2_ownerWorld) {
    if(_ZFP_P2Joint_d->ownerWorld) {
        return _ZFP_P2Joint_d->ownerWorld;
    }
    else if(_ZFP_P2Joint_d->ownerBody) {
        return _ZFP_P2Joint_d->ownerBody->p2_ownerWorld();
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Joint, ZFObject *, p2_owner) {
    return _ZFP_P2Joint_d->ownerWorld
        ? zfcast(ZFObject *, _ZFP_P2Joint_d->ownerWorld)
        : zfcast(ZFObject *, _ZFP_P2Joint_d->ownerBody)
        ;
}

ZFMETHOD_DEFINE_0(P2Joint, void, p2_removeFromParent) {
    if(_ZFP_P2Joint_d->ownerWorld) {
        _ZFP_P2Joint_d->ownerWorld->p2_jointRemove(this);
    }
    else if(_ZFP_P2Joint_d->ownerBody) {
        _ZFP_P2Joint_d->ownerBody->p2_jointRemove(this);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Joint, zfstring, p2_bodyId0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(propertyValue != propertyValueOld) {
            this->p2_ownerWorld()->_ZFP_P2World_d->pendingJoint[this] = zftrue;
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Joint, zfstring, p2_bodyId1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(propertyValue != propertyValueOld) {
            this->p2_ownerWorld()->_ZFP_P2World_d->pendingJoint[this] = zftrue;
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Joint, zfbool, p2_contactEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetCollideConnected(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}

ZFMETHOD_DEFINE_0(P2Joint, zfbool, p2_implAttached) {
    return B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId);
}

ZFOBJECT_ON_INIT_DEFINE_2(P2Joint
        , ZFMP_IN(const zfstring &, bodyId0)
        , ZFMP_IN(const zfstring &, bodyId1)
        ) {
    this->p2_bodyId0(bodyId0);
    this->p2_bodyId1(bodyId1);
}
void P2Joint::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Joint_d = zfpoolNew(_ZFP_P2JointPrivate);
}
void P2Joint::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Joint_d->implJointId));
    zfpoolDelete(_ZFP_P2Joint_d);
    zfsuper::objectOnDealloc();
}

static void _ZFP_P2Joint_implJointDef(ZF_IN P2Joint *owner, ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(owner->_ZFP_P2Joint_d->implJointId)
            , "joint already attached to body, joint: %s, body0: %s, body1: %s, world: %s, adding to body0: %d, body1: %s"
            , owner
            , owner->p2_ownerBody0()
            , owner->p2_ownerBody1()
            , owner->p2_ownerWorld()
            , ownerBody0
            , ownerBody1
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , "joint ownerBody0 not attached to world, joint: %s, body: %s"
            , owner
            , ownerBody0
            );
    ZFCoreAssertWithMessageTrim(B2_IS_NON_NULL(ownerBody1->_ZFP_P2Body_d->implBodyId)
            , "joint ownerBody1 not attached to world, joint: %s, body: %s"
            , owner
            , ownerBody1
            );
}

ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzSoft, 4)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingSoft, 0.5f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzNormal, 8)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingNormal, 0.7f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringHertzHard, 12)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointSpringDampingHard, 0.9f)

ZFOBJECT_REGISTER(P2JointSpring)

// ============================================================
ZFOBJECT_REGISTER(P2JointDistance)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody0()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody1()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2DistanceJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2DistanceJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale);
        b2DistanceJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2DistanceJoint_SetMaxMotorForce(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
        b2DistanceJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2DistanceJoint_SetLength(_ZFP_P2Joint_d->implJointId, propertyValue * _ZFP_P2JointPrivate::jointScale(this));
    }
}
ZFMETHOD_DEFINE_0(P2JointDistance, zffloat, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2DistanceJoint_GetCurrentLength(_ZFP_P2Joint_d->implJointId);
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() >= 0) {
                b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , this->p2_distanceLimitMax() >= this->p2_distanceLimitMin()
                                ? (float)(this->p2_distanceLimitMax() * jointScale)
                                : (float)(this->p2_distanceLimitMin() * jointScale)
                        );
            }
            else {
                b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                        , (float)0
                        , this->p2_distanceLimitMax() >= 0 ? (float)(this->p2_distanceLimitMax() * jointScale) : (float)0
                        );
            }
        }
        else {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointDistance, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() >= 0) {
                b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , this->p2_distanceLimitMax() >= this->p2_distanceLimitMin()
                                ? (float)(this->p2_distanceLimitMax() * jointScale)
                                : (float)(this->p2_distanceLimitMin() * jointScale)
                        );
            }
            else {
                b2DistanceJoint_SetLengthRange(_ZFP_P2Joint_d->implJointId
                        , (float)0
                        , this->p2_distanceLimitMax() >= 0 ? (float)(this->p2_distanceLimitMax() * jointScale) : (float)0
                        );
            }
        }
        else {
            b2DistanceJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointDistance::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);
    zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);

    b2DistanceJointDef implJointDef = b2DefaultDistanceJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor0(), ownerBody0->p2_bodyScaleInherit()));
    implJointDef.localAnchorB = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor1(), ownerBody1->p2_bodyScaleInherit()));
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed() * jointScale;
        implJointDef.maxMotorForce = this->p2_motorForce() * jointScale * jointScale;
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.length = (float)(this->p2_distance() * jointScale);
    if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
        implJointDef.enableLimit = zftrue;
        if(this->p2_distanceLimitMin() >= 0) {
            implJointDef.minLength = (float)(this->p2_distanceLimitMin() * jointScale);
            implJointDef.maxLength = this->p2_distanceLimitMax() >= this->p2_distanceLimitMin()
                ? (float)(this->p2_distanceLimitMax() * jointScale)
                : (float)(this->p2_distanceLimitMin() * jointScale)
                ;
        }
        else {
            implJointDef.minLength = (float)0;
            implJointDef.maxLength = this->p2_distanceLimitMax() >= 0 ? (float)(this->p2_distanceLimitMax() * jointScale) : (float)0;
        }
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateDistanceJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointRevolute)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody0()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody1()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2RevoluteJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2RevoluteJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale);
        b2RevoluteJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2RevoluteJoint_SetMaxMotorTorque(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
        b2RevoluteJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angular) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2RevoluteJoint_SetTargetAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFMETHOD_DEFINE_0(P2JointRevolute, zffloat, p2_angularCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2RadToZF(b2RevoluteJoint_GetAngle(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_angular();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_angularLimitMin() != -P2_MAX() || this->p2_angularLimitMax() != P2_MAX()) {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_angularLimitMin() <= this->p2_angularLimitMax()) {
                b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , b2RadFromZF(this->p2_angularLimitMax())
                        , b2RadFromZF(this->p2_angularLimitMin())
                        );
            }
            else {
                b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , b2RadFromZF(this->p2_angularLimitMin())
                        , b2RadFromZF(this->p2_angularLimitMin())
                        );
            }
        }
        else {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointRevolute, zffloat, p2_angularLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_angularLimitMin() != -P2_MAX() || this->p2_angularLimitMax() != P2_MAX()) {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_angularLimitMin() <= this->p2_angularLimitMax()) {
                b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , b2RadFromZF(this->p2_angularLimitMax())
                        , b2RadFromZF(this->p2_angularLimitMin())
                        );
            }
            else {
                b2RevoluteJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , b2RadFromZF(this->p2_angularLimitMin())
                        , b2RadFromZF(this->p2_angularLimitMin())
                        );
            }
        }
        else {
            b2RevoluteJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointRevolute::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);
    zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);

    b2RevoluteJointDef implJointDef = b2DefaultRevoluteJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor0(), ownerBody0->p2_bodyScaleInherit()));
    implJointDef.localAnchorB = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor1(), ownerBody1->p2_bodyScaleInherit()));
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed() * jointScale;
        implJointDef.maxMotorTorque = this->p2_motorForce() * jointScale * jointScale;
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.referenceAngle = b2RadFromZF(ownerBody1->p2_rotation() - ownerBody0->p2_rotation() + this->p2_angularRef());
    implJointDef.targetAngle = b2RadFromZF(this->p2_angular());
    if(this->p2_angularLimitMin() != -P2_MAX() || this->p2_angularLimitMax() != P2_MAX()) {
        implJointDef.enableLimit = zftrue;
        if(this->p2_angularLimitMin() <= this->p2_angularLimitMax()) {
            implJointDef.lowerAngle = b2RadFromZF(this->p2_angularLimitMax());
        }
        else {
            implJointDef.lowerAngle = b2RadFromZF(this->p2_angularLimitMin());
        }
        implJointDef.upperAngle = b2RadFromZF(this->p2_angularLimitMin());
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateRevoluteJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointPrismatic)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody0()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody1()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2PrismaticJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2PrismaticJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale);
        b2PrismaticJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2PrismaticJoint_SetMaxMotorForce(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
        b2PrismaticJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, ZFUIPoint, p2_axis) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAxisA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2PrismaticJoint_SetTargetTranslation(_ZFP_P2Joint_d->implJointId, propertyValue * _ZFP_P2JointPrivate::jointScale(this));
    }
}
ZFMETHOD_DEFINE_0(P2JointPrismatic, zffloat, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2PrismaticJoint_GetTranslation(_ZFP_P2Joint_d->implJointId);
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
                b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , (float)(this->p2_distanceLimitMax() * jointScale)
                        );
            }
            else {
                b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        );
            }
        }
        else {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointPrismatic, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
                b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , (float)(this->p2_distanceLimitMax() * jointScale)
                        );
            }
            else {
                b2PrismaticJoint_SetLimits(_ZFP_P2Joint_d->implJointId
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        , (float)(this->p2_distanceLimitMin() * jointScale)
                        );
            }
        }
        else {
            b2PrismaticJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zffalse);
        }
    }
}

void P2JointPrismatic::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);
    zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);

    b2PrismaticJointDef implJointDef = b2DefaultPrismaticJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor0(), ownerBody0->p2_bodyScaleInherit()));
    implJointDef.localAnchorB = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor1(), ownerBody1->p2_bodyScaleInherit()));
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed() * jointScale;
        implJointDef.maxMotorForce = this->p2_motorForce() * jointScale * jointScale;
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.localAxisA = b2Vec2FromZF(this->p2_axis());
    implJointDef.referenceAngle = b2RadFromZF(ownerBody1->p2_rotation() - ownerBody0->p2_rotation() + this->p2_angularRef());
    implJointDef.targetTranslation = this->p2_distance() * jointScale;
    if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
        implJointDef.enableLimit = zftrue;
        if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
            implJointDef.lowerTranslation = (float)(this->p2_distanceLimitMin() * jointScale);
            implJointDef.upperTranslation = (float)(this->p2_distanceLimitMax() * jointScale);
        }
        else {
            implJointDef.lowerTranslation = (float)(this->p2_distanceLimitMin() * jointScale);
            implJointDef.upperTranslation = (float)(this->p2_distanceLimitMin() * jointScale);
        }
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreatePrismaticJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointWheel)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody0()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody1()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_springHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetSpringHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_springDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WheelJoint_SetSpringDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
        b2WheelJoint_EnableSpring(_ZFP_P2Joint_d->implJointId, this->p2_springHertz() > 0 && this->p2_springDamping() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_motorSpeed) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2WheelJoint_SetMotorSpeed(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale);
        b2WheelJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2WheelJoint_SetMaxMotorTorque(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
        b2WheelJoint_EnableMotor(_ZFP_P2Joint_d->implJointId, this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, ZFUIPoint, p2_axis) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAxisA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_distanceLimitMin) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
                b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, (float)(this->p2_distanceLimitMin() * jointScale), (float)(this->p2_distanceLimitMax() * jointScale));
            }
            else {
                b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, (float)(this->p2_distanceLimitMin() * jointScale), (float)(this->p2_distanceLimitMin() * jointScale));
            }
        }
        else {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWheel, zffloat, p2_distanceLimitMax) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
            zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
            if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
                b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, (float)(this->p2_distanceLimitMin() * jointScale), (float)(this->p2_distanceLimitMax() * jointScale));
            }
            else {
                b2WheelJoint_SetLimits(_ZFP_P2Joint_d->implJointId, (float)(this->p2_distanceLimitMin() * jointScale), (float)(this->p2_distanceLimitMin() * jointScale));
            }
        }
        else {
            b2WheelJoint_EnableLimit(_ZFP_P2Joint_d->implJointId, zftrue);
        }
    }
}

void P2JointWheel::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);
    zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);

    b2WheelJointDef implJointDef = b2DefaultWheelJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor0(), ownerBody0->p2_bodyScaleInherit()));
    implJointDef.localAnchorB = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor1(), ownerBody1->p2_bodyScaleInherit()));
    if(this->p2_springHertz() > 0 && this->p2_springDamping() > 0) {
        implJointDef.enableSpring = zftrue;
        implJointDef.hertz = this->p2_springHertz();
        implJointDef.dampingRatio = this->p2_springDamping();
    }
    else {
        implJointDef.enableSpring = zffalse;
    }
    if(this->p2_motorSpeed() > 0 && this->p2_motorForce() > 0) {
        implJointDef.enableMotor = zftrue;
        implJointDef.motorSpeed = this->p2_motorSpeed() * jointScale;
        implJointDef.maxMotorTorque = this->p2_motorForce() * jointScale * jointScale;
    }
    else {
        implJointDef.enableMotor = zffalse;
    }

    implJointDef.localAxisA = b2Vec2FromZF(this->p2_axis());
    if(this->p2_distanceLimitMin() != -P2_MAX() || this->p2_distanceLimitMax() != P2_MAX()) {
        implJointDef.enableLimit = zftrue;
        if(this->p2_distanceLimitMin() <= this->p2_distanceLimitMax()) {
            implJointDef.lowerTranslation = this->p2_distanceLimitMin() * jointScale;
            implJointDef.upperTranslation = this->p2_distanceLimitMax() * jointScale;
        }
        else {
            implJointDef.lowerTranslation = this->p2_distanceLimitMin() * jointScale;
            implJointDef.upperTranslation = this->p2_distanceLimitMin() * jointScale;
        }
    }
    else {
        implJointDef.enableLimit = zffalse;
    }
    _ZFP_P2Joint_d->implJointId = b2CreateWheelJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorSoft, 0.2f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorNormal, 0.3f)
ZFEXPORT_VAR_DEFINE(zffloat, P2JointMotorFactorHard, 0.5f)

ZFOBJECT_REGISTER(P2JointMotor)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorFactor) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetCorrectionFactor(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2MotorJoint_SetMaxForce(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_motorTorque) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);
        b2MotorJoint_SetMaxTorque(_ZFP_P2Joint_d->implJointId, propertyValue * jointScale * jointScale);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, ZFUIPoint, p2_distance) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetLinearOffset(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, _ZFP_P2JointPrivate::jointScale(this))));
    }
}
ZFMETHOD_DEFINE_0(P2JointMotor, ZFUIPoint, p2_distanceCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2Vec2ToZF(b2MotorJoint_GetLinearOffset(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_distance();
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMotor, zffloat, p2_angular) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MotorJoint_SetAngularOffset(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFMETHOD_DEFINE_0(P2JointMotor, zffloat, p2_angularCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        return b2RadToZF(b2MotorJoint_GetAngularOffset(_ZFP_P2Joint_d->implJointId));
    }
    else {
        return this->p2_angular();
    }
}

void P2JointMotor::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);
    zffloat jointScale = _ZFP_P2JointPrivate::jointScale(this);

    b2MotorJointDef implJointDef = b2DefaultMotorJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();

    implJointDef.correctionFactor = this->p2_motorFactor();
    implJointDef.maxForce = this->p2_motorForce() * jointScale * jointScale;
    implJointDef.maxTorque = this->p2_motorTorque() * jointScale * jointScale;
    implJointDef.linearOffset = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_distance(), ownerBody1->p2_bodyScaleInherit()));
    implJointDef.angularOffset = b2RadFromZF(this->p2_angular());
    _ZFP_P2Joint_d->implJointId = b2CreateMotorJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointWeld)

ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, ZFUIPoint, p2_anchor0) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody0()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, ZFUIPoint, p2_anchor1) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetLocalAnchorA(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(ZFUIPointApplyScale(propertyValue, this->p2_ownerBody1()->p2_bodyScaleInherit())));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularRef) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2Joint_SetReferenceAngle(_ZFP_P2Joint_d->implJointId, b2RadFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_distanceHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetLinearHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_distanceDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetLinearDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularHertz) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetAngularHertz(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointWeld, zffloat, p2_angularDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2WeldJoint_SetAngularDampingRatio(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}

void P2JointWeld::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2WeldJointDef implJointDef = b2DefaultWeldJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.collideConnected = this->p2_contactEnable();
    implJointDef.localAnchorA = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor0(), ownerBody0->p2_bodyScaleInherit()));
    implJointDef.localAnchorB = b2Vec2FromZF(ZFUIPointApplyScale(this->p2_anchor1(), ownerBody1->p2_bodyScaleInherit()));
    implJointDef.referenceAngle = b2RadFromZF(ownerBody1->p2_rotation() - ownerBody0->p2_rotation() + this->p2_angularRef());
    implJointDef.linearHertz = this->p2_distanceHertz();
    implJointDef.linearDampingRatio = this->p2_distanceDamping();
    implJointDef.angularHertz = this->p2_angularHertz();
    implJointDef.angularDampingRatio = this->p2_angularDamping();
    _ZFP_P2Joint_d->implJointId = b2CreateWeldJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointFilter)
void P2JointFilter::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2FilterJointDef implJointDef = b2DefaultFilterJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    _ZFP_P2Joint_d->implJointId = b2CreateFilterJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

ZFOBJECT_REGISTER(P2JointMouse)
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMouse, ZFUIPoint, p2_position) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MouseJoint_SetTarget(_ZFP_P2Joint_d->implJointId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2JointMouse, zffloat, p2_motorForce) {
    if(B2_IS_NON_NULL(_ZFP_P2Joint_d->implJointId)) {
        b2MouseJoint_SetMaxForce(_ZFP_P2Joint_d->implJointId, propertyValue);
    }
}
void P2JointMouse::p2impl_jointCreate(ZF_IN P2Body *ownerBody0, ZF_IN P2Body *ownerBody1) {
    zfsuper::p2impl_jointCreate(ownerBody0, ownerBody1);
    _ZFP_P2Joint_implJointDef(this, ownerBody0, ownerBody1);

    b2MouseJointDef implJointDef = b2DefaultMouseJointDef();
    implJointDef.bodyIdA = ownerBody0->_ZFP_P2Body_d->implBodyId;
    implJointDef.bodyIdB = ownerBody1->_ZFP_P2Body_d->implBodyId;
    implJointDef.target = b2Vec2FromZF(this->p2_position());
    implJointDef.maxForce = this->p2_motorForce();
    _ZFP_P2Joint_d->implJointId = b2CreateMouseJoint(
            b2Body_GetWorld(ownerBody0->_ZFP_P2Body_d->implBodyId)
            , &implJointDef
            );
}

// ============================================================
ZFENUM_DEFINE(P2BodyType)

ZFTYPEID_DEFINE_BY_STRING_CONVERTER(P2BodyTransform, P2BodyTransform, {
        ZFCoreArray<ZFIndexRange> pos;
        if(!ZFCoreDataPairSplitString(pos, 3, src, srcLen)) {
            zfstringAppend(errorHint, "invalid data: %s", zftext(src, srcLen));
            return zffalse;
        }
        if(zffalse
                || !ZFUIPointFromStringT(v.p2_position, zftext(src + pos[0].start, pos[0].count), errorHint)
                || !zffloatFromStringT(v.p2_rotation, zftext(src + pos[1].start, pos[1].count), errorHint)
                || !zffloatFromStringT(v.p2_bodyScale, zftext(src + pos[2].start, pos[2].count), errorHint)
                ) {
            return zffalse;
        }
        return zftrue;
    }, {
        s += "(";
        if(zffalse
                || !ZFUIPointToStringT(s, v.p2_position, errorHint) || (s.append(','), zffalse)
                || !zffloatToStringT(s, v.p2_rotation, errorHint) || (s.append(','), zffalse)
                || !zffloatToStringT(s, v.p2_bodyScale, errorHint) || (s.append(','), zffalse)
                ) {
            return zffalse;
        }
        s += ")";
        return zftrue;
    })
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR(v_P2BodyTransform, ZFUIPoint, p2_position)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR(v_P2BodyTransform, zffloat, p2_rotation)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR(v_P2BodyTransform, zffloat, p2_bodyScale)

zfclassNotPOD _ZFP_P2BodyTransfrom {
public:
    static ZFCoreArray<P2BodyTransform> transforms(ZF_IN P2Body *body) {
        ZFCoreArray<P2BodyTransform> ret;
        while(body) {
            P2BodyTransform &item = ret.add();
            item.p2_position = body->p2_position();
            item.p2_rotation = body->p2_rotation();
            item.p2_bodyScale = body->p2_bodyScale();
            body = body->_ZFP_P2Body_d->ownerBody;
        }
        return ret;
    }
    static void toLocal(
            ZF_OUT P2BodyTransform &transformLocal
            , ZF_IN const P2BodyTransform &transformGlobal
            , ZF_IN const ZFCoreArray<P2BodyTransform> &transforms
            ) {
        P2BodyTransform parentGlobal;
        _parentsToGlobal(parentGlobal, transforms);
        _calcInv(transformLocal, transformGlobal, parentGlobal);
    }
    static void toGlobal(
            ZF_OUT P2BodyTransform &transformGlobal
            , ZF_IN const P2BodyTransform &transformLocal
            , ZF_IN const ZFCoreArray<P2BodyTransform> &transforms
            ) {
        P2BodyTransform parentGlobal;
        _parentsToGlobal(parentGlobal, transforms);
        _calc(transformGlobal, transformLocal, parentGlobal);
    }
private:
    static zffloat _scaleFix(ZF_IN zffloat scale) {
        return scale > 0 ? scale : (zffloat)1;
    }
    static void _rotCW(ZF_OUT zffloat &c, ZF_OUT zffloat &s, ZF_IN zffloat deg) {
        zffloat a = deg * zfm_PI() / 180.0f;
        c = cos(a);
        s = sin(a);
    }
    static void _applyR(ZF_OUT zffloat &ox, ZF_OUT zffloat &oy, ZF_IN zffloat c, ZF_IN zffloat s, ZF_IN zffloat x, ZF_IN zffloat y) {
        ox =  c * x + s * y;
        oy = -s * x + c * y;
    }
    static void _applyRInv(ZF_OUT zffloat &ox, ZF_OUT zffloat &oy, ZF_IN zffloat c, ZF_IN zffloat s, ZF_IN zffloat x, ZF_IN zffloat y) {
        ox = c * x - s * y;
        oy = s * x + c * y;
    }
    static void _calc(ZF_OUT P2BodyTransform &childGlobal, ZF_IN const P2BodyTransform &childLocal, ZF_IN const P2BodyTransform &parentGlobal) {
        P2BodyTransform P = parentGlobal;
        P2BodyTransform C = childLocal;
        zffloat c, s;
        _rotCW(c, s, P.p2_rotation);

        zffloat px, py;
        _applyR(px, py, c, s, C.p2_position.x, C.p2_position.y);
        childGlobal.p2_position.x = P.p2_position.x + _scaleFix(P.p2_bodyScale) * px;
        childGlobal.p2_position.y = P.p2_position.y + _scaleFix(P.p2_bodyScale) * py;

        childGlobal.p2_rotation = P.p2_rotation + C.p2_rotation;
        childGlobal.p2_bodyScale = _scaleFix(P.p2_bodyScale) * _scaleFix(C.p2_bodyScale);
    }
    static void _calcInv(ZF_OUT P2BodyTransform &childLocal, ZF_IN const P2BodyTransform &childGlobal, ZF_IN const P2BodyTransform &parentGlobal) {
        P2BodyTransform P = parentGlobal;
        P2BodyTransform C = childGlobal;

        zffloat invS = 1.0f / _scaleFix(P.p2_bodyScale);
        zffloat c, s;
        _rotCW(c, s, P.p2_rotation);

        zffloat ux = (C.p2_position.x - P.p2_position.x) * invS;
        zffloat uy = (C.p2_position.y - P.p2_position.y) * invS;
        _applyRInv(childLocal.p2_position.x, childLocal.p2_position.y, c, s, ux, uy);

        childLocal.p2_rotation = C.p2_rotation - P.p2_rotation;
        childLocal.p2_bodyScale = _scaleFix(C.p2_bodyScale) * invS;
    }
    static void _parentsToGlobal(ZF_OUT P2BodyTransform &parentGlobal, ZF_IN const ZFCoreArray<P2BodyTransform> &parents) {
        P2BodyTransform t;
        zfmemset(&t, 0, sizeof(P2BodyTransform));
        t.p2_bodyScale = 1;
        for(zfindex i = parents.count() - 1; i != zfindexMax(); --i) {
            _calc(t, parents[i], t);
        }
        parentGlobal = t;
    }
};
ZFMETHOD_FUNC_DEFINE_3(void, P2BodyTransformToLocal
        , ZFMP_OUT(P2BodyTransform &, transformLocal)
        , ZFMP_IN(const P2BodyTransform &, transformGlobal)
        , ZFMP_IN(const ZFCoreArray<P2BodyTransform> &, transforms)
        ) {
    _ZFP_P2BodyTransfrom::toLocal(transformLocal, transformGlobal, transforms);
}
ZFMETHOD_FUNC_DEFINE_3(void, P2BodyTransformToGlobal
        , ZFMP_OUT(P2BodyTransform &, transformGlobal)
        , ZFMP_IN(const P2BodyTransform &, transformLocal)
        , ZFMP_IN(const ZFCoreArray<P2BodyTransform> &, transforms)
        ) {
    _ZFP_P2BodyTransfrom::toGlobal(transformGlobal, transformLocal, transforms);
}

ZFMETHOD_FUNC_DEFINE_3(ZFUIPoint, P2BodyMassPosFromOriginPos
        , ZFMP_IN(const ZFUIPoint &, originPos)
        , ZFMP_IN(const ZFUIPoint &, massCenter)
        , ZFMP_IN(zffloat, rotation)
        ) {
    zffloat rad = rotation * zfm_PI() / 180.0f;
    zffloat rCos = zfm_cos(rad);
    zffloat rSin = zfm_sin(rad);
    return ZFUIPointCreate(
            originPos.x + massCenter.x * rCos + massCenter.y * rSin
            , originPos.y - massCenter.x * rSin + massCenter.y * rCos
            );
}
ZFMETHOD_FUNC_DEFINE_3(ZFUIPoint, P2BodyMassPosToOriginPos
        , ZFMP_IN(const ZFUIPoint &, massPos)
        , ZFMP_IN(const ZFUIPoint &, massCenter)
        , ZFMP_IN(zffloat, rotation)
        ) {
    zffloat rad = rotation * zfm_PI() / 180.0f;
    zffloat rCos = zfm_cos(rad);
    zffloat rSin = zfm_sin(rad);
    return ZFUIPointCreate(
            massPos.x - massCenter.x * rCos - massCenter.y * rSin
            , massPos.y + massCenter.x * rSin - massCenter.y * rCos
            );
}

// ============================================================
ZFOBJECT_REGISTER(P2Body)

ZFMETHOD_DEFINE_0(P2Body, P2Body *, p2_ownerBody) {
    return _ZFP_P2Body_d->ownerBody;
}
ZFMETHOD_DEFINE_0(P2Body, P2World *, p2_ownerWorld) {
    if(_ZFP_P2Body_d->ownerWorld) {
        return _ZFP_P2Body_d->ownerWorld;
    }
    else if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2WorldId implWorldId = b2Body_GetWorld(_ZFP_P2Body_d->implBodyId);
        ZFCoreAssert(B2_IS_NON_NULL(implWorldId));
        return (P2World *)(b2World_GetUserData(implWorldId));
    }
    else if(_ZFP_P2Body_d->ownerBody) {
        return _ZFP_P2Body_d->ownerBody->p2_ownerWorld();
    }
    else {
        return zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFObject *, p2_owner) {
    return _ZFP_P2Body_d->ownerWorld
        ? zfcast(ZFObject *, _ZFP_P2Body_d->ownerWorld)
        : zfcast(ZFObject *, _ZFP_P2Body_d->ownerBody)
        ;
}

ZFMETHOD_DEFINE_0(P2Body, void, p2_removeFromParent) {
    if(_ZFP_P2Body_d->ownerWorld) {
        _ZFP_P2Body_d->ownerWorld->p2_bodyRemove(this);
    }
    else if(_ZFP_P2Body_d->ownerBody) {
        _ZFP_P2Body_d->ownerBody->p2_bodyRemove(this);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfstring, p2_bodyId) {
    ZFCoreAssertWithMessageTrim(B2_IS_NULL(_ZFP_P2Body_d->implBodyId)
            , "not allowed to change bodyId after adding to world, body: %s, new bodyId: %s"
            , this
            , propertyValue
            );
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, P2BodyType, p2_type) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2BodyType implType;
        switch(propertyValue) {
            case v_P2BodyType::e_Static:
                implType = b2_staticBody;
                break;
            case v_P2BodyType::e_Kinematic:
                implType = b2_kinematicBody;
                break;
            case v_P2BodyType::e_Dynamic:
                implType = b2_dynamicBody;
                break;
            default:
                ZFCoreCriticalShouldNotGoHere();
                return;
        }
        b2Body_SetType(_ZFP_P2Body_d->implBodyId, implType);
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_bodyScale) {
    if(propertyValue <= 0) {
        propertyValue = 1;
    }
    if(propertyValue != propertyValueOld) {
        ZFBitSet(_ZFP_P2Body_d->stateFlag, 0
                | _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested
                | _ZFP_P2BodyPrivate::stateFlag_bodyScaleUpdateRequested
                | _ZFP_P2BodyPrivate::stateFlag_massUpdateRequested
                );
        P2World *world = this->p2_ownerWorld();
        if(world) {
            world->_ZFP_P2World_d->pendingBody[this] = zftrue;
        }
    }
}

ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_bodyScaleInherit) {
    if(_ZFP_P2Body_d->ownerBody) {
        zffloat v = 1;
        for(P2Body *body = _ZFP_P2Body_d->ownerBody; body; body = body->_ZFP_P2Body_d->ownerBody) {
            v *= body->p2_bodyScale();
        }
        return v;
    }
    else {
        return 1;
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_bodyScaleFixed) {
    if(_ZFP_P2Body_d->ownerBody) {
        zffloat v = this->p2_bodyScale();
        for(P2Body *body = _ZFP_P2Body_d->ownerBody; body; body = body->_ZFP_P2Body_d->ownerBody) {
            v *= body->p2_bodyScale();
        }
        return v;
    }
    else {
        return this->p2_bodyScale();
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_enable) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        if(propertyValue) {
            b2Body_Enable(_ZFP_P2Body_d->implBodyId);
        }
        else {
            b2Body_Disable(_ZFP_P2Body_d->implBodyId);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_sleepEnable) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_EnableSleep(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_bullet) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetBullet(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zfbool, p2_rotationFixed) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetFixedRotation(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_gravityScale) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetGravityScale(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, ZFUIPoint, p2_position) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        this->p2_ownerWorld()->_ZFP_P2World_d->pendingBodyMove[this] = zftrue;
        if(_ZFP_P2Body_d->ownerBody) {
            ZFCoreArray<P2BodyTransform> parents = _ZFP_P2Body_d->ownerBody->p2_transforms();

            P2BodyTransform g;
            b2Transform implTransform = b2Body_GetTransform(_ZFP_P2Body_d->implBodyId);
            g.p2_position = b2Vec2ToZF(implTransform.p);
            g.p2_rotation = b2RotToZF(implTransform.q);
            g.p2_bodyScale = this->p2_bodyScale();

            P2BodyTransform l;
            P2BodyTransformToLocal(l, g, parents);
            l.p2_position = propertyValue;

            P2BodyTransformToGlobal(g, l, parents);
            b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(g.p2_position), b2RotFromZF(g.p2_rotation));
        }
        else {
            b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(propertyValue), b2Body_GetTransform(_ZFP_P2Body_d->implBodyId).q);
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, ZFUIPoint, p2_positionVelocity) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetLinearVelocity(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_positionDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetLinearDamping(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotation) {
    propertyValue = b2DegreeNormalize(propertyValue);

    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        this->p2_ownerWorld()->_ZFP_P2World_d->pendingBodyMove[this] = zftrue;
        if(_ZFP_P2Body_d->ownerBody) {
            ZFCoreArray<P2BodyTransform> parents = _ZFP_P2Body_d->ownerBody->p2_transforms();

            P2BodyTransform g;
            b2Transform implTransform = b2Body_GetTransform(_ZFP_P2Body_d->implBodyId);
            g.p2_position = b2Vec2ToZF(implTransform.p);
            g.p2_rotation = b2RotToZF(implTransform.q);
            g.p2_bodyScale = this->p2_bodyScale();

            P2BodyTransform l;
            P2BodyTransformToLocal(l, g, parents);
            l.p2_rotation = propertyValue;

            P2BodyTransformToGlobal(g, l, parents);
            b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(g.p2_position), b2RotFromZF(g.p2_rotation));
        }
        else {
            b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Body_GetTransform(_ZFP_P2Body_d->implBodyId).p, b2RotFromZF(propertyValue));
        }
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotationVelocity) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAngularVelocity(_ZFP_P2Body_d->implBodyId, b2AngularVelocityFromZF(propertyValue));
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2Body, zffloat, p2_rotationDamping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAngularDamping(_ZFP_P2Body_d->implBodyId, propertyValue);
    }
}

ZFMETHOD_DEFINE_0(P2Body, ZFUIRect, p2_AABB) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2AABBToZF(b2Body_ComputeAABB(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return ZFUIRectZero();
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIRect, p2_AABBLocal) {
    if(ZFBitTest(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested)) {
        _ZFP_P2Body_d->bodyPosUpdate();
        return _ZFP_P2Body_d->bodyAABBLocal;
    }
    else {
        return _ZFP_P2Body_d->bodyAABBLocal;
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_massCenter) {
    if(ZFBitTest(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested)) {
        _ZFP_P2Body_d->bodyPosUpdate();
        return _ZFP_P2Body_d->massCenter;
    }
    else {
        return _ZFP_P2Body_d->massCenter;
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_mass) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Body_GetMass(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return 0;
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationInertia) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Body_GetRotationalInertia(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return 0;
    }
}

ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_massPositionCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetWorldCenterOfMass(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return this->p2_positionCur();
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_massPositionCur
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        this->p2_ownerWorld()->_ZFP_P2World_d->pendingBodyMove[this] = zftrue;
        b2Body_SetTransform(_ZFP_P2Body_d->implBodyId
                , b2Vec2FromZF(P2BodyMassPosToOriginPos(v, ZFUIPointApplyScale(this->p2_massCenter(), this->p2_bodyScaleFixed()), this->p2_rotationCur()))
                , b2Body_GetRotation(_ZFP_P2Body_d->implBodyId)
                );
    }
}

ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_positionCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetPosition(_ZFP_P2Body_d->implBodyId));
    }
    else {
        if(_ZFP_P2Body_d->ownerBody) {
            P2BodyTransform l;
            l.p2_position = this->p2_position();
            P2BodyTransform g;
            P2BodyTransformToGlobal(g, l, _ZFP_P2Body_d->ownerBody->p2_transforms());
            return g.p2_position;
        }
        else {
            return this->p2_position();
        }
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_positionCur
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        this->p2_ownerWorld()->_ZFP_P2World_d->pendingBodyMove[this] = zftrue;
        b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(v), b2Body_GetRotation(_ZFP_P2Body_d->implBodyId));
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFUIPoint, p2_positionVelocityCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2Vec2ToZF(b2Body_GetLinearVelocity(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return this->p2_positionVelocity();
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_positionVelocityCur
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetLinearVelocity(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(v));
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2RotToZF(b2Body_GetRotation(_ZFP_P2Body_d->implBodyId));
    }
    else {
        if(_ZFP_P2Body_d->ownerBody) {
            P2BodyTransform l;
            l.p2_rotation = this->p2_rotation();
            P2BodyTransform g;
            P2BodyTransformToGlobal(g, l, _ZFP_P2Body_d->ownerBody->p2_transforms());
            return g.p2_rotation;
        }
        else {
            return this->p2_rotation();
        }
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_rotationCur
        , ZFMP_IN(zffloat, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        this->p2_ownerWorld()->_ZFP_P2World_d->pendingBodyMove[this] = zftrue;
        b2Body_SetTransform(_ZFP_P2Body_d->implBodyId, b2Body_GetPosition(_ZFP_P2Body_d->implBodyId), b2RotFromZF(v));
    }
}
ZFMETHOD_DEFINE_0(P2Body, zffloat, p2_rotationVelocityCur) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return b2AngularVelocityToZF(b2Body_GetAngularVelocity(_ZFP_P2Body_d->implBodyId));
    }
    else {
        return this->p2_rotationVelocity();
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_rotationVelocityCur
        , ZFMP_IN(zffloat, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAngularVelocity(_ZFP_P2Body_d->implBodyId, b2AngularVelocityFromZF(v));
    }
}

ZFMETHOD_DEFINE_2(P2Body, void, p2_pos
        , ZFMP_IN(const ZFUIPoint &, massPos)
        , ZFMP_IN_OPT(zffloat, rotation, P2_MAX())
        ) {
    if(massPos.x == P2_MAX() && massPos.y == P2_MAX()) {
        if(rotation != P2_MAX()) {
            this->p2_position(P2BodyMassPosToOriginPos(
                        P2BodyMassPosFromOriginPos(this->p2_position(), this->p2_massCenter(), this->p2_rotation())
                        , this->p2_massCenter()
                        , rotation
                        ));
            this->p2_rotation(rotation);
        }
    }
    else if(massPos.x == P2_MAX()) {
        ZFUIPoint t;
        t.x = P2BodyMassPosFromOriginPos(this->p2_position(), this->p2_massCenter(), this->p2_rotation()).x;
        t.y = massPos.y;
        if(rotation != P2_MAX()) {
            this->p2_position(P2BodyMassPosToOriginPos(t, this->p2_massCenter(), rotation));
            this->p2_rotation(rotation);
        }
        else {
            this->p2_position(P2BodyMassPosToOriginPos(t, this->p2_massCenter(), this->p2_rotation()));
        }
    }
    else if(massPos.y == P2_MAX()) {
        ZFUIPoint t;
        t.x = massPos.x;
        t.y = P2BodyMassPosFromOriginPos(this->p2_position(), this->p2_massCenter(), this->p2_rotation()).y;
        if(rotation != P2_MAX()) {
            this->p2_position(P2BodyMassPosToOriginPos(t, this->p2_massCenter(), rotation));
            this->p2_rotation(rotation);
        }
        else {
            this->p2_position(P2BodyMassPosToOriginPos(t, this->p2_massCenter(), this->p2_rotation()));
        }
    }
    else {
        if(rotation != P2_MAX()) {
            this->p2_position(P2BodyMassPosToOriginPos(massPos, this->p2_massCenter(), rotation));
            this->p2_rotation(rotation);
        }
        else {
            this->p2_position(P2BodyMassPosToOriginPos(massPos, this->p2_massCenter(), this->p2_rotation()));
        }
    }
}

ZFMETHOD_DEFINE_2(P2Body, void, p2_toLocal
        , ZFMP_OUT(P2BodyTransform &, transformLocal)
        , ZFMP_IN(const P2BodyTransform &, transformGlobal)
        ) {
    if(_ZFP_P2Body_d->ownerBody) {
        _ZFP_P2BodyTransfrom::toLocal(transformLocal, transformGlobal, _ZFP_P2BodyTransfrom::transforms(this));
    }
    else {
        transformLocal = transformGlobal;
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_toGlobal
        , ZFMP_OUT(P2BodyTransform &, transformGlobal)
        , ZFMP_IN(const P2BodyTransform &, transformLocal)
        ) {
    if(_ZFP_P2Body_d->ownerBody) {
        _ZFP_P2BodyTransfrom::toGlobal(transformGlobal, transformLocal, _ZFP_P2BodyTransfrom::transforms(this));
    }
    else{
        transformGlobal = transformLocal;
    }
}
ZFMETHOD_DEFINE_0(P2Body, ZFCoreArray<P2BodyTransform>, p2_transforms) {
    return _ZFP_P2BodyTransfrom::transforms(this);
}

ZFMETHOD_DEFINE_0(P2Body, zfbool, p2_bodyVisible) {
    return ZFBitTest(_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyVisible);
}

ZFMETHOD_DEFINE_0(P2Body, zfbool, p2_sleeping) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        return !b2Body_IsAwake(_ZFP_P2Body_d->implBodyId);
    }
    else {
        return zffalse;
    }
}
ZFMETHOD_DEFINE_1(P2Body, void, p2_sleeping
        , ZFMP_IN(zfbool, v)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_SetAwake(_ZFP_P2Body_d->implBodyId, !v);
    }
}

ZFMETHOD_DEFINE_3(P2Body, void, p2_applyPositionForceInWorld
        , ZFMP_IN(const ZFUIPoint &, force)
        , ZFMP_IN(const ZFUIPoint &, worldPosition)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyForce(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(force), b2Vec2FromZF(worldPosition), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyPositionForce
        , ZFMP_IN(const ZFUIPoint &, force)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyForceToCenter(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(force), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyRotationForce
        , ZFMP_IN(zffloat, force)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyTorque(_ZFP_P2Body_d->implBodyId, force, wakeup);
    }
}

ZFMETHOD_DEFINE_3(P2Body, void, p2_applyPositionImpulseInWorld
        , ZFMP_IN(const ZFUIPoint &, impulse)
        , ZFMP_IN(const ZFUIPoint &, worldPosition)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyLinearImpulse(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(impulse), b2Vec2FromZF(worldPosition), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyPositionImpulse
        , ZFMP_IN(const ZFUIPoint &, impulse)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyLinearImpulseToCenter(_ZFP_P2Body_d->implBodyId, b2Vec2FromZF(impulse), wakeup);
    }
}
ZFMETHOD_DEFINE_2(P2Body, void, p2_applyRotationImpulse
        , ZFMP_IN(zffloat, impulse)
        , ZFMP_IN_OPT(zfbool, wakeup, zftrue)
        ) {
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        b2Body_ApplyTorque(_ZFP_P2Body_d->implBodyId, impulse, wakeup);
    }
}

ZFMETHOD_DEFINE_1(P2Body, void, p2_shape
        , ZFMP_IN(P2Shape *, v)
        ) {
    ZFCoreAssert(v != zfnull && v->p2_ownerBody() == zfnull);
    this->p2_shapeList()->add(v);
    _ZFP_P2ShapeAttach(this, v);
}
ZFMETHOD_DEFINE_0(P2Body, zfindex, p2_shapeCount) {
    return this->p2_shapeList()->count();
}
ZFMETHOD_DEFINE_1(P2Body, P2Shape *, p2_shapeAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_shapeList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Shape>, p2_shapeRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Shape> item = this->p2_shapeList()->removeAndGet(index);
    _ZFP_P2ShapeDetach(this, item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Shape>, p2_shapeRemove
        , ZFMP_IN(P2Shape *, v)
        ) {
    zfindex index = this->p2_shapeList()->find(v);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Shape> item = this->p2_shapeList()->removeAndGet(index);
    _ZFP_P2ShapeDetach(this, item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Body, void, p2_shapeRemoveAll) {
    ZFArray *l = this->p2_shapeList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Shape *item = l->get(i);
            _ZFP_P2ShapeDetach(this, item);
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2Body, P2Shape *, p2_shapeFind
        , ZFMP_IN(const zfstring &, shapeId)
        ) {
    if(!shapeId) {
        return zfnull;
    }
    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring shapeIdTmp;
    {
        zfindex dotPos = zfstringFindReversely(shapeId, ".");
        if(dotPos == zfindexMax()) {
            toCheck.add(this);
            shapeIdTmp = shapeId;
        }
        else {
            P2Body *body = this->p2_bodyFind(zftext(shapeId, dotPos));
            if(!body) {
                return zfnull;
            }
            toCheck.add(body);
            shapeIdTmp = zftext(shapeId + dotPos + 1);
        }
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        ZFArray *l = body->p2_shapeList();
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Shape *item = l->get(i);
            if(item->p2_shapeId() == shapeIdTmp) {
                return item;
            }
        }
        l = body->p2_bodyList();
        for(zfindex i = 0; i < l->count(); ++i) {
            toCheck.add(l->get(i));
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_1(P2Body, void, p2_body
        , ZFMP_IN(P2Body *, v)
        ) {
    ZFCoreAssert(v && v->p2_owner() == zfnull);
    this->p2_bodyList()->add(v);
    _ZFP_P2BodyAttach(zfnull, this, v);
}
ZFMETHOD_DEFINE_0(P2Body, zfindex, p2_bodyCount) {
    return this->p2_bodyList()->count();
}
ZFMETHOD_DEFINE_1(P2Body, P2Body *, p2_bodyAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_bodyList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Body>, p2_bodyRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Body> item = this->p2_bodyList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Body>, p2_bodyRemove
        , ZFMP_IN(P2Body *, v)
        ) {
    zfindex index = this->p2_bodyList()->find(v);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Body> item = this->p2_bodyList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Body, void, p2_bodyRemoveAll) {
    ZFArray *l = this->p2_bodyList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Body *item = l->get(i);
            _ZFP_P2BodyDetach(item);
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2Body, P2Body *, p2_bodyFind
        , ZFMP_IN(const zfstring &, bodyId)
        ) {
    if(!bodyId) {
        return zfnull;
    }
    if(this->p2_bodyId() == bodyId) {
        return this;
    }
    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring bodyIdTmp;
    {
        zfindex dotPos = zfstringFindReversely(bodyId, ".");
        if(dotPos == zfindexMax()) {
            toCheck.add(this);
            bodyIdTmp = bodyId;
        }
        else {
            P2Body *body = this->p2_bodyFind(zftext(bodyId, dotPos));
            if(!body) {
                return zfnull;
            }
            toCheck.add(body);
            bodyIdTmp = zftext(bodyId + dotPos + 1);
        }
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        if(body->p2_bodyId() == bodyIdTmp) {
            return body;
        }
        ZFArray *l = body->p2_bodyList();
        for(zfindex i = 0; i < l->count(); ++i) {
            toCheck.add(l->get(i));
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_1(P2Body, void, p2_joint
        , ZFMP_IN(P2Joint *, v)
        ) {
    ZFCoreAssert(v && v->p2_owner() == zfnull);
    this->p2_jointList()->add(v);
    _ZFP_P2JointAttach(zfnull, this, v);
}
ZFMETHOD_DEFINE_0(P2Body, zfindex, p2_jointCount) {
    return this->p2_jointList()->count();
}
ZFMETHOD_DEFINE_1(P2Body, P2Joint *, p2_jointAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_jointList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Joint>, p2_jointRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Joint>, p2_jointRemove
        , ZFMP_IN(P2Joint *, v)
        ) {
    zfindex index = this->p2_jointList()->find(v);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2Body, void, p2_jointRemoveAll
        ) {
    ZFArray *l = this->p2_jointList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Joint *item = l->get(i);
            _ZFP_P2JointDetach(item);
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2Body, P2Joint *, p2_jointFind
        , ZFMP_IN(const zfstring &, jointId)
        ) {
    if(!jointId) {
        return zfnull;
    }
    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring jointIdTmp;
    {
        zfindex dotPos = zfstringFindReversely(jointId, ".");
        if(dotPos == zfindexMax()) {
            toCheck.add(this);
            jointIdTmp = jointId;
        }
        else {
            P2Body *body = this->p2_bodyFind(zftext(jointId, dotPos));
            if(!body) {
                return zfnull;
            }
            toCheck.add(body);
            jointIdTmp = zftext(jointId + dotPos + 1);
        }
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        ZFArray *l = body->_ZFP_P2Body_d->bodyRefJointList;
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Joint *item = l->get(i);
            if(item->p2_jointId() == jointIdTmp) {
                return item;
            }
        }
        l = body->p2_bodyList();
        for(zfindex i = 0; i < l->count(); ++i) {
            toCheck.add(l->get(i));
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_0(P2Body, ZFArray *, p2_refJointList) {
    return _ZFP_P2Body_d->bodyRefJointList;
}

ZFMETHOD_DEFINE_1(P2Body, void, p2_ext
        , ZFMP_IN(P2Ext *, v)
        ) {
    ZFCoreAssert(v && v->p2_ownerBody() == zfnull);
    this->p2_extList()->add(v);
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ExtAttach(this, v);
    }
}
ZFMETHOD_DEFINE_0(P2Body, zfindex, p2_extCount) {
    return this->p2_extList()->count();
}
ZFMETHOD_DEFINE_1(P2Body, P2Ext *, p2_extAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_extList()->get(index);
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Ext>, p2_extRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Ext> v = this->p2_extList()->get(index);
    this->p2_extList()->remove(index);
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ExtDetach(v);
    }
    return v;
}
ZFMETHOD_DEFINE_1(P2Body, zfautoT<P2Ext>, p2_extRemove
        , ZFMP_IN(P2Ext *, v)
        ) {
    zfindex index = this->p2_extList()->find(v);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Ext> item = this->p2_extList()->removeAndGet(index);
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ExtDetach(item);
    }
    return item;
}
ZFMETHOD_DEFINE_0(P2Body, void, p2_extRemoveAll) {
    ZFArray *l = this->p2_extList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Ext *item = l->get(i);
            if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
                _ZFP_P2ExtDetach(item);
            }
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2Body, zfindex, p2_extCheck
        , ZFMP_IN(const ZFClass *, extCls)
        ) {
    if(extCls && extCls->classIsTypeOf(P2Ext::ClassData())) {
        ZFArray *l = this->p2_extList();
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Ext *v = l->get(i);
            if(v && v->classData()->classIsTypeOf(extCls)) {
                return i;
            }
        }
    }
    return zfindexMax();
}
ZFMETHOD_DEFINE_1(P2Body, P2Ext *, p2_extAccess
        , ZFMP_IN(const ZFClass *, extCls)
        ) {
    ZFCoreAssert(extCls && extCls->classIsTypeOf(P2Ext::ClassData()));
    ZFArray *l = this->p2_extList();
    for(zfindex i = 0; i < l->count(); ++i) {
        P2Ext *v = l->get(i);
        if(v && v->classData()->classIsTypeOf(extCls)) {
            return v;
        }
    }
    zfautoT<P2Ext> v = extCls->newInstance();
    ZFCoreAssert(v);
    this->p2_extList()->add(v);
    if(B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ExtAttach(this, v);
    }
    return v;
}

ZFMETHOD_DEFINE_0(P2Body, zfbool, p2_implAttached) {
    return B2_IS_NON_NULL(_ZFP_P2Body_d->implBodyId);
}

void P2Body::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2Body_d = zfpoolNew(_ZFP_P2BodyPrivate);

    zfclassNotPOD _Impl {
    public:
        static void prev(ZF_IN const ZFArgs &zfargs) {
            P2Body *owner = zfargs.sender();
            owner->p2_extRemoveAll();
            owner->p2_bodyRemoveAll();
            owner->p2_jointRemoveAll();
            owner->p2_shapeRemoveAll();
        }
        static void post(ZF_IN const ZFArgs &zfargs) {
            P2Body *owner = zfargs.sender();
            {
                ZFArray *l = owner->p2_shapeList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    _ZFP_P2ShapeAttach(owner, l->get(i));
                }
            }
            {
                ZFArray *l = owner->p2_jointList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    _ZFP_P2JointAttach(zfnull, owner, l->get(i));
                }
            }
            {
                ZFArray *l = owner->p2_bodyList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    _ZFP_P2BodyAttach(zfnull, owner, l->get(i));
                }
            }
            {
                ZFArray *l = owner->p2_extList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    if(B2_IS_NON_NULL(owner->_ZFP_P2Body_d->implBodyId)) {
                        _ZFP_P2ExtAttach(owner, l->get(i));
                    }
                }
            }
        }
    };
    this->observerAdd(ZFSerializable::E_SerializeFromPrev(), ZFCallbackForFunc(_Impl::prev));
    this->observerAdd(ZFSerializable::E_SerializeFromPost(), ZFCallbackForFunc(_Impl::post));
}
void P2Body::objectOnDealloc(void) {
    ZFCoreAssert(B2_IS_NULL(_ZFP_P2Body_d->implBodyId));
    zfpoolDelete(_ZFP_P2Body_d);
    zfsuper::objectOnDealloc();
}

// ============================================================
ZFOBJECT_REGISTER(P2Ext)

ZFEVENT_REGISTER(P2Ext, P2ExtAttach)
ZFEVENT_REGISTER(P2Ext, P2ExtDetach)
ZFEVENT_REGISTER(P2Ext, P2ExtUpdate)

ZFMETHOD_DEFINE_0(P2Ext, P2Body *, p2_ownerBody) {
    return _ZFP_ownerBody;
}

// ============================================================
ZFOBJECT_REGISTER(P2BodyVisibilityEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2BodyVisibilityEvent, ZFCoreArray<P2Body *>, p2_bodyEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2BodyVisibilityEvent, ZFCoreArray<P2Body *>, p2_bodyExitList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2BodyMoveEventData, P2BodyMoveEventData)
ZFOUTPUT_TYPE_DEFINE(P2BodyMoveEventData, {
    zfstringAppend(s
            , "<Move %s %s %s>"
            , v.p2_position
            , v.p2_rotation
            , v.p2_body
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, P2Body *, p2_body)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, ZFUIPoint, p2_position)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2BodyMoveEventData, zffloat, p2_rotation)

ZFOBJECT_REGISTER(P2BodyMoveEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2BodyMoveEvent, ZFCoreArray<P2BodyMoveEventData>, p2_moveEventList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2SensorEventData, P2SensorEventData)
ZFOUTPUT_TYPE_DEFINE(P2SensorEventData, {
    zfstringAppend(s
            , "<Sensor %s %s>"
            , v.p2_shape0
            , v.p2_shape1
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2SensorEventData, P2Shape *, p2_shape0)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2SensorEventData, P2Shape *, p2_shape1)

ZFOBJECT_REGISTER(P2SensorEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2SensorEvent, ZFCoreArray<P2SensorEventData>, p2_sensorEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2SensorEvent, ZFCoreArray<P2SensorEventData>, p2_sensorExitList)

ZFTYPEID_ACCESS_ONLY_DEFINE_UNCOMPARABLE(P2ContactEventData, P2ContactEventData)
ZFOUTPUT_TYPE_DEFINE(P2ContactEventData, {
    zfstringAppend(s
            , "<Contact %s %s %s %s>"
            , v.p2_shape0
            , v.p2_shape1
            , v.p2_normal
            , v.p2_contactEnable
            );
})
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2ContactEventData, P2Shape *, p2_shape0)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2ContactEventData, P2Shape *, p2_shape1)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR_READONLY(v_P2ContactEventData, ZFUIPoint, p2_normal)
ZFMETHOD_USER_REGISTER_FOR_WRAPPER_VAR(v_P2ContactEventData, zfbool, p2_contactEnable)
static void _ZFP_P2ContactEventDataFromManifold(ZF_IN_OUT P2ContactEventData &v, ZF_IN const b2Manifold &t) {
    v.p2_normal = b2Vec2ToZF(t.normal);
}

ZFOBJECT_REGISTER(P2ContactEvent)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2ContactEvent, ZFCoreArray<P2ContactEventData>, p2_contactEnterList)
ZFMETHOD_USER_REGISTER_FOR_ZFOBJECT_VAR_READONLY(P2ContactEvent, ZFCoreArray<P2ContactEventData>, p2_contactExitList)

// ============================================================
ZFOBJECT_REGISTER(P2World)

ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIPoint, p2_gravity) {
    if(B2_IS_NON_NULL(_ZFP_P2World_d->implWorldId)) {
        b2World_SetGravity(_ZFP_P2World_d->implWorldId, b2Vec2FromZF(propertyValue));
    }
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIPoint, p2_UIOffset) {
    if(propertyValue != propertyValueOld) {
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIOffsetChanged);
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUISize, p2_UISize) {
    if(propertyValue != propertyValueOld) {
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UISizeChanged);
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
    }
}
ZFPROPERTY_ON_UPDATE_DEFINE(P2World, zffloat, p2_UIScale) {
    if(propertyValue <= 0) {
        propertyValue = 1;
    }
    if(propertyValue != propertyValueOld) {
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIScaleChanged);
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
    }
}
ZFMETHOD_DEFINE_0(P2World, void, p2_UIUpdateRequest) {
    ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
}

ZFPROPERTY_ON_UPDATE_DEFINE(P2World, ZFUIMargin, p2_UIVisibleAreaMargin) {
    if(propertyValue != propertyValueOld) {
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIVisibleAreaChanged);
        ZFBitSet(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
    }
}
ZFMETHOD_DEFINE_0(P2World, const ZFUIRect &, p2_UIVisibleArea) {
    return _ZFP_P2World_d->visibleArea;
}

ZFMETHOD_DEFINE_0(P2World, zfbool, p2_UIOffsetChanged) {
    return ZFBitTest(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIOffsetChanged);
}
ZFMETHOD_DEFINE_0(P2World, zfbool, p2_UISizeChanged) {
    return ZFBitTest(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UISizeChanged);
}
ZFMETHOD_DEFINE_0(P2World, zfbool, p2_UIScaleChanged) {
    return ZFBitTest(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIScaleChanged);
}
ZFMETHOD_DEFINE_0(P2World, zfbool, p2_UIVisibleAreaChanged) {
    return ZFBitTest(_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIChangedMask);
}

ZFMETHOD_DEFINE_0(P2World, void, p2_start) {
    if(!_ZFP_P2World_d->implTimer) {
        zfself *owner = this;
        ZFLISTENER_1(impl
                , zfself *, owner
                ) {
            _ZFP_P2WorldImplUpdate(owner);
        } ZFLISTENER_END()
        _ZFP_P2World_d->implTimer = impl;
        ZFGlobalTimerAttach(impl);
    }
}
ZFMETHOD_DEFINE_0(P2World, void, p2_stop) {
    if(_ZFP_P2World_d->implTimer) {
        ZFGlobalTimerDetach(_ZFP_P2World_d->implTimer);
        _ZFP_P2World_d->implTimer = zfnull;
    }
}
ZFMETHOD_DEFINE_0(P2World, zfbool, p2_started) {
    return _ZFP_P2World_d->implTimer;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_manualUpdate) {
    if(_ZFP_P2World_d->implTimer) {
        ZFGlobalTimerDetach(_ZFP_P2World_d->implTimer);
        _ZFP_P2World_d->implTimer = zfnull;
    }
    _ZFP_P2WorldImplUpdate(this);
}

ZFMETHOD_DEFINE_1(P2World, void, p2_body
        , ZFMP_IN(P2Body *, v)
        ) {
    ZFCoreAssert(v && v->p2_owner() == zfnull);
    this->p2_bodyList()->add(v);
    _ZFP_P2BodyAttach(this, zfnull, v);
}
ZFMETHOD_DEFINE_0(P2World, zfindex, p2_bodyCount) {
    return this->p2_bodyList()->count();
}
ZFMETHOD_DEFINE_1(P2World, P2Body *, p2_bodyAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_bodyList()->get(index);
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Body>, p2_bodyRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Body> item = this->p2_bodyList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Body>, p2_bodyRemove
        , ZFMP_IN(P2Body *, body)
        ) {
    zfindex index = this->p2_bodyList()->find(body);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Body> item = this->p2_bodyList()->removeAndGet(index);
    _ZFP_P2BodyDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_bodyRemoveAll) {
    ZFArray *l = this->p2_jointList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Body *item = l->get(i);
            _ZFP_P2BodyDetach(item);
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2World, P2Body *, p2_bodyFind
        , ZFMP_IN(const zfstring &, bodyId)
        ) {
    if(!bodyId) {
        return zfnull;
    }
    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring bodyIdTmp;
    {
        zfindex dotPos = zfstringFindReversely(bodyId, ".");
        if(dotPos == zfindexMax()) {
            ZFArray *l = this->p2_bodyList();
            for(zfindex i = 0; i < l->count(); ++i) {
                toCheck.add(l->get(i));
            }
            bodyIdTmp = bodyId;
        }
        else {
            P2Body *body = this->p2_bodyFind(zftext(bodyId, dotPos));
            if(!body) {
                return zfnull;
            }
            toCheck.add(body);
            bodyIdTmp = zftext(bodyId + dotPos + 1);
        }
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        if(body->p2_bodyId() == bodyIdTmp) {
            return body;
        }
        ZFArray *l = body->p2_bodyList();
        for(zfindex i = 0; i < l->count(); ++i) {
            toCheck.add(l->get(i));
        }
    }
    return zfnull;
}
ZFMETHOD_DEFINE_1(P2World, P2Shape *, p2_shapeFind
        , ZFMP_IN(const zfstring &, shapeId)
        ) {
    if(!shapeId) {
        return zfnull;
    }
    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring shapeIdTmp;
    {
        zfindex dotPos = zfstringFindReversely(shapeId, ".");
        if(dotPos == zfindexMax()) {
            ZFArray *l = this->p2_bodyList();
            for(zfindex i = 0; i < l->count(); ++i) {
                toCheck.add(l->get(i));
            }
            shapeIdTmp = shapeId;
        }
        else {
            P2Body *body = this->p2_bodyFind(zftext(shapeId, dotPos));
            if(!body) {
                return zfnull;
            }
            toCheck.add(body);
            shapeIdTmp = zftext(shapeId + dotPos + 1);
        }
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        ZFArray *l = body->p2_shapeList();
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Shape *item = l->get(i);
            if(item->p2_shapeId() == shapeIdTmp) {
                return item;
            }
        }
    }
    return zfnull;
}

ZFMETHOD_DEFINE_1(P2World, void, p2_joint
        , ZFMP_IN(P2Joint *, v)
        ) {
    ZFCoreAssert(v && v->p2_owner() == zfnull);
    this->p2_jointList()->add(v);
    _ZFP_P2JointAttach(this, zfnull, v);
}
ZFMETHOD_DEFINE_0(P2World, zfindex, p2_jointCount) {
    return this->p2_jointList()->count();
}
ZFMETHOD_DEFINE_1(P2World, P2Joint *, p2_jointAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->p2_jointList()->get(index);
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Joint>, p2_jointRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_1(P2World, zfautoT<P2Joint>, p2_jointRemove
        , ZFMP_IN(P2Joint *, v)
        ) {
    zfindex index = this->p2_jointList()->find(v);
    if(index == zfindexMax()) {
        return zfnull;
    }
    zfautoT<P2Joint> item = this->p2_jointList()->removeAndGet(index);
    _ZFP_P2JointDetach(item);
    return item;
}
ZFMETHOD_DEFINE_0(P2World, void, p2_jointRemoveAll
        ) {
    ZFArray *l = this->p2_jointList();
    if(!l->isEmpty()) {
        for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
            P2Joint *item = l->get(i);
            _ZFP_P2JointDetach(item);
        }
        l->removeAll();
    }
}
ZFMETHOD_DEFINE_1(P2World, P2Joint *, p2_jointFind
        , ZFMP_IN(const zfstring &, jointId)
        ) {
    if(!jointId) {
        return zfnull;
    }
    zfindex dotPos = zfstringFindReversely(jointId, ".");
    if(dotPos == zfindexMax()) {
        ZFArray *l = this->p2_jointList();
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Joint *item = l->get(i);
            if(item->p2_jointId() == jointId) {
                return item;
            }
        }
        return zfnull;
    }

    ZFCoreQueuePOD<P2Body *> toCheck;
    zfstring jointIdTmp = zftext(jointId + dotPos + 1);
    {
        P2Body *body = this->p2_bodyFind(zftext(jointId, dotPos));
        if(!body) {
            return zfnull;
        }
        toCheck.add(body);
    }
    while(!toCheck.isEmpty()) {
        P2Body *body = toCheck.take();
        ZFArray *l = body->_ZFP_P2Body_d->bodyRefJointList;
        for(zfindex i = 0; i < l->count(); ++i) {
            P2Joint *joint = l->get(i);
            if(joint->p2_jointId() == jointIdTmp) {
                return joint;
            }
        }
        l = body->p2_bodyList();
        for(zfindex i = 0; i < l->count(); ++i) {
            toCheck.add(l->get(i));
        }
    }
    return zfnull;
}

zfclassLikePOD _ZFP_P2World_overlapTestContext {
public:
    ZFListener callback;
    ZFArgs zfargs;
};
static bool _ZFP_P2World_overlapTestCb(b2ShapeId shapeId, void *context) {
    _ZFP_P2World_overlapTestContext &t = *(_ZFP_P2World_overlapTestContext *)context;
    t.zfargs.param0((P2Shape *)b2Shape_GetUserData(shapeId));
    t.callback.execute(t.zfargs);
    return !t.zfargs.eventFiltered();
}
ZFMETHOD_DEFINE_4(P2World, zfauto, p2_overlapTest
        , ZFMP_IN(const ZFUIRect &, rect)
        , ZFMP_IN(const ZFListener &, callback)
        , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
        , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
        ) {
    _ZFP_P2World_overlapTestContext context;
    context.callback = callback;
    context.zfargs.sender(this);
    b2QueryFilter implFilter;
    implFilter.maskBits = (uint64_t)filterMask;
    implFilter.categoryBits = (uint64_t)filterCategory;
    b2World_OverlapAABB(
            _ZFP_P2World_d->implWorldId
            , b2AABBFromZF(rect)
            , implFilter
            , _ZFP_P2World_overlapTestCb
            , &context
            );
    return context.zfargs.result();
}
static float _ZFP_P2World_rayTestCb(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void *context) {
    _ZFP_P2World_overlapTestContext &t = *(_ZFP_P2World_overlapTestContext *)context;
    t.zfargs.param0((P2Shape *)b2Shape_GetUserData(shapeId));
    t.callback.execute(t.zfargs);
    if(t.zfargs.eventFiltered()) {
        return 0;
    }
    else {
        return 1;
    }
}
ZFMETHOD_DEFINE_5(P2World, zfauto, p2_rayTest
        , ZFMP_IN(const ZFUIPoint &, src)
        , ZFMP_IN(const ZFUIPoint &, direction)
        , ZFMP_IN(const ZFListener &, callback)
        , ZFMP_IN_OPT(zfflags, filterMask, P2FilterMaskAll())
        , ZFMP_IN_OPT(zfflags, filterCategory, P2FilterMaskAll())
        ) {
    _ZFP_P2World_overlapTestContext context;
    context.callback = callback;
    context.zfargs.sender(this);
    b2QueryFilter implFilter;
    implFilter.maskBits = (uint64_t)filterMask;
    implFilter.categoryBits = (uint64_t)filterCategory;
    b2World_CastRay(
            _ZFP_P2World_d->implWorldId
            , b2Vec2FromZF(src)
            , b2Vec2FromZF(direction)
            , implFilter
            , _ZFP_P2World_rayTestCb
            , &context
            );
    return context.zfargs.result();
}

static bool _ZFP_P2World_PreSolve(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold *manifold, void *context) {
    P2Shape *shapeA = (P2Shape *)b2Shape_GetUserData(shapeIdA);
    P2Shape *shapeB = (P2Shape *)b2Shape_GetUserData(shapeIdB);
    P2Body *bodyA = shapeA ? shapeA->p2_ownerBody() : zfnull;
    P2Body *bodyB = shapeB ? shapeB->p2_ownerBody() : zfnull;
    if(zffalse
            || (bodyA && ZFBitTest(bodyA->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter))
            || (bodyB && ZFBitTest(bodyB->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter))
            ) {
        zfobj<v_P2ContactEventData> eventData;
        eventData->zfv.p2_shape0 = shapeA;
        eventData->zfv.p2_shape1 = shapeB;
        eventData->zfv.p2_contactEnable = zftrue;
        _ZFP_P2ContactEventDataFromManifold(eventData->zfv, *manifold);
        if(bodyA && ZFBitTest(bodyA->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter)) {
            bodyA->observerNotify(P2Body::E_P2ContactFilter(), ZFArgs()
                    .param0(bodyB)
                    .param1(eventData)
                    );
        }
        if(bodyB && ZFBitTest(bodyB->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactFilter)) {
            bodyB->observerNotify(P2Body::E_P2ContactFilter(), ZFArgs()
                    .param0(bodyA)
                    .param1(eventData)
                    );
        }
        return eventData->zfv.p2_contactEnable;
    }
    return zftrue;
}
void P2World::objectOnInit(void) {
    zfsuper::objectOnInit();
    _ZFP_P2World_d = zfpoolNew(_ZFP_P2WorldPrivate);

    b2WorldDef implWorldDef = b2DefaultWorldDef();
    implWorldDef.userData = this;
    implWorldDef.gravity = b2Vec2FromZF(this->p2_gravity());
    implWorldDef.enableSleep = zftrue;
    implWorldDef.enableContinuous = zftrue;
    _ZFP_P2World_d->implWorldId = b2CreateWorld(&implWorldDef);
    b2World_SetPreSolveCallback(_ZFP_P2World_d->implWorldId, _ZFP_P2World_PreSolve, zfnull);

    zfclassNotPOD _Impl {
    public:
        static void prev(ZF_IN const ZFArgs &zfargs) {
            P2World *owner = zfargs.sender();
            owner->p2_jointRemoveAll();
            owner->p2_bodyRemoveAll();
        }
        static void post(ZF_IN const ZFArgs &zfargs) {
            P2World *owner = zfargs.sender();
            {
                ZFArray *l = owner->p2_bodyList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    _ZFP_P2BodyAttach(owner, zfnull, l->get(i));
                }
            }
            {
                ZFArray *l = owner->p2_jointList();
                for(zfindex i = 0, count = l->count(); i < count; ++i) {
                    _ZFP_P2JointAttach(owner, zfnull, l->get(i));
                }
            }
        }
    };
    this->observerAdd(ZFSerializable::E_SerializeFromPrev(), ZFCallbackForFunc(_Impl::prev));
    this->observerAdd(ZFSerializable::E_SerializeFromPost(), ZFCallbackForFunc(_Impl::post));
}
void P2World::objectOnDealloc(void) {
    b2DestroyWorld(_ZFP_P2World_d->implWorldId);
    zfpoolDelete(_ZFP_P2World_d);
    zfsuper::objectOnDealloc();
}
void P2World::objectOnDeallocPrepare(void) {
    this->p2_stop();

    // recursively cleanup internal state
    // impl would be destroyed by b2DestroyWorld
    {
        zfclassNotPOD CleanupJoint {
        public:
            static void a(ZF_IN P2Joint *joint) {
                if(B2_IS_NON_NULL(joint->_ZFP_P2Joint_d->implJointId)) {
                    P2World *world = joint->p2_ownerWorld();
                    if(world) {
                        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                                ) {
                            world->observerNotify(P2World::E_P2JointDetach(), ZFArgs()
                                    .param0(joint)
                                    );
                        }
                    }
                    joint->_ZFP_P2Joint_d->implJointId = b2_nullJointId;
                }
                joint->_ZFP_P2Joint_d->ownerWorld = zfnull;
                joint->_ZFP_P2Joint_d->ownerBody = zfnull;
            }
        };
        zfclassNotPOD CleanupBody {
        public:
            static void a(ZF_IN P2World *world, ZF_IN P2Body *body) {
                if(B2_IS_NON_NULL(body->_ZFP_P2Body_d->implBodyId)) {
                    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach)
                            || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach)
                            ) {
                        world->observerNotify(P2World::E_P2BodyDetach(), ZFArgs()
                                .param0(body)
                                );
                    }
                    world->_ZFP_P2World_d->customUpdateBody.erase(body);
                    world->p2impl->bodyRemove(world, body);
                    body->_ZFP_P2Body_d->implBodyId = b2_nullBodyId;
                }
                body->_ZFP_P2Body_d->ownerWorld = zfnull;
                body->_ZFP_P2Body_d->ownerBody = zfnull;
                body->_ZFP_P2Body_d->bodyRefJointList->removeAll();
                {
                    ZFArray *l = body->p2_extList();
                    for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                        P2Ext *item = l->get(i);
                        _ZFP_P2ExtPrivate::detach(item);
                    }
                    l->removeAll();
                }
                {
                    ZFArray *l = body->p2_jointList();
                    for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                        P2Joint *item = l->get(i);
                        CleanupJoint::a(item);
                    }
                    l->removeAll();
                }
                {
                    ZFArray *l = body->p2_bodyList();
                    for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                        P2Body *item = l->get(i);
                        CleanupBody::a(world, item);
                    }
                    l->removeAll();
                }
                {
                    ZFArray *l = body->p2_shapeList();
                    for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                        P2Shape *item = l->get(i);
                        item->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
                    }
                    l->removeAll();
                }
            }
        };

        {
            ZFArray *l = this->p2_jointList();
            for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                CleanupJoint::a(l->get(i));
            }
            l->removeAll();
        }

        {
            ZFArray *l = this->p2_bodyList();
            for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                CleanupBody::a(this, l->get(i));
            }
            l->removeAll();
        }

        _ZFP_P2World_d->pendingBody.clear();
        _ZFP_P2World_d->pendingJoint.clear();
        _ZFP_P2World_d->pendingBodyMove.clear();
        _ZFP_P2World_d->visibleBody.clear();
        _ZFP_P2World_d->visibleBodyPrev.clear();
        _ZFP_P2World_d->customUpdateBody.clear();
    }

    zfsuper::objectOnDeallocPrepare();
}

// ============================================================
static void _ZFP_P2ShapeAttach(ZF_IN P2Body *owner, ZF_IN P2Shape *shape) {
    ZFBitSet(owner->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested);
    if(B2_IS_NON_NULL(owner->_ZFP_P2Body_d->implBodyId)) {
        _ZFP_P2ShapePrivate::shapeCreate(owner, shape, owner->p2_bodyScaleFixed());
        _ZFP_P2BodyImplMassUpdateRequest(owner);
    }
}
static void _ZFP_P2ShapeDetach(ZF_IN P2Body *owner, ZF_IN P2Shape *shape) {
    ZFBitSet(owner->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested);
    if(B2_IS_NON_NULL(shape->_ZFP_P2Shape_d->implShapeId)) {
        b2DestroyShape(shape->_ZFP_P2Shape_d->implShapeId, zffalse);
        shape->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
        _ZFP_P2BodyImplMassUpdateRequest(owner);
    }
}
static void _ZFP_P2JointAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Body *ownerBody, ZF_IN P2Joint *joint) {
    ZFCoreAssertWithMessageTrim(joint->_ZFP_P2Joint_d->ownerWorld == zfnull && joint->_ZFP_P2Joint_d->ownerBody == zfnull
            , "joint already attached to owner, joint: %s, owner: %s"
            , joint
            , joint->p2_owner()
            );
    joint->_ZFP_P2Joint_d->ownerWorld = ownerWorld;
    joint->_ZFP_P2Joint_d->ownerBody = ownerBody;
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingJoint[joint] = zftrue;
    }
    else if(ownerBody) {
        P2World *world = ownerBody->p2_ownerWorld();
        if(world) {
            world->_ZFP_P2World_d->pendingJoint[joint] = zftrue;
        }
    }
}
static void _ZFP_P2JointDetach(ZF_IN P2Joint *joint) {
    P2World *ownerWorld = joint->p2_ownerWorld();
    if(B2_IS_NON_NULL(joint->_ZFP_P2Joint_d->implJointId)) {
        if(ownerWorld) {
            if(ZFBitTest(ownerWorld->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                    || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                    ) {
                ownerWorld->observerNotify(P2World::E_P2JointDetach(), ZFArgs()
                        .param0(joint)
                        );
            }
        }
        joint->p2_ownerBody0()->_ZFP_P2Body_d->bodyRefJointList->removeElement(joint);
        joint->p2_ownerBody1()->_ZFP_P2Body_d->bodyRefJointList->removeElement(joint);
        b2DestroyJoint(joint->_ZFP_P2Joint_d->implJointId);
        joint->_ZFP_P2Joint_d->implJointId = b2_nullJointId;
    }
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingJoint.erase(joint);
    }
    joint->_ZFP_P2Joint_d->ownerWorld = zfnull;
    joint->_ZFP_P2Joint_d->ownerBody = zfnull;
}
static void _ZFP_P2BodyAttach(ZF_IN P2World *ownerWorld, ZF_IN P2Body *ownerBody, ZF_IN P2Body *body) {
    ZFCoreAssertWithMessageTrim(body->_ZFP_P2Body_d->ownerWorld == zfnull && body->_ZFP_P2Body_d->ownerBody == zfnull
            , "body already attached to owner, body: %s, owner: %s"
            , body
            , body->p2_owner()
            );
    body->_ZFP_P2Body_d->ownerWorld = ownerWorld;
    body->_ZFP_P2Body_d->ownerBody = ownerBody;
    if(ownerWorld) {
        _ZFP_P2WorldPrivate::bodyTreeAttach(ownerWorld, body);
    }
    else if(ownerBody) {
        P2World *world = ownerBody->p2_ownerWorld();
        if(world) {
            _ZFP_P2WorldPrivate::bodyTreeAttach(ownerWorld, body);
        }
    }
}
static void _ZFP_P2BodyDetach(ZF_IN P2Body *body) {
    P2World *world = body->p2_ownerWorld();
    if(world) {
        world->_ZFP_P2World_d->pendingBody.erase(body);
        world->_ZFP_P2World_d->pendingBodyMove.erase(body);
        world->_ZFP_P2World_d->visibleBody.erase(body);
        world->_ZFP_P2World_d->visibleBodyPrev.erase(body);
        if(B2_IS_NON_NULL(body->_ZFP_P2Body_d->implBodyId)) {
            world->p2impl->bodyRemove(world, body);

            ZFArray *bodyRefJointList = body->_ZFP_P2Body_d->bodyRefJointList;
            for(zfindex i = bodyRefJointList->count() - 1; i != zfindexMax(); --i) {
                P2Joint *joint = bodyRefJointList->get(i);
                world->_ZFP_P2World_d->pendingJoint.erase(joint);

                P2Body *owenrBody2 = joint->p2_ownerBody0();
                if(owenrBody2 == body) {
                    owenrBody2 = joint->p2_ownerBody1();
                }
                owenrBody2->_ZFP_P2Body_d->bodyRefJointList->removeElement(joint);

                if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                        || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointDetach)
                        ) {
                    world->observerNotify(P2World::E_P2JointDetach(), ZFArgs()
                            .param0(joint)
                            );
                }
            }

            if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach)
                    || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyDetach)
                    ) {
                world->observerNotify(P2World::E_P2BodyDetach(), ZFArgs()
                        .param0(body)
                        );
            }

            ZFArray *shapeList = body->p2_shapeList();
            for(zfindex i = shapeList->count() - 1; i != zfindexMax(); --i) {
                P2Shape *shape = shapeList->get(i);
                shape->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
            }

            world->_ZFP_P2World_d->customUpdateBody.erase(body);
            b2DestroyBody(body->_ZFP_P2Body_d->implBodyId);
            body->_ZFP_P2Body_d->implBodyId = b2_nullBodyId;
            ZFBitSet(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested);
        }
    }
    body->_ZFP_P2Body_d->ownerWorld = zfnull;
    body->_ZFP_P2Body_d->ownerBody = zfnull;
}
static void _ZFP_P2ExtAttach(ZF_IN P2Body *owner, ZF_IN P2Ext *ext) {
    _ZFP_P2ExtPrivate::attach(owner, ext);
}
static void _ZFP_P2ExtDetach(ZF_IN P2Ext *ext) {
    _ZFP_P2ExtPrivate::detach(ext);
}

static void _ZFP_P2BodyImplMassUpdateRequest(ZF_IN P2Body *body) {
    P2World *ownerWorld = body->p2_ownerWorld();
    if(ownerWorld) {
        ownerWorld->_ZFP_P2World_d->pendingBody[body] = zftrue;
    }
    ZFBitSet(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_massUpdateRequested);
}

// ============================================================
static void _ZFP_P2BodyImplCreate(ZF_IN P2World *world, ZF_IN P2Body *body) {
    b2BodyDef implBodyDef = b2DefaultBodyDef();
    implBodyDef.userData = body;
    switch(body->p2_type()) {
        case v_P2BodyType::e_Static:
            implBodyDef.type = b2_staticBody;
            break;
        case v_P2BodyType::e_Kinematic:
            implBodyDef.type = b2_kinematicBody;
            break;
        case v_P2BodyType::e_Dynamic:
            implBodyDef.type = b2_dynamicBody;
            break;
        default:
            ZFCoreCriticalShouldNotGoHere();
            break;
    }
    implBodyDef.isEnabled = body->p2_enable();
    implBodyDef.enableSleep = body->p2_sleepEnable();
    implBodyDef.isBullet = body->p2_bullet();
    implBodyDef.fixedRotation = body->p2_rotationFixed();
    implBodyDef.gravityScale = body->p2_gravityScale();
    implBodyDef.linearDamping = body->p2_positionDamping();
    implBodyDef.angularDamping = body->p2_rotationDamping();
    implBodyDef.linearVelocity = b2Vec2FromZF(body->p2_positionVelocity());
    implBodyDef.angularVelocity = b2AngularVelocityFromZF(body->p2_rotationVelocity());
    if(body->_ZFP_P2Body_d->ownerBody) {
        P2BodyTransform l;
        l.p2_position = body->p2_position();
        l.p2_rotation = body->p2_rotation();
        l.p2_bodyScale = body->p2_bodyScale();

        P2BodyTransform g;
        P2BodyTransformToGlobal(g, l, body->_ZFP_P2Body_d->ownerBody->p2_transforms());

        implBodyDef.position = b2Vec2FromZF(g.p2_position);
        implBodyDef.rotation = b2RotFromZF(g.p2_rotation);
    }
    else {
        implBodyDef.position = b2Vec2FromZF(body->p2_position());
        implBodyDef.rotation = b2RotFromZF(body->p2_rotation());
    }
    body->_ZFP_P2Body_d->implBodyId = b2CreateBody(world->_ZFP_P2World_d->implWorldId, &implBodyDef);
    ZFBitSet(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyPosUpdateRequested);

    zffloat shapeScale = body->p2_bodyScaleFixed();
    ZFArray *shapeList = body->p2_shapeList();
    for(zfindex i = 0; i < shapeList->count(); ++i) {
        P2Shape *shape = shapeList->get(i);
        _ZFP_P2ShapePrivate::shapeCreate(body, shape, shapeScale);
    }

    body->_ZFP_P2Body_d->massUpdate();

    world->p2impl->bodyAdd(world, body);

    if(ZFBitTest(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2Update)) {
        world->_ZFP_P2World_d->customUpdateBody[body] = zftrue;
    }
    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach)
            || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyAttach)
            ) {
        world->observerNotify(P2World::E_P2BodyAttach(), ZFArgs()
                .param0(body)
                );
    }

    {
        ZFArray *l = body->p2_extList();
        for(zfindex i = 0; i < l->count(); ++i) {
            _ZFP_P2ExtAttach(body, l->get(i));
        }
    }
}

static void _ZFP_P2WorldImplUpdate_pendingBody(ZF_IN P2World *world) {
    zfimplhashmap<P2Body *, zfbool> &pendingBody = world->_ZFP_P2World_d->pendingBody;
    for(zfimplhashmap<P2Body *, zfbool>::iterator it = pendingBody.begin(); it != pendingBody.end(); ++it) {
        P2Body *body = it->first;
        {
            zfbool parentShapeScaleUpdate = zffalse;
            for(P2Body *parent = body->_ZFP_P2Body_d->ownerBody; parent; parent = parent->_ZFP_P2Body_d->ownerBody) {
                if(ZFBitTest(parent->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyScaleUpdateRequested)) {
                    parentShapeScaleUpdate = zftrue;
                    break;
                }
            }
            if(parentShapeScaleUpdate) {
                // when parent scale needs update, the entire tree also need update
                continue;
            }
        }
        if(ZFBitTest(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyScaleUpdateRequested)) {
            ZFCoreQueuePOD<P2Body *> toUpdate;
            toUpdate.add(body);
            while(!toUpdate.isEmpty()) {
                P2Body *tmp = toUpdate.take();
                ZFBitUnset(tmp->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyScaleUpdateRequested);
                {
                    ZFArray *l = tmp->_ZFP_P2Body_d->bodyRefJointList;
                    for(zfindex i = l->count() - 1; i != zfindexMax(); --i) {
                        world->_ZFP_P2World_d->pendingJoint[l->get(i)] = zftrue;
                    }
                }
                if(B2_IS_NULL(tmp->_ZFP_P2Body_d->implBodyId)) {
                    _ZFP_P2BodyImplCreate(world, tmp);
                }
                else {
                    zffloat shapeScale = tmp->p2_bodyScaleFixed();
                    ZFArray *l = tmp->p2_shapeList();
                    for(zfindex i = 0; i < l->count(); ++i) {
                        P2Shape *shape = l->get(i);
                        b2DestroyShape(shape->_ZFP_P2Shape_d->implShapeId, zffalse);
                        shape->_ZFP_P2Shape_d->implShapeId = b2_nullShapeId;
                        _ZFP_P2ShapePrivate::shapeCreate(tmp, shape, shapeScale);
                    }
                    tmp->_ZFP_P2Body_d->massUpdate();
                }
                ZFArray *l = tmp->p2_bodyList();
                for(zfindex i = 0; i < l->count(); ++i) {
                    toUpdate.add(l->get(i));
                }
            }
        }
        if(B2_IS_NULL(body->_ZFP_P2Body_d->implBodyId)) {
            _ZFP_P2BodyImplCreate(world, body);
        }
        else {
            body->_ZFP_P2Body_d->massUpdate();
        }
    }
    pendingBody.clear();
}

static void _ZFP_P2WorldImplUpdate_pendingJoint(ZF_IN P2World *world) {
    zfimplhashmap<P2Joint *, zfbool> &pendingJoint = world->_ZFP_P2World_d->pendingJoint;
    for(zfimplhashmap<P2Joint *, zfbool>::iterator it = pendingJoint.begin(); it != pendingJoint.end(); ++it) {
        P2Joint *joint = it->first;
        if(B2_IS_NON_NULL(joint->_ZFP_P2Joint_d->implJointId)) {
            b2DestroyJoint(joint->_ZFP_P2Joint_d->implJointId);
            joint->_ZFP_P2Joint_d->implJointId = b2_nullJointId;
        }
        P2Body *ownerBody0 = zfnull;
        P2Body *ownerBody1 = zfnull;
        if(joint->_ZFP_P2Joint_d->ownerWorld) {
            ownerBody0 = joint->_ZFP_P2Joint_d->ownerWorld->p2_bodyFind(joint->p2_bodyId0());
            ownerBody1 = joint->_ZFP_P2Joint_d->ownerWorld->p2_bodyFind(joint->p2_bodyId1());
        }
        else {
            ownerBody0 = joint->_ZFP_P2Joint_d->ownerBody->p2_bodyFind(joint->p2_bodyId0());
            ownerBody1 = joint->_ZFP_P2Joint_d->ownerBody->p2_bodyFind(joint->p2_bodyId1());
        }
        if(ownerBody0 == zfnull) {
            ZFLogTrim("P2Joint: no body found, bodyId0: %s, joint: %s, owner: %s", joint->p2_bodyId0(), joint, joint->p2_owner());
            continue;
        }
        if(ownerBody1 == zfnull) {
            ZFLogTrim("P2Joint: no body found, bodyId1: %s, joint: %s, owner: %s", joint->p2_bodyId1(), joint, joint->p2_owner());
            continue;
        }
        _ZFP_P2JointPrivate::jointCreate(ownerBody0, ownerBody1, joint);

        ownerBody0->_ZFP_P2Body_d->bodyRefJointList->add(joint);
        ownerBody1->_ZFP_P2Body_d->bodyRefJointList->add(joint);

        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach)
                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2JointAttach)
                ) {
            world->observerNotify(P2World::E_P2JointAttach(), ZFArgs()
                    .param0(joint)
                    );
        }
    }
    pendingJoint.clear();
}

static void _ZFP_P2WorldImplUpdate_events(ZF_IN P2World *world) {
    {
        b2BodyEvents implEventList = b2World_GetBodyEvents(world->_ZFP_P2World_d->implWorldId);
        {
            ZFCoreArray<P2BodyMoveEventData> &eventList = world->_ZFP_P2World_d->bodyMoveEvent->p2_moveEventList;
            eventList.resize(implEventList.moveCount);
            for(int i = 0, j = 0; i < implEventList.moveCount; ++i) {
                b2BodyMoveEvent const &src = implEventList.moveEvents[i];
                if(!b2Body_IsValid(src.bodyId)) {
                    eventList.resize(eventList.count() - 1);
                    continue;
                }
                P2BodyMoveEventData &dst = eventList[j++];
                dst.p2_body = (P2Body *)b2Body_GetUserData(src.bodyId);
                dst.p2_position = b2Vec2ToZF(src.transform.p);
                dst.p2_rotation = b2RotToZF(src.transform.q);
            }
        }
        if(!world->_ZFP_P2World_d->pendingBodyMove.empty()) {
            ZFCoreArray<P2BodyMoveEventData> &eventList = world->_ZFP_P2World_d->bodyMoveEvent->p2_moveEventList;
            for(zfimplhashmap<P2Body *, zfbool>::iterator it = world->_ZFP_P2World_d->pendingBodyMove.begin(); it != world->_ZFP_P2World_d->pendingBodyMove.end(); ++it) {
                ZFCoreQueuePOD<P2Body *> toMove;
                toMove.add(it->first);
                while(!toMove.isEmpty()) {
                    P2Body *body = toMove.take();
                    ZFArray *l = body->p2_bodyList();
                    for(zfindex i = 0; i < l->count(); ++i) {
                        toMove.add(l->get(i));
                    }
                    zfbool exist = zffalse;
                    for(zfindex i = eventList.count() - 1; i != zfindexMax(); --i) {
                        if(eventList[i].p2_body == body) {
                            exist = zftrue;
                            break;
                        }
                    }
                    if(!exist) {
                        P2BodyMoveEventData dst;
                        dst.p2_body = it->first;
                        dst.p2_position = it->first->p2_positionCur();
                        dst.p2_rotation = it->first->p2_rotationCur();
                        eventList.add(dst);
                    }
                }
            }
            world->_ZFP_P2World_d->pendingBodyMove.clear();
        }
        if(!world->_ZFP_P2World_d->bodyMoveEvent->p2_moveEventList.isEmpty()) {
            world->p2impl->bodyMoveEvent(world, world->_ZFP_P2World_d->bodyMoveEvent);
        }
        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent)
                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyMoveEvent)
                ) {
            if(!world->_ZFP_P2World_d->bodyMoveEvent->p2_moveEventList.isEmpty()) {
                world->observerNotify(P2World::E_P2BodyMoveEvent(), ZFArgs()
                        .param0(world->_ZFP_P2World_d->bodyMoveEvent)
                        );
            }
        }
    }

    {
        b2SensorEvents implEventList = b2World_GetSensorEvents(world->_ZFP_P2World_d->implWorldId);
        {
            ZFCoreArray<P2SensorEventData> &eventList = world->_ZFP_P2World_d->sensorEvent->p2_sensorEnterList;
            eventList.resize(implEventList.beginCount);
            for(int i = 0, j = 0; i < implEventList.beginCount; ++i) {
                b2SensorBeginTouchEvent const &src = implEventList.beginEvents[i];
                if(!b2Shape_IsValid(src.sensorShapeId) || !b2Shape_IsValid(src.visitorShapeId)) {
                    eventList.resize(eventList.count() - 1);
                    continue;
                }
                P2SensorEventData &dst = eventList[j++];
                dst.p2_shape0 = (P2Shape *)b2Shape_GetUserData(src.sensorShapeId);
                dst.p2_shape1 = (P2Shape *)b2Shape_GetUserData(src.visitorShapeId);
                if(dst.p2_shape0 && dst.p2_shape1) {
                    P2Body *body0 = dst.p2_shape0->p2_ownerBody();
                    P2Body *body1 = dst.p2_shape1->p2_ownerBody();
                    if(body0 && body1) {
                        if(ZFBitTest(body0->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorEnter)
                                ) {
                            body0->observerNotify(P2Body::E_P2SensorEnter(), ZFArgs()
                                    .param0(body1)
                                    );
                        }
                        if(ZFBitTest(body1->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorEnter)
                                ) {
                            body1->observerNotify(P2Body::E_P2SensorVisitorEnter(), ZFArgs()
                                    .param0(body0)
                                    );
                        }
                    }
                }
            }
        }
        {
            ZFCoreArray<P2SensorEventData> &eventList = world->_ZFP_P2World_d->sensorEvent->p2_sensorExitList;
            eventList.resize(implEventList.endCount);
            for(int i = 0, j = 0; i < implEventList.endCount; ++i) {
                b2SensorEndTouchEvent const &src = implEventList.endEvents[i];
                if(!b2Shape_IsValid(src.sensorShapeId) || !b2Shape_IsValid(src.visitorShapeId)) {
                    eventList.resize(eventList.count() - 1);
                    continue;
                }
                P2SensorEventData &dst = eventList[j++];
                dst.p2_shape0 = (P2Shape *)b2Shape_GetUserData(src.sensorShapeId);
                dst.p2_shape1 = (P2Shape *)b2Shape_GetUserData(src.visitorShapeId);
                if(dst.p2_shape0 && dst.p2_shape1) {
                    P2Body *body0 = dst.p2_shape0->p2_ownerBody();
                    P2Body *body1 = dst.p2_shape1->p2_ownerBody();
                    if(body0 && body1) {
                        if(ZFBitTest(body0->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorExit)
                                ) {
                            body0->observerNotify(P2Body::E_P2SensorExit(), ZFArgs()
                                    .param0(body1)
                                    );
                        }
                        if(ZFBitTest(body1->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2SensorVisitorExit)
                                ) {
                            body1->observerNotify(P2Body::E_P2SensorVisitorExit(), ZFArgs()
                                    .param0(body0)
                                    );
                        }
                    }
                }
            }
        }
        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent)
                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2SensorEvent)
                ) {
            if(!world->_ZFP_P2World_d->sensorEvent->p2_sensorEnterList.isEmpty()
                    || !world->_ZFP_P2World_d->sensorEvent->p2_sensorExitList.isEmpty()
                    ) {
                world->observerNotify(P2World::E_P2SensorEvent(), ZFArgs()
                        .param0(world->_ZFP_P2World_d->sensorEvent)
                        );
            }
        }
    }

    {
        b2ContactEvents implEventList = b2World_GetContactEvents(world->_ZFP_P2World_d->implWorldId);
        {
            ZFCoreArray<P2ContactEventData> &eventList = world->_ZFP_P2World_d->contactEvent->p2_contactEnterList;
            eventList.resize(implEventList.beginCount);
            for(int i = 0, j = 0; i < implEventList.beginCount; ++i) {
                b2ContactBeginTouchEvent const &src = implEventList.beginEvents[i];
                if(!b2Shape_IsValid(src.shapeIdA) || !b2Shape_IsValid(src.shapeIdB)) {
                    eventList.resize(eventList.count() - 1);
                    continue;
                }
                P2ContactEventData &dst = eventList[j++];
                dst.p2_shape0 = (P2Shape *)b2Shape_GetUserData(src.shapeIdA);
                dst.p2_shape1 = (P2Shape *)b2Shape_GetUserData(src.shapeIdB);
                _ZFP_P2ContactEventDataFromManifold(dst, src.manifold);
                if(dst.p2_shape0 && dst.p2_shape1) {
                    P2Body *body0 = dst.p2_shape0->p2_ownerBody();
                    P2Body *body1 = dst.p2_shape1->p2_ownerBody();
                    if(body0 && body1) {
                        if(ZFBitTest(body0->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter)
                                ) {
                            body0->observerNotify(P2Body::E_P2ContactEnter(), ZFArgs()
                                    .param0(body1)
                                    .param1(zfobj<v_P2ContactEventData>(dst))
                                    );
                        }
                        if(ZFBitTest(body1->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactEnter)
                                ) {
                            body1->observerNotify(P2Body::E_P2ContactEnter(), ZFArgs()
                                    .param0(body0)
                                    .param1(zfobj<v_P2ContactEventData>(dst))
                                    );
                        }
                    }
                }
            }
        }
        {
            ZFCoreArray<P2ContactEventData> &eventList = world->_ZFP_P2World_d->contactEvent->p2_contactExitList;
            eventList.resize(implEventList.endCount);
            for(int i = 0, j = 0; i < implEventList.endCount; ++i) {
                b2ContactEndTouchEvent const &src = implEventList.endEvents[i];
                if(!b2Shape_IsValid(src.shapeIdA) || !b2Shape_IsValid(src.shapeIdB)) {
                    eventList.resize(eventList.count() - 1);
                    continue;
                }
                P2ContactEventData &dst = eventList[j++];
                zfmemset(&dst, 0, sizeof(dst));
                dst.p2_shape0 = (P2Shape *)b2Shape_GetUserData(src.shapeIdA);
                dst.p2_shape1 = (P2Shape *)b2Shape_GetUserData(src.shapeIdB);
                dst.p2_normal = ZFUIPointZero();
                dst.p2_contactEnable = true;
                if(dst.p2_shape0 && dst.p2_shape1) {
                    P2Body *body0 = dst.p2_shape0->p2_ownerBody();
                    P2Body *body1 = dst.p2_shape1->p2_ownerBody();
                    if(body0 && body1) {
                        if(ZFBitTest(body0->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit)
                                ) {
                            body0->observerNotify(P2Body::E_P2ContactExit(), ZFArgs()
                                    .param0(body1)
                                    .param1(zfobj<v_P2ContactEventData>(dst))
                                    );
                        }
                        if(ZFBitTest(body1->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit)
                                || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2ContactExit)
                                ) {
                            body1->observerNotify(P2Body::E_P2ContactExit(), ZFArgs()
                                    .param0(body0)
                                    .param1(zfobj<v_P2ContactEventData>(dst))
                                    );
                        }
                    }
                }
            }
        }
        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent)
                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2ContactEvent)
                ) {
            if(!world->_ZFP_P2World_d->contactEvent->p2_contactEnterList.isEmpty()
                    || !world->_ZFP_P2World_d->contactEvent->p2_contactExitList.isEmpty()
                    ) {
                world->observerNotify(P2World::E_P2ContactEvent(), ZFArgs()
                        .param0(world->_ZFP_P2World_d->contactEvent)
                        );
            }
        }
    }
}

static bool _ZFP_P2WorldImplUpdate_visibleBodyUpdate_cb(b2ShapeId shapeId, void *context) {
    P2World *world = (P2World *)context;
    if(b2Shape_IsValid(shapeId)) {
        P2Shape *shape = (P2Shape *)b2Shape_GetUserData(shapeId);
        P2Body *body = shape->p2_ownerBody();
        while(body->_ZFP_P2Body_d->ownerBody) {
            body = body->_ZFP_P2Body_d->ownerBody;
        }
        world->_ZFP_P2World_d->visibleBody[body] = zftrue;
    }
    return zftrue;
}
static void _ZFP_P2WorldImplUpdate_visibleBodyUpdate(ZF_IN P2World *world) {
    zfimplhashmap<P2Body *, zfbool> &visibleBody = world->_ZFP_P2World_d->visibleBody;
    zfimplhashmap<P2Body *, zfbool> &visibleBodyPrev = world->_ZFP_P2World_d->visibleBodyPrev;
    visibleBody.swap(visibleBodyPrev);
    visibleBody.clear();

    if(!ZFUIRectIsEmpty(world->p2_UIVisibleArea())) {
        b2AABB implAABB;
        {
            const ZFUIPoint &UIOffset = world->p2_UIOffset();
            const ZFUIRect &UIVisibleArea = world->p2_UIVisibleArea();
            implAABB.lowerBound.x = UIOffset.x + UIVisibleArea.x;
            implAABB.lowerBound.y = UIOffset.y + UIVisibleArea.y;
            implAABB.upperBound.x = UIOffset.x + UIVisibleArea.x + UIVisibleArea.width;
            implAABB.upperBound.y = UIOffset.y + UIVisibleArea.y + UIVisibleArea.height;
        }

        b2QueryFilter implFilter;
        implFilter.categoryBits = P2FilterMaskAll();
        implFilter.maskBits = P2FilterMaskAll();

        b2World_OverlapAABB(world->_ZFP_P2World_d->implWorldId, implAABB, implFilter, _ZFP_P2WorldImplUpdate_visibleBodyUpdate_cb, world);
    }

    P2BodyVisibilityEvent *event = world->_ZFP_P2World_d->bodyVisibilityEvent;
    event->p2_bodyEnterList.removeAll();
    event->p2_bodyExitList.removeAll();
    for(zfimplhashmap<P2Body *, zfbool>::iterator it = visibleBody.begin(); it != visibleBody.end(); ++it) {
        P2Body *body = it->first;
        if(visibleBodyPrev.find(body) == visibleBodyPrev.end()) {
            event->p2_bodyEnterList.add(body);
            ZFBitSet(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyVisible);
            if(ZFBitTest(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate)
                    || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate)
                    ) {
                body->observerNotify(P2Body::E_P2VisibilityUpdate());
            }
        }
    }
    for(zfimplhashmap<P2Body *, zfbool>::iterator it = visibleBodyPrev.begin(); it != visibleBodyPrev.end(); ++it) {
        P2Body *body = it->first;
        if(visibleBody.find(body) == visibleBody.end()) {
            event->p2_bodyExitList.add(body);
            ZFBitUnset(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_bodyVisible);
            if(ZFBitTest(body->_ZFP_P2Body_d->stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate)
                    || ZFBitTest(_ZFP_P2Body_stateFlag, _ZFP_P2BodyPrivate::stateFlag_E_P2VisibilityUpdate)
                    ) {
                body->observerNotify(P2Body::E_P2VisibilityUpdate());
            }
        }
    }
    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent)
            || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2BodyVisibilityEvent)
            ) {
        if(!event->p2_bodyEnterList.isEmpty() || event->p2_bodyExitList.isEmpty()) {
            world->observerNotify(P2World::E_P2BodyVisibilityEvent(), ZFArgs()
                    .param0(world->_ZFP_P2World_d->bodyVisibilityEvent)
                    );
        }
    }
}

static void _ZFP_P2WorldImplUpdate(ZF_IN P2World *world) {
    ZFCoreAssertWithMessageTrim(world->p2impl
            , "p2impl not set, world: %s"
            , world
            );

    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev)
            || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePrev)
            ) {
        world->observerNotify(P2World::E_P2UpdatePrev());
    }
    _ZFP_P2WorldImplUpdate_pendingBody(world);
    _ZFP_P2WorldImplUpdate_pendingJoint(world);

    ZFBitSet(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_updateRunning);
    b2World_Step(world->_ZFP_P2World_d->implWorldId, 1.0f / ZFGlobalTimerInterval(), 4);
    _ZFP_P2WorldImplUpdate_events(world);
    _ZFP_P2WorldImplUpdate_visibleBodyUpdate(world);
    ZFBitUnset(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_updateRunning);

    for(zfimplhashmap<P2Body *, zfbool>::iterator it = world->_ZFP_P2World_d->customUpdateBody.begin(); it != world->_ZFP_P2World_d->customUpdateBody.end(); ++it) {
        it->first->observerNotify(P2Body::E_P2Update());
    }

    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag)) {
        ZFBitUnset(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIUpdateFlag);
        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIChangedMask)) {
            world->_ZFP_P2World_d->visibleArea = ZFUIRectApplyMargin(ZFUIRectCreate(
                        0
                        , 0
                        , world->p2_UISize().width
                        , world->p2_UISize().height
                        )
                    , world->p2_UIVisibleAreaMargin()
                    );
        }
        world->p2impl->UIUpdate(world);
        if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate)
                || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UIUpdate)
                ) {
            world->observerNotify(P2World::E_P2UIUpdate());
        }
        ZFBitUnset(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_UIChangedMask);
    }
    if(ZFBitTest(world->_ZFP_P2World_d->stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost)
            || ZFBitTest(_ZFP_P2World_stateFlag, _ZFP_P2WorldPrivate::stateFlag_E_P2UpdatePost)
            ) {
        world->observerNotify(P2World::E_P2UpdatePost());
    }
}

ZF_NAMESPACE_GLOBAL_END

