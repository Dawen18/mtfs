/**
 * @author David Wittwer
 * @date 20.06.17.
**/

#ifndef MTFS_VISITOR_H
#define MTFS_VISITOR_H


#include "PoolManager.h"

namespace mtfs {
	class Visitor {
	public:
		virtual void visit(class PoolManager *pm)=0;

		virtual void visit(class Pool *pool)=0;

		virtual void visit(class Volume *volume)=0;
	};
}

#endif //MTFS_VISITOR_H
