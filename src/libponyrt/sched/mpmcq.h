#ifndef sched_mpmcq_h
#define sched_mpmcq_h

#include <stdint.h>
#include <platform.h>

PONY_EXTERN_C_BEGIN

typedef struct mpmcq_node_t mpmcq_node_t;

typedef struct mpmcq_dwcas_t
{
  union
  {
    struct
    {
      uintptr_t aba;
      mpmcq_node_t* node;
    };

    dw_t dw;
  };
} mpmcq_dwcas_t;

typedef struct mpmcq_t
{
  mpmcq_node_t* volatile head;
  mpmcq_dwcas_t tail;
} mpmcq_t;

void ponyint_mpmcq_init(mpmcq_t* q);

void ponyint_mpmcq_destroy(mpmcq_t* q);

void ponyint_mpmcq_push(mpmcq_t* q, void* data);

void ponyint_mpmcq_push_single(mpmcq_t* q, void* data);

void* ponyint_mpmcq_pop(mpmcq_t* q);

PONY_EXTERN_C_END

#endif
