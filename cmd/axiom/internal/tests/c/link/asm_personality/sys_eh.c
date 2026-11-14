// The exception's raise and the personality routine of main.c's assembly
// procedures: each frame's LSDA (its first word says which procedure's)
// is given to it; asm_catch's says where its handler is.
#include <stdint.h>
#include <unwind.h>

int sys_seen;
static struct _Unwind_Exception ex;

_Unwind_Reason_Code sys_personality(int version, _Unwind_Action actions, _Unwind_Exception_Class cls,
                                    struct _Unwind_Exception *e, struct _Unwind_Context *ctx) {
    (void)version;
    (void)cls;
    (void)e;
    const uint32_t *lsda = (const uint32_t *)_Unwind_GetLanguageSpecificData(ctx);
    if (!lsda) return _URC_FATAL_PHASE1_ERROR;
    if (lsda[0] == 0x50415353) {  // asm_pass: nothing to do
        if (actions & _UA_SEARCH_PHASE) sys_seen |= 1;
        if (actions & _UA_CLEANUP_PHASE) sys_seen |= 2;
        return _URC_CONTINUE_UNWIND;
    }
    if (lsda[0] != 0x4c534441) return _URC_FATAL_PHASE1_ERROR;
    if (actions & _UA_SEARCH_PHASE) return _URC_HANDLER_FOUND;
    sys_seen |= 4;
    _Unwind_SetIP(ctx, (_Unwind_Ptr)((const uint64_t *)lsda)[1]);
    return _URC_INSTALL_CONTEXT;
}

void sys_raise(void) {
    ex.exception_class = 0x41584f4d43000000ULL;
    ex.exception_cleanup = 0;
    _Unwind_RaiseException(&ex);
    sys_seen = -1;  // (not reached: the handler is above)
}
