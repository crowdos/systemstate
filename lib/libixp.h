#ifndef LIB_IXP_H
#define LIB_IXP_H

extern "C" {
#include <ixp.h>
};

template<class T> T *ixp_mallocz(uint size) {
  return reinterpret_cast<T *>(ixp_emallocz(size));
}

#endif /* LIB_IXP_H */
