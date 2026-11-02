#include "P2GamePad.h"
#include "protocol/ZFProtocolP2GamePad.h"

ZF_NAMESPACE_GLOBAL_BEGIN

ZFENUM_DEFINE(P2GamePadButton)

ZF_NAMESPACE_BEGIN(ZFGlobalEvent)
ZFEVENT_GLOBAL_REGISTER(P2GamePadChange)
ZF_NAMESPACE_END(ZFGlobalEvent)

// ============================================================
zfclassNotPOD _ZFP_P2GamePad {
public:
    enum {
        PAD_MAX = 8,
    };
    zfclassNotPOD Pad {
    public:
        ZFObserver ob;
        zfbool connected;
        ZFCoreMap<P2GamePadButton, zffloat> valueMap;
        ZFCoreMap<P2GamePadButton, P2GamePadButton> remapMap;
    public:
        void valueMapRemoveAll(void) {
            for(zfiter it = valueMap.iter(); it; ++it) {
                valueMap.iterValue(0);
            }
        }
    };
public:
    static Pad *pads(void) {
        static Pad d[PAD_MAX];
        return d;
    }
    static ZFCoreArray<zfindex> &indexList(void) {
        static ZFCoreArray<zfindex> d;
        return d;
    }
    static Pad &changeState(ZF_IN zfindex padIndex, ZF_IN zfbool connected) {
        Pad &pad = pads()[padIndex];
        if(pad.connected != connected) {
            pad.connected = connected;
            ZFCoreArray<zfindex> &list = indexList();
            if(connected) {
                list.add(padIndex);
            }
            else {
                list.removeElement(padIndex);
                pad.valueMapRemoveAll();
            }
            ZFGlobalObserver().observerNotify(ZFGlobalEvent::E_P2GamePadChange());
        }
        return pad;
    }
    static ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &remapKeyMap(void) {
        static ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > d;
        return d;
    }
    static void remapKeyObserverUpdate(void) {
        static ZFListener _attached;
        if(_attached.valid() == !remapKeyMap().isEmpty()) {
            return;
        }
        if(remapKeyMap().isEmpty()) {
            ZFGlobalObserver().observerRemove(ZFUIView::E_ViewOnEvent(), _attached);
            _attached = zfnull;
        }
        else {
            _attached = ZFCallbackForFunc(_viewOnEvent);
            ZFGlobalObserver().observerAdd(ZFUIView::E_ViewOnEvent(), _attached, ZFLevelZFFrameworkNormal);
        }
    }
private:
    static void _viewOnEvent(ZF_IN const ZFArgs &zfargs) {
        ZFUIKeyEvent *event = zfargs.param0();
        if(!event) {
            return;
        }
        switch(event->keyAction) {
            case v_ZFUIKeyAction::e_Down:
            case v_ZFUIKeyAction::e_Repeat:
                break;
            case v_ZFUIKeyAction::e_Up:
            case v_ZFUIKeyAction::e_Cancel:
                break;
            default:
                return;
        }
    }
};

ZF_GLOBAL_INITIALIZER_INIT_WITH_LEVEL(P2GamePadCleanup, ZFLevelZFFrameworkNormal) {
}
ZF_GLOBAL_INITIALIZER_DESTROY(P2GamePadCleanup) {
    _ZFP_P2GamePad::Pad *pads = _ZFP_P2GamePad::pads();
    for(zfindex i = 0; i < _ZFP_P2GamePad::PAD_MAX; ++i) {
        _ZFP_P2GamePad::Pad &pad = pads[i];
        pad.ob.observerRemoveAll();
        pad.connected = zffalse;
        pad.valueMapRemoveAll();
    }
}
ZF_GLOBAL_INITIALIZER_END(P2GamePadCleanup)

// ============================================================
ZFMETHOD_FUNC_DEFINE_0(zfindex, P2GamePadMax) {
    return _ZFP_P2GamePad::PAD_MAX;
}

ZFMETHOD_FUNC_DEFINE_1(ZFObserver, P2GamePadObserver
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].ob;
}
ZFMETHOD_FUNC_DEFINE_2(zfbool, P2GamePadPressed
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].valueMap.access(btn) != 0;
}
ZFMETHOD_FUNC_DEFINE_2(zffloat, P2GamePadValue
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreAssert(padIndex < _ZFP_P2GamePad::PAD_MAX);
    return _ZFP_P2GamePad::pads()[padIndex].valueMap.access(btn);
}

ZFMETHOD_FUNC_DEFINE_0(ZFCoreArray<zfindex>, P2GamePadIndexList) {
    return _ZFP_P2GamePad::indexList();
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2GamePadRemap
        , ZFMP_IN(P2GamePadButton, from)
        , ZFMP_IN(P2GamePadButton, to)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    if(from == ZFEnumInvalid() || to == ZFEnumInvalid()) {
        _ZFP_P2GamePad::pads()[padIndex].remapMap.remove(from);
    }
    else {
        _ZFP_P2GamePad::pads()[padIndex].remapMap.access(from) = to;
    }
}
ZFMETHOD_FUNC_DEFINE_3(void, P2GamePadRemap
        , ZFMP_IN(ZFUIKeyCode, from)
        , ZFMP_IN(P2GamePadButton, to)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    if(from == ZFEnumInvalid()) {
        return;
    }
    ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &m = _ZFP_P2GamePad::remapKeyMap();
    zfiter it = m.iterFind(from);
    if(to == ZFEnumInvalid()) {
        if(it) {
            ZFCoreMap<zfindex, P2GamePadButton> &t = m.iterValue(it);
            t.remove(padIndex);
            if(t.isEmpty()) {
                m.iterRemove(it);
            }
        }
    }
    else {
        m.access(from).access(padIndex) = to;
    }
    _ZFP_P2GamePad::remapKeyObserverUpdate();
}

ZFMETHOD_FUNC_DEFINE_2(P2GamePadButton, P2GamePadRemapGet
        , ZFMP_IN(P2GamePadButton, from)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreMap<P2GamePadButton, P2GamePadButton> &m = _ZFP_P2GamePad::pads()[padIndex].remapMap;
    zfiter it = m.iterFind(from);
    if(it) {
        return m.iterValue(it);
    }
    else {
        return (P2GamePadButton)ZFEnumInvalid();
    }
}
ZFMETHOD_FUNC_DEFINE_2(P2GamePadButton, P2GamePadRemapGet
        , ZFMP_IN(ZFUIKeyCode, from)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &m = _ZFP_P2GamePad::remapKeyMap();
    zfiter it = m.iterFind(from);
    if(it) {
        ZFCoreMap<zfindex, P2GamePadButton> &t = m.iterValue(it);
        zfiter it2 = t.iterFind(padIndex);
        if(it2) {
            return t.iterValue(it2);
        }
    }
    return (P2GamePadButton)ZFEnumInvalid();
}

ZFMETHOD_FUNC_DEFINE_1(zfautoT<ZFHashMap>, P2GamePadRemapGetAllGamePadButton
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    zfobj<ZFHashMap> ret;
    ZFCoreMap<P2GamePadButton, P2GamePadButton> &m = _ZFP_P2GamePad::pads()[padIndex].remapMap;
    for(zfiter it = m.iter(); it; ++it) {
        ret->set(
                zfobj<v_P2GamePadButton>(m.iterKey(it))
                , zfobj<v_P2GamePadButton>(m.iterValue(it))
                );
    }
    return ret;
}
ZFMETHOD_FUNC_DEFINE_1(zfautoT<ZFHashMap>, P2GamePadRemapGetAllKeyCode
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    zfobj<ZFHashMap> ret;
    ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &m = _ZFP_P2GamePad::remapKeyMap();
    for(zfiter it = m.iter(); it; ++it) {
        if(m.iterKey(it) != padIndex) {
            continue;
        }
        ZFCoreMap<zfindex, P2GamePadButton> &t = m.iterValue(it);
        for(zfiter it2 = t.iter(); it2; ++it2) {
            ret->set(
                    zfobj<v_ZFUIKeyCode>(m.iterKey(it))
                    , zfobj<v_P2GamePadButton>(t.iterValue(it2))
                    );
        }
    }
    return ret;
}

ZFMETHOD_FUNC_DEFINE_1(void, P2GamePadRemapRemoveAll
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    _ZFP_P2GamePad::pads()[padIndex].remapMap.removeAll();

    ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &m = _ZFP_P2GamePad::remapKeyMap();
    for(zfiter it = m.iter(); it; ) {
        ZFCoreMap<zfindex, P2GamePadButton> &t = m.iterValue(it);
        t.remove(padIndex);
        if(t.isEmpty()) {
            m.iterRemove(it);
        }
        else {
            ++it;
        }
    }
    _ZFP_P2GamePad::remapKeyObserverUpdate();
}

ZFMETHOD_FUNC_DEFINE_1(zfautoT<ZFHashMap>, P2GamePadRemapSave
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    zfobj<ZFHashMap> ret;
    {
        ZFCoreMap<P2GamePadButton, P2GamePadButton> &m = _ZFP_P2GamePad::pads()[padIndex].remapMap;
        for(zfiter it = m.iter(); it; ++it) {
            ret->set(
                    zfobj<v_P2GamePadButton>(m.iterKey(it))
                    , zfobj<v_P2GamePadButton>(m.iterValue(it))
                    );
        }
    }
    {
        ZFCoreMap<ZFUIKeyCode, ZFCoreMap<zfindex, P2GamePadButton> > &m = _ZFP_P2GamePad::remapKeyMap();
        for(zfiter it = m.iter(); it; ++it) {
            if(m.iterKey(it) != padIndex) {
                continue;
            }
            ZFCoreMap<zfindex, P2GamePadButton> &t = m.iterValue(it);
            for(zfiter it2 = t.iter(); it2; ++it2) {
                ret->set(
                        zfobj<v_ZFUIKeyCode>(m.iterKey(it))
                        , zfobj<v_P2GamePadButton>(t.iterValue(it2))
                        );
            }
        }
    }
    return ret;
}
ZFMETHOD_FUNC_DEFINE_2(void, P2GamePadRemapLoad
        , ZFMP_IN(ZFHashMap *, state)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        ) {
    if(state == zfnull) {
        return;
    }
    for(zfiter it = state->iter(); it; ++it) {
        v_P2GamePadButton *v = state->iterValue(it);
        if(!v) {
            continue;
        }
        zfauto key = state->iterKey(it);
        {
            v_P2GamePadButton *k = key;
            if(k) {
                P2GamePadRemap(k->zfv(), v->zfv());
                continue;
            }
        }
        {
            v_ZFUIKeyCode *k = key;
            if(k) {
                P2GamePadRemap(k->zfv(), v->zfv());
                continue;
            }
        }
    }
}

// ============================================================
ZFMETHOD_FUNC_DEFINE_3(void, P2GamePadImpl_notifyButtonValue
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN(zffloat, value)
        ) {
    _ZFP_P2GamePad::Pad &pad = _ZFP_P2GamePad::pads()[padIndex];
    {
        zfiter it = pad.remapMap.iterFind(btn);
        if(it) {
            btn = pad.remapMap.iterValue(it);
        }
    }

    if(value != 0) {
        _ZFP_P2GamePad::changeState(padIndex, zftrue);
    }
    zffloat &valueRef = pad.valueMap.access(btn);
    if(valueRef != value) {
        valueRef = value;
        pad.ob.observerNotify(zfidentityInvalid(), ZFArgs()
                .param0(zfobj<v_P2GamePadButton>(btn))
                .param1(zfobj<v_zffloat>(value))
                );
    }
}
ZFMETHOD_FUNC_DEFINE_2(void, P2GamePadImpl_notifyPadChange
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(zfbool, connected)
        ) {
    _ZFP_P2GamePad::changeState(padIndex, connected);
}

ZF_NAMESPACE_GLOBAL_END

