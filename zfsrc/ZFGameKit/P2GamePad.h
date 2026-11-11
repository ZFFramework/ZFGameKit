/**
 * @file P2GamePad.h
 * @brief game pad util
 */

#ifndef _ZFI_P2GamePad_h_
#define _ZFI_P2GamePad_h_

#include "ZFGameKitDef.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief game pad button define
 */
ZFENUM_BEGIN(ZFLIB_ZFGameKit, P2GamePadButton)
    ZFENUM_VALUE(DPadL) /**< @brief dpad left */
    ZFENUM_VALUE(DPadT) /**< @brief dpad top */
    ZFENUM_VALUE(DPadR) /**< @brief dpad right */
    ZFENUM_VALUE(DPadB) /**< @brief dpad bottom */
    ZFENUM_VALUE(BtnL) /**< @brief button left */
    ZFENUM_VALUE(BtnT) /**< @brief button top */
    ZFENUM_VALUE(BtnR) /**< @brief button right */
    ZFENUM_VALUE(BtnB) /**< @brief button bottom */
    ZFENUM_VALUE(Select) /**< @brief select button */
    ZFENUM_VALUE(Start) /**< @brief start button */
    ZFENUM_VALUE(L1) /**< @brief L1 */
    ZFENUM_VALUE(R1) /**< @brief R1 */
    ZFENUM_VALUE(L2) /**< @brief L2 */
    ZFENUM_VALUE(R2) /**< @brief R2 */
    ZFENUM_VALUE(L3) /**< @brief L3 */
    ZFENUM_VALUE(R3) /**< @brief R3 */
    ZFENUM_VALUE(LStickX) /**< @brief left stick's X axis, right is positive */
    ZFENUM_VALUE(LStickY) /**< @brief left stick's Y axis, top is positive */
    ZFENUM_VALUE(RStickX) /**< @brief left stick's X axis, right is positive */
    ZFENUM_VALUE(RStickY) /**< @brief left stick's Y axis, top is positive */
    ZFENUM_VALUE(Custom0) /**< @brief custom button */
    ZFENUM_VALUE(Custom1) /**< @brief custom button */
    ZFENUM_VALUE(Custom2) /**< @brief custom button */
    ZFENUM_VALUE(Custom3) /**< @brief custom button */
    ZFENUM_VALUE(Custom4) /**< @brief custom button */
    ZFENUM_VALUE(Custom5) /**< @brief custom button */
    ZFENUM_VALUE(Custom6) /**< @brief custom button */
    ZFENUM_VALUE(Custom7) /**< @brief custom button */
ZFENUM_SEPARATOR()
    ZFENUM_VALUE_REGISTER(DPadL)
    ZFENUM_VALUE_REGISTER(DPadT)
    ZFENUM_VALUE_REGISTER(DPadR)
    ZFENUM_VALUE_REGISTER(DPadB)
    ZFENUM_VALUE_REGISTER(BtnL)
    ZFENUM_VALUE_REGISTER(BtnT)
    ZFENUM_VALUE_REGISTER(BtnR)
    ZFENUM_VALUE_REGISTER(BtnB)
    ZFENUM_VALUE_REGISTER(Select)
    ZFENUM_VALUE_REGISTER(Start)
    ZFENUM_VALUE_REGISTER(L1)
    ZFENUM_VALUE_REGISTER(R1)
    ZFENUM_VALUE_REGISTER(L2)
    ZFENUM_VALUE_REGISTER(R2)
    ZFENUM_VALUE_REGISTER(L3)
    ZFENUM_VALUE_REGISTER(R3)
    ZFENUM_VALUE_REGISTER(LStickX)
    ZFENUM_VALUE_REGISTER(LStickY)
    ZFENUM_VALUE_REGISTER(RStickX)
    ZFENUM_VALUE_REGISTER(RStickY)
    ZFENUM_VALUE_REGISTER(Custom0)
    ZFENUM_VALUE_REGISTER(Custom1)
    ZFENUM_VALUE_REGISTER(Custom2)
    ZFENUM_VALUE_REGISTER(Custom3)
    ZFENUM_VALUE_REGISTER(Custom4)
    ZFENUM_VALUE_REGISTER(Custom5)
    ZFENUM_VALUE_REGISTER(Custom6)
    ZFENUM_VALUE_REGISTER(Custom7)
ZFENUM_END(ZFLIB_ZFGameKit, P2GamePadButton)
ZFENUM_REG(ZFLIB_ZFGameKit, P2GamePadButton)

// ============================================================
ZF_NAMESPACE_BEGIN(ZFGlobalEvent)
/**
 * @brief called when #P2GamePadIndexList changed
 */
ZFEVENT_GLOBAL(ZFLIB_ZFGameKit, P2GamePadChange)
/**
 * @brief called when any button value changed, see #P2GamePadObserver
 *
 * param0 would be a #v_P2GamePadButton holds button
 * whose #P2GamePadPressed or #P2GamePadValue changed,
 * param1 would be a #v_zfbool holds #P2GamePadPressed
 */
ZFEVENT_GLOBAL(ZFLIB_ZFGameKit, P2GamePadButtonUpdate)
ZF_NAMESPACE_END(ZFGlobalEvent)

/**
 * @brief max possible pads
 */
ZFMETHOD_FUNC_DECLARE_0(ZFLIB_ZFGameKit, zfindex, P2GamePadMax)

/**
 * @brief return an observer for specified padIndex
 *
 * attach observer with #ZFGlobalEvent::E_P2GamePadButtonUpdate
 *
 * it's ensured safe to attach or detach observers
 * even if game pad not connected,
 * but the returned observer object should not be stored for future use\n
 * note the observers previously attached won't be cleared
 * even if game pad disconnected (until #ZFFrameworkCleanup),
 * you must manage them manually
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZFGameKit, ZFObserver, P2GamePadObserver
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief check whether specified button pressed
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, zfbool, P2GamePadPressed
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief check specified button's value
 *
 * -  for normal button, possible values are 0 and 1
 * -  for joy sticks, ranged in [-1, 1],
 *   values inside dead zone already normalized to 0
 * -  for L2/R2, may be only 0 or 1, may be range in [0, 1], depends on game pad
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, zffloat, P2GamePadValue
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief a list of currently connected game pad's index
 *
 * note:
 * -  the index may or may not be continuous
 * -  the index ranged in [0, #P2GamePadMax)
 */
ZFMETHOD_FUNC_DECLARE_0(ZFLIB_ZFGameKit, ZFCoreArray<zfindex>, P2GamePadIndexList)

// ============================================================
/**
 * @brief remap game pad, set `to` to #ZFEnumInvalid to remove remap
 */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, void, P2GamePadRemap
        , ZFMP_IN(P2GamePadButton, from)
        , ZFMP_IN(P2GamePadButton, to)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief remap game pad, set `to` to #ZFEnumInvalid to remove remap
 *
 * typical example:
 * @code
 *   P2GamePadRemap(ZFUIKeyCode.e_kA(), P2GamePadButton.e_DPadL())
 *   P2GamePadRemap(ZFUIKeyCode.e_kW(), P2GamePadButton.e_DPadT())
 *   P2GamePadRemap(ZFUIKeyCode.e_kD(), P2GamePadButton.e_DPadR())
 *   P2GamePadRemap(ZFUIKeyCode.e_kS(), P2GamePadButton.e_DPadB())
 *   P2GamePadRemap(ZFUIKeyCode.e_kJ(), P2GamePadButton.e_BtnL())
 *   P2GamePadRemap(ZFUIKeyCode.e_kU(), P2GamePadButton.e_BtnT())
 *   P2GamePadRemap(ZFUIKeyCode.e_kI(), P2GamePadButton.e_BtnR())
 *   P2GamePadRemap(ZFUIKeyCode.e_kK(), P2GamePadButton.e_BtnB())
 *   P2GamePadRemap(ZFUIKeyCode.e_kSpace(), P2GamePadButton.e_L1())
 *   P2GamePadRemap(ZFUIKeyCode.e_kL(), P2GamePadButton.e_R1())
 *   P2GamePadRemap(ZFUIKeyCode.e_kQ(), P2GamePadButton.e_L2())
 *   P2GamePadRemap(ZFUIKeyCode.e_kO(), P2GamePadButton.e_R2())
 *   P2GamePadRemap(ZFUIKeyCode.e_kC(), P2GamePadButton.e_L3())
 *   P2GamePadRemap(ZFUIKeyCode.e_kN(), P2GamePadButton.e_R3())
 *   P2GamePadRemap(ZFUIKeyCode.e_kF(), P2GamePadButton.e_Select())
 *   P2GamePadRemap(ZFUIKeyCode.e_kH(), P2GamePadButton.e_Start())
 *
 *   // save and load
 *   ZFObjectIOSave(ZFOutputForFile('xxx.json'), P2GamePadRemapSave())
 *   P2GamePadRemapLoad(ZFObjectIOLoad(ZFInputForFile('xxx.json')))
 * @endcode
 */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, void, P2GamePadRemap
        , ZFMP_IN(ZFUIKeyCode, from)
        , ZFMP_IN(P2GamePadButton, to)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief get current remap, return #ZFEnumInvalid if none
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, P2GamePadButton, P2GamePadRemapGet
        , ZFMP_IN(P2GamePadButton, from)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief get current remap, return #ZFEnumInvalid if none
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, P2GamePadButton, P2GamePadRemapGet
        , ZFMP_IN(ZFUIKeyCode, from)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief get all of current remap
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZFGameKit, zfautoT<ZFHashMap>, P2GamePadRemapGetAllGamePadButton
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief get all of current remap
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZFGameKit, zfautoT<ZFHashMap>, P2GamePadRemapGetAllKeyCode
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief remove all remap
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZFGameKit, void, P2GamePadRemapRemoveAll
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

/**
 * @brief save remap to state
 */
ZFMETHOD_FUNC_DECLARE_1(ZFLIB_ZFGameKit, zfautoT<ZFHashMap>, P2GamePadRemapSave
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )
/**
 * @brief load remap from state
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, void, P2GamePadRemapLoad
        , ZFMP_IN(ZFHashMap *, state)
        , ZFMP_IN_OPT(zfindex, padIndex, 0)
        )

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_P2GamePad_h_

