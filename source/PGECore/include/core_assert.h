#ifndef PGE_CORE_CORE_ASSERT_H
#define PGE_CORE_CORE_ASSERT_H

#include "core_log.h"

namespace pge
{
#ifndef core_CrashAndBurn
#    define core_CrashAndBurn(reason)                                                                   \
        pge::core_LogErrorf("CRASHED: in file %s at line %d, because: %s", __FILE__, __LINE__, reason); \
        __debugbreak();
#endif


#ifndef core_Assert
#    ifdef _DEBUG
#        define core_Assert(expr)                                                                          \
            if (!(expr)) {                                                                                 \
                pge::core_LogErrorf("ASSERT FAILED: %s in file %s at line %d", #expr, __FILE__, __LINE__); \
                __debugbreak();                                                                            \
            } else {                                                                                       \
            }
#        define core_AssertWithReason(expr, reason)                                                                             \
            if (!(expr)) {                                                                                                      \
                pge::core_LogErrorf("ASSERT FAILED: %s in file %s at line %d, because: %s", #expr, __FILE__, __LINE__, reason); \
                __debugbreak();                                                                                                 \
            } else {                                                                                                            \
            }
#    else
#        define core_Assert(expr)                   /*no-op*/
#        define core_AssertWithReason(expr, reason) /*no-op*/
#    endif
#endif // #ifndef core_Assert


#ifndef core_Verify
#    ifdef _DEBUG
#        define core_Verify(expr)                                                                          \
            if (!(expr)) {                                                                                 \
                pge::core_LogErrorf("VERIFY FAILED: %s in file %s at line %d", #expr, __FILE__, __LINE__); \
                __debugbreak();                                                                            \
            } else {                                                                                       \
            }
#        define core_VerifyWithReason(expr, reason)                                                                             \
            if (!(expr)) {                                                                                                      \
                pge::core_LogErrorf("VERIFY FAILED: %s in file %s at line %d, because: %s", #expr, __FILE__, __LINE__, reason); \
                __debugbreak();                                                                                                 \
            } else {                                                                                                            \
            }
#    else
#        define core_Verify(expr)                   expr
#        define core_VerifyWithReason(expr, reason) expr
#    endif
#endif // #ifndef core_Verify

} // namespace pge

#endif // #ifndef PGE_CORE_CORE_ASSERT_H