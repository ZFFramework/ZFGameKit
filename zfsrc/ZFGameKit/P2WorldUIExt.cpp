#include "P2WorldUIExt.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFCLASS_EXTEND(P2WorldView, P2World)
ZFCLASS_EXTEND(ZFUIView, P2Body)

zfclassNotPOD _ZFP_P2WorldViewPrivate : zfextend P2WorldImpl {
public:
    typedef enum {
        ScreenTileUpdateByUI,
        ScreenTileUpdateBySpeed,
        ScreenTileUpdateByOffset,
    } ScreenTileUpdateMode;
public:
    P2World *world;
    P2WorldView *worldView;
    ZFUIView *container;
    zffloat UIScalePrev;
    ScreenTileUpdateMode screenTileUpdateMode;
    ZFUIPoint screenTileUpdateParam; // screenTileIsUpdateBySpeed's speed, or screenTileIsUpdateByOffset's offset

public:
    _ZFP_P2WorldViewPrivate(void)
    : world(zfnull)
    , worldView(zfnull)
    , container(zfobjAlloc(ZFUIView))
    , UIScalePrev(1)
    , screenTileUpdateMode(ScreenTileUpdateByUI)
    , screenTileUpdateParam()
    {
    }
    ~_ZFP_P2WorldViewPrivate(void) {
        zfobjRelease(container);
    }

public:
    zfoverride
    virtual void bodyAdd(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFUIView *bodyView = zfcast(ZFUIView *, body);
        if(bodyView) {
            container->child(bodyView);
            _bodyPosUpdate(body, bodyView, body->p2_position(), body->p2_rotation());
        }
    }
    zfoverride
    virtual void bodyRemove(ZF_IN P2World *world, ZF_IN P2Body *body) {
        ZFUIView *bodyView = zfcast(ZFUIView *, body);
        if(bodyView) {
            bodyView->removeFromParent();
        }
    }
    zfoverride
    virtual void bodyMoveEvent(ZF_IN P2World *world, ZF_IN P2BodyMoveEvent *event) {
        for(zfindex i = event->p2_moveEventList.count() - 1; i != zfindexMax(); --i) {
            P2BodyMoveEventData const &data = event->p2_moveEventList[i];
            ZFUIView *bodyView = zfcast(ZFUIView *, data.p2_body);
            if(bodyView) {
                _bodyPosUpdate(data.p2_body, bodyView, data.p2_position, data.p2_rotation);
            }
        }
    }
    zfoverride
    virtual void UIUpdate(ZF_IN P2World *world) {
        ZFCoreArray<zfautoT<ZFUIView> > childList = container->childArray();
        for(zfindex i = childList.count() - 1; i != zfindexMax(); --i) {
            ZFUIView *bodyView = childList[i];
            P2Body *body = zfcast(P2Body *, bodyView);
            if(body) {
                _bodyPosUpdate(body, bodyView, body->p2_positionCur(), body->p2_rotationCur());
            }
        }

        ZFArray *screenTileBg = worldView->screenTileBgList();
        ZFArray *screenTileFg = worldView->screenTileFgList();
        if(!screenTileBg->isEmpty() || !screenTileFg->isEmpty()) {
            switch(this->screenTileUpdateMode) {
                case ScreenTileUpdateByUI: {
                    ZFUIPoint offset;
                    offset.x = world->p2_UIOffset().x * world->p2_UIScale();
                    offset.y = 0 - world->p2_UIOffset().y * world->p2_UIScale();
                    for(zfindex i = screenTileBg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileBg->get(i);
                        v->tileOffset(offset);
                    }
                    for(zfindex i = screenTileFg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileFg->get(i);
                        v->tileOffset(offset);
                    }
                    break;
                }
                case ScreenTileUpdateBySpeed:
                    for(zfindex i = screenTileBg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileBg->get(i);
                        v->tileOffsetStep(this->screenTileUpdateParam.x, this->screenTileUpdateParam.y);
                    }
                    for(zfindex i = screenTileFg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileFg->get(i);
                        v->tileOffsetStep(this->screenTileUpdateParam.x, this->screenTileUpdateParam.y);
                    }
                    break;
                case ScreenTileUpdateByOffset: {
                    for(zfindex i = screenTileBg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileBg->get(i);
                        v->tileOffset(this->screenTileUpdateParam);
                    }
                    for(zfindex i = screenTileFg->count() - 1; i != zfindexMax(); --i) {
                        P2ScreenTileView *v = screenTileFg->get(i);
                        v->tileOffset(this->screenTileUpdateParam);
                    }
                    break;
                }
                default:
                    ZFCoreCriticalShouldNotGoHere();
                    break;
            }
        }
    }
private:
    void _bodyPosUpdate(ZF_IN P2Body *body, ZF_IN ZFUIView *bodyView, ZF_IN const ZFUIPoint &position, ZF_IN zffloat rotation) {
        bodyView->UIScale(body->p2_ownerUnit()->p2_unitScale());
        bodyView->rotateZ(rotation);
        bodyView->viewFrame(P2UIRectFromBody(
                    worldView->height()
                    , world->p2_UIScale()
                    , world->p2_UIOffset()
                    , position
                    , body->p2_AABBLocal()
                    , rotation
                    , body->p2_centerOfMass()
                    ));
    }
};

ZFOBJECT_REGISTER(P2WorldView)

// ============================================================
ZFMETHOD_DEFINE_1(P2WorldView, void, screenTileBg
        , ZFMP_IN(P2ScreenTileView *, v)
        ) {
    ZFCoreAssert(v && v->parent() == zfnull);
    this->screenTileBgList()->add(v);
    this->internalImplView(v, 0);
}
ZFMETHOD_DEFINE_0(P2WorldView, zfindex, screenTileBgCount) {
    return this->screenTileBgList()->count();
}
ZFMETHOD_DEFINE_1(P2WorldView, P2ScreenTileView *, screenTileBgAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->screenTileBgList()->get(index);
}
ZFMETHOD_DEFINE_1(P2WorldView, zfautoT<P2ScreenTileView>, screenTileBgRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2ScreenTileView> ret = this->screenTileBgList()->removeAndGet(index);
    ret->removeFromParent();
    return ret;
}
ZFMETHOD_DEFINE_0(P2WorldView, void, screenTileBgRemoveAll) {
    ZFArray *l = this->screenTileBgList();
    if(!l->isEmpty()) {
        for(zfindex i = 0; i < l->count(); ++i) {
            P2ScreenTileView *v = l->get(i);
            v->removeFromParent();
        }
        l->removeAll();
    }
}

ZFMETHOD_DEFINE_1(P2WorldView, void, screenTileFg
        , ZFMP_IN(P2ScreenTileView *, v)
        ) {
    ZFCoreAssert(v && v->parent() == zfnull);
    this->screenTileFgList()->add(v);
    this->internalImplView(v);
}
ZFMETHOD_DEFINE_0(P2WorldView, zfindex, screenTileFgCount) {
    return this->screenTileFgList()->count();
}
ZFMETHOD_DEFINE_1(P2WorldView, P2ScreenTileView *, screenTileFgAt
        , ZFMP_IN(zfindex, index)
        ) {
    return this->screenTileFgList()->get(index);
}
ZFMETHOD_DEFINE_1(P2WorldView, zfautoT<P2ScreenTileView>, screenTileFgRemoveAt
        , ZFMP_IN(zfindex, index)
        ) {
    zfautoT<P2ScreenTileView> ret = this->screenTileFgList()->removeAndGet(index);
    ret->removeFromParent();
    return ret;
}
ZFMETHOD_DEFINE_0(P2WorldView, void, screenTileFgRemoveAll) {
    ZFArray *l = this->screenTileFgList();
    if(!l->isEmpty()) {
        for(zfindex i = 0; i < l->count(); ++i) {
            P2ScreenTileView *v = l->get(i);
            v->removeFromParent();
        }
        l->removeAll();
    }
}

// ============================================================
ZFMETHOD_DEFINE_0(P2WorldView, void, screenTileUpdateByUI) {
    d->screenTileUpdateMode = _ZFP_P2WorldViewPrivate::ScreenTileUpdateByUI;
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, screenTileIsUpdateByUI) {
    return d->screenTileUpdateMode == _ZFP_P2WorldViewPrivate::ScreenTileUpdateByUI;
}

ZFMETHOD_DEFINE_1(P2WorldView, void, screenTileSpeed
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    d->screenTileUpdateMode = _ZFP_P2WorldViewPrivate::ScreenTileUpdateBySpeed;
    d->screenTileUpdateParam = v;
}
ZFMETHOD_DEFINE_0(P2WorldView, ZFUIPoint, screenTileSpeed) {
    if(d->screenTileUpdateMode == _ZFP_P2WorldViewPrivate::ScreenTileUpdateBySpeed) {
        return d->screenTileUpdateParam;
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, screenTileIsUpdateBySpeed) {
    return d->screenTileUpdateMode == _ZFP_P2WorldViewPrivate::ScreenTileUpdateBySpeed;
}

ZFMETHOD_DEFINE_1(P2WorldView, void, screenTileOffset
        , ZFMP_IN(const ZFUIPoint &, v)
        ) {
    d->screenTileUpdateMode = _ZFP_P2WorldViewPrivate::ScreenTileUpdateByOffset;
    d->screenTileUpdateParam = v;
}
ZFMETHOD_DEFINE_0(P2WorldView, ZFUIPoint, screenTileOffset) {
    if(d->screenTileUpdateMode == _ZFP_P2WorldViewPrivate::ScreenTileUpdateByOffset) {
        return d->screenTileUpdateParam;
    }
    else {
        return ZFUIPointZero();
    }
}
ZFMETHOD_DEFINE_0(P2WorldView, zfbool, screenTileIsUpdateByOffset) {
    return d->screenTileUpdateMode == _ZFP_P2WorldViewPrivate::ScreenTileUpdateByOffset;
}

void P2WorldView::objectOnInit(void) {
    zfsuper::objectOnInit();
    d = zfpoolNew(_ZFP_P2WorldViewPrivate);

    zfclassNotPOD _Impl {
    public:
        static void prev(ZF_IN const ZFArgs &zfargs) {
            P2WorldView *owner = zfargs.sender();
            owner->screenTileBgRemoveAll();
            owner->screenTileFgRemoveAll();
        }
        static void post(ZF_IN const ZFArgs &zfargs) {
            P2WorldView *owner = zfargs.sender();
            {
                ZFArray *list = owner->screenTileBgList();
                for(zfindex i = 0, count = list->count(); i < count; ++i) {
                    P2ScreenTileView *v = list->get(i);
                    owner->internalImplView(v, 0);
                }
            }
            {
                ZFArray *list = owner->screenTileFgList();
                for(zfindex i = 0, count = list->count(); i < count; ++i) {
                    P2ScreenTileView *v = list->get(i);
                    owner->internalImplView(v);
                }
            }
        }
    };
    this->observerAdd(ZFSerializable::E_SerializeFromPrev(), ZFCallbackForFunc(_Impl::prev));
    this->observerAdd(ZFSerializable::E_SerializeFromPost(), ZFCallbackForFunc(_Impl::post));
}
void P2WorldView::objectOnDealloc(void) {
    zfpoolDelete(d);
    zfsuper::objectOnDealloc();
}
void P2WorldView::objectOnInitFinish(void) {
    zfsuper::objectOnInitFinish();
    d->world = zfcast(P2World *, this);
    d->world->p2impl = d;
    d->worldView = this;
    this->internalImplView(d->container)->sizeFill();
    d->UIScalePrev = d->world->p2_UIScale();
}
void P2WorldView::objectOnDeallocPrepare(void) {
    d->container->removeFromParent();
    d->world = zfnull;
    d->worldView = zfnull;
    zfcast(P2World *, this)->p2impl = zfnull;
    zfsuper::objectOnDeallocPrepare();
}

void P2WorldView::layoutOnLayout(ZF_IN const ZFUIRect &bounds) {
    zfsuper::layoutOnLayout(bounds);
    const ZFUIRect &viewFrame = this->viewFrame();
    d->world->p2_UISize(ZFUISizeCreate(viewFrame.width, viewFrame.height));
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_2(void, P2UIRectFromBodyT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2Body *, body)
        ) {
    if(body == zfnull || body->p2_ownerWorld() == zfnull) {
        rect = ZFUIRectZero();
    }
    else {
        P2World *world = body->p2_ownerWorld();
        P2UIRectFromBodyT(
                rect
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , body->p2_positionCur()
                , body->p2_AABBLocal()
                , body->p2_rotationCur()
                , body->p2_centerOfMass()
                );
    }
}
ZFMETHOD_FUNC_DEFINE_1(ZFUIRect, P2UIRectFromBody
        , ZFMP_IN(P2Body *, body)
        ) {
    ZFUIRect ret;
    P2UIRectFromBodyT(ret, body);
    return ret;
}

ZFMETHOD_FUNC_DEFINE_8(void, P2UIRectFromBodyT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        ) {
    zffloat r = (zffloat)(rotation * zfm_PI() / 180);
    zffloat cr = cos(r);
    zffloat sr = sin(r);
    zffloat w = aabbLocal.width;
    zffloat h = aabbLocal.height;
    zffloat dx = w / 2 - (centerOfMass.x - aabbLocal.x);
    zffloat dy = h / 2 - (centerOfMass.y - aabbLocal.y);
    zffloat x = position.x + dx * cr + dy * sr;
    zffloat y = position.y - dx * sr + dy * cr;

    rect.x = (x + worldUIOffset.x - w / 2) * worldUIScale;
    rect.y = worldUIHeight + (-y - worldUIOffset.y - h / 2) * worldUIScale;
    rect.width = w * worldUIScale;
    rect.height = h * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_7(ZFUIRect, P2UIRectFromBody
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, position)
        , ZFMP_IN(const ZFUIRect &, aabbLocal)
        , ZFMP_IN_OPT(zffloat, rotation, 0)
        , ZFMP_IN_OPT(const ZFUIPoint &, centerOfMass, ZFUIPointZero())
        ) {
    ZFUIRect rect;
    P2UIRectFromBodyT(
            rect
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , position
            , aabbLocal
            , rotation
            , centerOfMass
            );
    return rect;
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2UIPointFromWorldT
        , ZFMP_OUT(ZFUIPoint &, pos)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    if(world == zfnull) {
        pos = ZFUIPointZero();
    }
    else {
        P2UIPointFromWorldT(
                pos
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , worldPos
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIPoint, P2UIPointFromWorld
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    ZFUIPoint pos;
    P2UIPointFromWorldT(
            pos
            , world
            , worldPos
            );
    return pos;
}

ZFMETHOD_FUNC_DEFINE_3(void, P2UIPointToWorldT
        , ZFMP_OUT(ZFUIPoint &, worldPos)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    if(world == zfnull) {
        worldPos = ZFUIPointZero();
    }
    else {
        P2UIPointToWorldT(
                worldPos
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , pos
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIPoint, P2UIPointToWorld
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    ZFUIPoint worldPos;
    P2UIPointToWorldT(
            worldPos
            , world
            , pos
            );
    return worldPos;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIPointFromWorldT
        , ZFMP_OUT(ZFUIPoint &, pos)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    pos.x = (worldPos.x + worldUIOffset.x) * worldUIScale;
    pos.y = worldUIHeight - (worldPos.y + worldUIOffset.y) * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIPoint, P2UIPointFromWorld
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, worldPos)
        ) {
    ZFUIPoint pos;
    P2UIPointFromWorldT(
            pos
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , worldPos
            );
    return pos;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIPointToWorldT
        , ZFMP_OUT(ZFUIPoint &, worldPos)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    worldPos.x = pos.x / worldUIScale - worldUIOffset.x;
    worldPos.y = (worldUIHeight - pos.y) / worldUIScale - worldUIOffset.y;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIPoint, P2UIPointToWorld
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIPoint &, pos)
        ) {
    ZFUIPoint worldPos;
    P2UIPointToWorldT(
            worldPos
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , pos
            );
    return worldPos;
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2UIRectFromAABBT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    if(world == zfnull) {
        rect = ZFUIRectZero();
    }
    else {
        P2UIRectFromAABBT(
                rect
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , aabb
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIRect, P2UIRectFromAABB
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    ZFUIRect rect;
    P2UIRectFromAABBT(
            rect
            , world
            , aabb
            );
    return rect;
}

ZFMETHOD_FUNC_DEFINE_3(void, P2UIRectToAABBT
        , ZFMP_OUT(ZFUIRect &, aabb)
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    if(world == zfnull) {
        aabb = ZFUIRectZero();
    }
    else {
        P2UIRectToAABBT(
                aabb
                , zfcast(ZFUIView *, world)->height()
                , world->p2_UIScale()
                , world->p2_UIOffset()
                , rect
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(ZFUIRect, P2UIRectToAABB
        , ZFMP_IN(P2World *, world)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    ZFUIRect aabb;
    P2UIRectToAABBT(
            aabb
            , world
            , rect
            );
    return aabb;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIRectFromAABBT
        , ZFMP_OUT(ZFUIRect &, rect)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    rect.width = aabb.width * worldUIScale;
    rect.height = aabb.height * worldUIScale;
    rect.x = (aabb.x + worldUIOffset.x) * worldUIScale;
    rect.y = worldUIHeight - (aabb.y + aabb.height + worldUIOffset.y) * worldUIScale;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIRect, P2UIRectFromAABB
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, aabb)
        ) {
    ZFUIRect rect;
    P2UIRectFromAABBT(
            rect
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , aabb
            );
    return rect;
}

ZFMETHOD_FUNC_DEFINE_5(void, P2UIRectToAABBT
        , ZFMP_OUT(ZFUIRect &, aabb)
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    aabb.width = rect.width / worldUIScale;
    aabb.height = rect.height / worldUIScale;
    aabb.x = rect.x / worldUIScale - worldUIOffset.x;
    aabb.y = (worldUIHeight - (rect.y + rect.height)) / worldUIScale - worldUIOffset.y;
}
ZFMETHOD_FUNC_DEFINE_4(ZFUIRect, P2UIRectToAABB
        , ZFMP_IN(zffloat, worldUIHeight)
        , ZFMP_IN(zffloat, worldUIScale)
        , ZFMP_IN(const ZFUIPoint &, worldUIOffset)
        , ZFMP_IN(const ZFUIRect &, rect)
        ) {
    ZFUIRect aabb;
    P2UIRectToAABBT(
            aabb
            , worldUIHeight
            , worldUIScale
            , worldUIOffset
            , rect
            );
    return aabb;
}

ZF_NAMESPACE_GLOBAL_END

