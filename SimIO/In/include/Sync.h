//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//        
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information. 
//

#if !defined(_SYNC_H)
# define _SYNC_H

# include <pthread.h>

class Mutex
{
  public:
    Mutex();
    ~Mutex();

    inline bool IsOk() const { return m_isOk; }

    int Lock();
    int TryLock();
    int Unlock();

    friend class Condition;

  private:
    pthread_mutex_t m_mutex;
    bool m_isOk;
};

class Condition
{
  public:
    Condition(Mutex& mutex);
    ~Condition();

    inline bool IsOk() const { return (m_isOk && m_mutex.IsOk()); }

    int Wait();
    int Signal();
    int Broadcast();

  private:
    Mutex& m_mutex;
    pthread_cond_t m_cond;
    bool m_isOk;
};

class Semaphore
{
  public:
    Semaphore(int initialcount = 0, int maxcount = 0);
    ~Semaphore();

    inline bool IsOk() const { return m_isOk; }

    bool Wait();
    bool TryWait();
    bool Post();

  private:
    Mutex m_mutex;
    Condition m_cond;
    int m_count;
    int m_maxcount;
    bool m_isOk;
};

#endif // !defined(_SYNC_H)



