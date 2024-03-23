#ifndef __SCULL_H__
#define __SCULL_H__

#include <linux/types.h>

#define SCULL_DEV_NAME	"scull"
#define SCULL_DEV_CLASS "scull"
#define SCULL_MEM_SIZE  (128 << 20)

#define NUM_SCULL_DEV	4
#define NUM_QUANTUM	4096	
#define NUM_QSET	4

#define SCULL_MINOR     0

struct class;

struct scull_module {
	struct class *scull_class;
	dev_t dev;
	int major;
	int minor;
};

extern struct scull_module module;

#endif /* __SCULL_H__ */