/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef HELPERS_HH
#define HELPERS_HH

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

using namespace Scroom;

void pass_destroy_and_clear(Semaphore* s0, Semaphore* s1, Semaphore* s2, ThreadPool::Queue::WeakPtr q);
void clear_sem(Semaphore* s);
void clear_and_pass(Semaphore* toClear, Semaphore* toPass);


#endif
