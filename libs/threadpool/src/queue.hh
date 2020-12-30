/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/threadpool.hh>

namespace Scroom
{
  namespace Detail
  {
    namespace ThreadPool
    {
      /**
       * ThreadPool::Queue implementation class.
       *
       * Keeps track of whether the actual ThreadPool::Queue still exists,
       * and whether jobs are currently being executed. When the last reference
       * to the ThreadPool::Queue is being deleted, it calls deletingQueue(),
       * which blocks until all jobs finish executing.
       */
      class QueueImpl
      {
      public:
        typedef boost::shared_ptr<QueueImpl> Ptr;

        static Ptr create();

        /**
         * Indicate that a job is being started
         *
         * @retval true if the ThreadPool::Queue still exists
         * @retval false if the ThreadPool::Queue has been deleted
         */
        bool jobStarted();

        /** Indicate that a job is finished */
        void jobFinished();

        /**
         * Indicate that the ThreadPool::Queue is being deleted
         *
         * This sets @c isDeleted to @c true and blocks until all jobs have finished executing.
         */
        void deletingQueue();

        /** Return the number of jobs currently running. Used for testing */
        int getCount();

      private:
        boost::mutex              mut;       /**< Guard internal data */
        boost::condition_variable cond;      /**< Gets signaled when a job completes */
        unsigned int              count;     /**< Number of jobs currently running */
        bool                      isDeleted; /**< @c true if the last reference to ThreadPool::Queue goes away */

      private:
        QueueImpl();
      };

      /**
       * Call Queue::jobStarted() and Queue::jobFinished().
       */
      class QueueLock
      {
        QueueImpl::Ptr q;       /**< Reference to our QueueImpl */
        bool           isValid; /**< @c true if there are still references to the ThreadPool::Queue associated with @c q */

      public:
        QueueLock(QueueImpl::Ptr queue);
        QueueLock(const QueueLock&) = delete;
        QueueLock(QueueLock&&)      = delete;
        QueueLock& operator=(const QueueLock&) = delete;
        QueueLock& operator=(QueueLock&&) = delete;
        ~QueueLock();

        /** Return @c true if there are still references to the ThreadPool::Queue associated with @c q */
        bool queueExists();
      };

    } // namespace ThreadPool
  }   // namespace Detail
} // namespace Scroom
