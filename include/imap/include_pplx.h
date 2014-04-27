#ifndef IMAP_INCLUDE_PPLX_H
#define IMAP_INCLUDE_PPLX_H

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) 
#include <ppltasks.h>
namespace pplx = concurrency;
#else
#include <pplx/pplxtasks.h>
#endif

#endif