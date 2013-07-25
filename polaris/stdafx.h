#ifndef STDAFX_H
#define STDAFX_H

#define NEW(x)          (x*)malloc(sizeof(x))
#define NEWARR(x,y)     (x*)malloc(sizeof(x)*y)
#define DEL(x)          free(x)
#define DELARR(x)       free(x)

#endif // STDAFX_H
