/// \file Migrator.h
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 20.06.17

#ifndef MTFS_MIGRATOR_H
#define MTFS_MIGRATOR_H

#include "Visitor.h"
#include "TimeRule.h"
#include "UserRightRule.h"

namespace mtfs {
	class Migrator: public Visitor {

	public:
		void visit(class PoolManager *pm) override;

		void visit(class Pool *pool) override;

		void visit(class Volume *volume) override;

	};
}


#endif //MTFS_MIGRATOR_H
