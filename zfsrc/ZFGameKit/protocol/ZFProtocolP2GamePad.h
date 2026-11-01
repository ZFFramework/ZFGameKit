/**
 * @file ZFProtocolP2GamePad.h
 * @brief protocol for P2GamePad
 */

#ifndef _ZFI_ZFProtocolP2GamePad_h_
#define _ZFI_ZFProtocolP2GamePad_h_

#include "ZFCore/ZFProtocol.h"
#include "../P2GamePad.h"
ZF_NAMESPACE_GLOBAL_BEGIN

/**
 * @brief for impl to notify button value change
 *
 * for buttons that does not support ranged value, the value should be 0 or 1 only
 */
ZFMETHOD_FUNC_DECLARE_3(ZFLIB_ZFGameKit, void, P2GamePadImpl_notifyButtonValue
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(P2GamePadButton, btn)
        , ZFMP_IN(zffloat, value)
        )
/**
 * @brief for impl to notify pad connected or disconnected
 *
 * note: when disconnected, all button state would be cleared
 */
ZFMETHOD_FUNC_DECLARE_2(ZFLIB_ZFGameKit, void, P2GamePadImpl_notifyPadChange
        , ZFMP_IN(zfindex, padIndex)
        , ZFMP_IN(zfbool, connected)
        )

// ============================================================
/**
 * @brief protocol for P2GamePad
 *
 * this is a dummy protocol for convenient,
 * use #P2GamePadImpl_notifyPadChange and #P2GamePadImpl_notifyButtonValue
 * for actual impl
 */
ZFPROTOCOL_INTERFACE_BEGIN(ZFLIB_ZFGameKit, P2GamePad)
ZFPROTOCOL_INTERFACE_OPTIONAL
ZFPROTOCOL_INTERFACE_END(P2GamePad)

ZF_NAMESPACE_GLOBAL_END
#endif // #ifndef _ZFI_ZFProtocolP2GamePad_h_

