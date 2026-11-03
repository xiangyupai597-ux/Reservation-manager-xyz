#pragma once

#include <QtGlobal>

#if defined(MEMBER_B_EXPORTS)
#define MEMBER_B_API Q_DECL_EXPORT
#else
#define MEMBER_B_API Q_DECL_IMPORT
#endif
