#pragma once

#include <QtGlobal>

#if defined(MEMBER_D_EXPORTS)
#define MEMBER_D_API Q_DECL_EXPORT
#else
#define MEMBER_D_API Q_DECL_IMPORT
#endif
