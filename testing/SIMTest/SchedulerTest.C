//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include "Action.h"
#include "Scheduler.h"
#include "SchedulerAction.h"
#include "gtest/gtest.h"

class ActionA : public Action {
 public:
  ActionA(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.writes(this, "b", 1);
    sched.writes(this, "c", 1);
  }
};

class ActionB : public Action {
 public:
  ActionB(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.reads(this, "b", 1);
    sched.writes(this, "d", 1);
  }
};

class ActionC : public Action {
 public:
  ActionC(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.reads(this, "c", 1);
    sched.writes(this, "c", 1);
    sched.writes(this, "e", 1);
  }
};

class ActionD : public Action {
 public:
  ActionD(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.reads(this, "d", 1);
    sched.reads(this, "c", 1);
  }
};

class ActionE : public SchedulerAction {
 public:
  ActionE(Scheduler *s, int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : SchedulerAction(s, n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) { sched.reads(this, "e", 1); }
};

class ActionF : public Action {
 public:
  ActionF(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) { sched.writes(this, "f", 1); }
};

class ActionG : public Action {
 public:
  ActionG(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) { sched.reads(this, "f", 1); }
};

class ActionH : public Action {
 public:
  ActionH(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.reads(this, "i", 1);
  }
};

class ActionI : public Action {
 public:
  ActionI(int n, const char *at[], int i[], void *p = 0,
          const char *name = NULL)
      : Action(n, at, i, p, name) {}
  virtual void declare(Scheduler &sched) {
    sched.writes(this, "i", 1);
  }
};

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(SchedulerTests, DisjointActions){
  Scheduler * sched = new Scheduler();

  std::string F("F");
  ActionF *f = new ActionF(0, NULL, NULL, NULL, F.c_str());
  sched->add_action(f);
  //f->declare(*subsched);

  const char *attr_g[] = { (char*)"f" };
  int idx_g[] = { 1 };
  std::string G("G");
  ActionG *g = new ActionG(1, attr_g, idx_g, NULL, G.c_str() );
  sched->add_action(g);
  //g->declare(*subsched);

  std::string I("I");
  ActionI *i = new ActionI(0, NULL, NULL, NULL, I.c_str() );
  sched->add_action(i);

  const char* attr_h[] = { (char*) "i"};
  int idx_h[] = { 1 };
  std::string H("H");
  ActionH *h = new ActionH(1, attr_h, idx_h, NULL, H.c_str() );
  sched->add_action(h);

  // all creation done now
  ASSERT_NO_THROW( sched->schedule() );

  // generate GDL file for visualization with iaSee tool
  sched->print("out.gdl");

  ASSERT_NO_THROW( sched->init_actions(1) );

  ASSERT_NO_THROW( sched->run_actions(1, 0.1 ) );

  ASSERT_NO_THROW( sched->finalize_actions() );
}


TEST(SchedulerTests, InOrder) {
  Scheduler *sched = new Scheduler();

  // root action
  std::string A("A");
  ActionA *a = new ActionA(0, NULL, NULL, NULL, A.c_str());
  sched->add_action(a);
  // a->declare(*sched);

  const char *attr_c[] = {(char *)"c"};
  int idx_c[] = {1};
  std::string C("C");
  ActionC *c = new ActionC(1, attr_c, idx_c, NULL, C.c_str());
  sched->add_action(c);
  // c->declare(*sched);

  const char *attr_b[] = {(char *)"b"};
  int idx_b[] = {1};
  std::string B("B");
  ActionB *b = new ActionB(1, attr_b, idx_b, NULL, B.c_str());
  sched->add_action(b);
  // b->declare(*sched);

  const char *attr_d[] = {(char *)"d", (char *)"c"};
  int idx_d[] = {1, 1};
  std::string D("D");
  ActionD *d = new ActionD(2, attr_d, idx_d, NULL, D.c_str());
  sched->add_action(d);
  // d->declare(*sched);

  // action E is a container action that has its own sub-schduler
  // the sub scheduler contains action F and G
  Scheduler *subsched = new Scheduler();

  std::string F("F");
  ActionF *f = new ActionF(0, NULL, NULL, NULL, F.c_str());
  subsched->add_action(f);
  // f->declare(*subsched);

  const char *attr_g[] = {(char *)"f"};
  int idx_g[] = {1};
  std::string G("G");
  ActionG *g = new ActionG(1, attr_g, idx_g, NULL, G.c_str());
  subsched->add_action(g);
  // g->declare(*subsched);

  const char *attr_e[] = {(char *)"f"};
  int idx_e[] = {1};
  std::string E("E");
  ActionE *e = new ActionE(subsched, 1, attr_e, idx_e, NULL, E.c_str());
  sched->add_action(e);
  // e->declare(*sched);

  // all creation done now
  ASSERT_NO_THROW(sched->schedule());

  // generate GDL file for visualization with iaSee tool
  sched->print("out.gdl");

  ASSERT_NO_THROW(sched->init_actions(1));

  ASSERT_NO_THROW(sched->run_actions(1, 0.1));

  ASSERT_NO_THROW(sched->finalize_actions());
}
