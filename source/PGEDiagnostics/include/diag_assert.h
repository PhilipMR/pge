  #ifndef PGE_DIAGNOSTICS_DIAG_ASSERT_H
#define PGE_DIAGNOSTICS_DIAG_ASSERT_H

#include "diag_log.h"

namespace pge
{
#ifndef diag_CrashAndBurn
#    define diag_CrashAndBurn(reason)                                                                   \
        pge::diag_LogErrorf("CRASHED: in file %s at line %d, because: %s", __FILE__, __LINE__, reason); \
        __debugbreak();
#endif


#ifndef diag_Assert
#    ifdef _DEBUG
#        define diag_Assert(expr)                                                                          \
            if (!(expr)) {                                                                                 \
                pge::diag_LogErrorf("ASSERT FAILED: %s in file %s at line %d", #expr, __FILE__, __LINE__); \
                __debugbreak();                                                                            \
            } else {                                                                                       \
            }
#        define diag_AssertWithReason(expr, reason)                                                                             \
            if (!(expr)) {                                                                                                      \
                pge::diag_LogErrorf("ASSERT FAILED: %s in file %s at line %d, because: %s", #expr, __FILE__, __LINE__, reason); \
                __debugbreak();                                                                                                 \
            } else {                                                                                                            \
            }
#    else
#        define diag_Assert(expr)                   /*no-op*/
#        define diag_AssertWithReason(expr, reason) /*no-op*/
#    endif
#endif // #ifndef diag_Assert


#ifndef diag_Verify
#    ifdef _DEBUG
#        define diag_Verify(expr)                                                                          \
            if (!(expr)) {                                                                                 \
                pge::diag_LogErrorf("VERIFY FAILED: %s in file %s at line %d", #expr, __FILE__, __LINE__); \
                __debugbreak();                                                                            \
            } else {                                                                                       \
            }
#        define diag_VerifyWithReason(expr, reason)                                                                             \
            if (!(expr)) {                                                                                                      \
                pge::diag_LogErrorf("VERIFY FAILED: %s in file %s at line %d, because: %s", #expr, __FILE__, __LINE__, reason); \
                __debugbreak();                                                                                                 \
            } else {                                                                                                            \
            }
#    else
#        define diag_Verify(expr)                   expr
#        define diag_VerifyWithReason(expr, reason) expr
#    endif
#endif // #ifndef diag_Verify

} // namespace pge

#endif // #ifndef PGE_DIAGNOSTICS_DIAG_ASSERT_H