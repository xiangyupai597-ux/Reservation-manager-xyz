#pragma once

#include <QtGlobal>

#if defined(MEMBER_C_EXPORTS)
#define MEMBER_C_API Q_DECL_EXPORT
#else
#define MEMBER_C_API Q_DECL_IMPORT
#endif
