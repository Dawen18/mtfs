/// \file Migrator.cpp
/// \brief
/// \author David Wittwer
/// \version 0.0.1
/// \date 20.06.17

#include "mtfs/Migrator.h"

void mtfs::Migrator::visit(mtfs::PoolManager *pm) {
	for (auto &&pool :pm->pools) {
		pool.second->accept(this);
	}
}

void mtfs::Migrator::visit(mtfs::Pool *pool) {
	for (auto &&volume :pool->volumes) {

	}
}

void mtfs::Migrator::visit(mtfs::Volume *volume) {
}

