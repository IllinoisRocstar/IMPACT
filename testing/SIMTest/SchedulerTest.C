//
//  Copyright@2013, Illinois Rocstar LLC. All rights reserved.
//
//  See LICENSE file included with this source or
//  (opensource.org/licenses/NCSA) for license information.
//

#include "Action.h"
#include "DDGScheduler.h"
#include "UserScheduler.h"
#include "SchedulerAction.h"
#include "gtest/gtest.h"

class TestAction : public Action {
public:
  using Action::Action;

  void init(double t) override {}
  void run(double t, double dt, double alpha) override {}
  void finalize() override {}
};

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(DDGSchedulerTests, DisjointActions) {
  auto *sched = new DDGScheduler(true);

  auto *f = new TestAction({{"f", 1, OUT}}, "F");
  sched->add_action(f);

  auto *g = new TestAction({{"f", 1, IN}}, "G");
  sched->add_action(g);

  auto *h = new TestAction({{"i", 1, IN}}, "H");
  sched->add_action(h);

  auto *i = new TestAction({{"i", 1, OUT}}, "I");
  sched->add_action(i);

  // all creation done now
  ASSERT_NO_THROW(sched->schedule());

  // generate GDL file for visualization with iaSee tool
  sched->print("out.gdl");

  ASSERT_NO_THROW(sched->init_actions(1));

  ASSERT_NO_THROW(sched->run_actions(1, 0.1, -1.0));

  ASSERT_NO_THROW(sched->finalize_actions());

  delete sched;
  delete f;
  delete g;
  delete h;
  delete i;
}

TEST(DDGSchedulerTests, InOrder) {
  auto *sched = new DDGScheduler(true);

  // root action
  auto *a = new TestAction({{"b", 1, OUT}, {"c", 1, OUT}}, "A");
  auto *c = new TestAction({{"c", 1, IN}, {"c", 2, OUT}, {"e", 1, OUT}}, "C");
  auto *b = new TestAction({{"b", 1, IN}, {"d", 1, OUT}}, "B");
  auto *d = new TestAction({{"d", 1, IN}, {"c", 2, IN}}, "D");
  sched->add_action(a);
  sched->add_action(c);
  sched->add_action(b);
  sched->add_action(d);

  // action E is a container action that has its own sub-scheduler
  // the sub scheduler contains action F and G
  auto *subsched = new DDGScheduler(true);

  auto *f = new TestAction({{"f", 1, OUT}}, "F");
  auto *g = new TestAction({{"f", 1, IN}}, "G");
  auto *e = new SchedulerAction(subsched, {{"e", 1, IN}}, "E");

  subsched->add_action(f);
  subsched->add_action(g);
  sched->add_action(e);

  // all creation done now
  ASSERT_NO_THROW(sched->schedule());

  // generate GDL file for visualization with iaSee tool
  //sched->print("out.gdl");

  ASSERT_NO_THROW(sched->init_actions(1));

  ASSERT_NO_THROW(sched->run_actions(1, 0.1, -1.0));

  ASSERT_NO_THROW(sched->finalize_actions());

  delete sched;
  delete a;
  delete b;
  delete c;
  delete d;
  delete e;
  delete f;
  delete g;
}

TEST(UserSchedulerTests, InOrder) {
  auto *sched = new UserScheduler(true);

  // root action
  auto *a = new TestAction({{"b", 1, OUT}, {"c", 1, OUT}}, "A");
  auto *c = new TestAction({{"c", 1, IN}, {"c", 2, OUT}, {"e", 1, OUT}}, "C");
  auto *b = new TestAction({{"b", 1, IN}, {"d", 1, OUT}}, "B");
  auto *d = new TestAction({{"d", 1, IN}, {"c", 2, IN}}, "D");
  sched->add_action(a);
  sched->add_action(c);
  sched->add_action(b);
  sched->add_action(d);

  // action E is a container action that has its own sub-scheduler
  // the sub scheduler contains action F and G
  auto *subsched = new UserScheduler(true);

  auto *f = new TestAction({{"f", 1, OUT}}, "F");
  auto *g = new TestAction({{"f", 1, IN}}, "G");
  auto *e = new SchedulerAction(subsched, {{"e", 1, IN}}, "E");

  subsched->add_action(f);
  subsched->add_action(g);
  sched->add_action(e);

  // all creation done now
  ASSERT_NO_THROW(sched->schedule());

  // generate GDL file for visualization with iaSee tool
  //sched->print("out.gdl");

  ASSERT_NO_THROW(sched->init_actions(1));

  ASSERT_NO_THROW(sched->run_actions(1, 0.1, -1.0));

  ASSERT_NO_THROW(sched->finalize_actions());

  delete sched;
  delete a;
  delete b;
  delete c;
  delete d;
  delete e;
  delete f;
  delete g;
}

TEST(DDGSchedulerTestsDeathTest, CycleInGraph) {
  auto *sched = new DDGScheduler(true);

  auto *ab = new TestAction({{"cc", 1, IN}, {"b", 1, IN}, {"a", 1, OUT}}, "AB");
  auto *ba = new TestAction({{"a", 1, IN}, {"b", 1, OUT}}, "BA");
  auto *cc = new TestAction({{"cc", 1, OUT}}, "CC");

  sched->add_action(ab);
  sched->add_action(ba);
  sched->add_action(cc);

  EXPECT_DEATH(sched->schedule(), "Error: Cycle detected in the DDG graph "
                                  "during the scheduling of actions!");

  delete sched;
  delete ab;
  delete ba;
  delete cc;
}

TEST(DDGSchedulerTestsDeathTest, TwoInputSources) {
  auto *sched = new DDGScheduler(true);

  auto *a1 = new TestAction({{"a", 1, OUT}}, "A1");
  auto *a2 = new TestAction({{"a", 1, OUT}}, "A2");
  auto *ar = new TestAction({{"a", 1, IN}}, "Ar");

  sched->add_action(a1);
  sched->add_action(a2);
  sched->add_action(ar);

  EXPECT_DEATH(sched->schedule(),
               "Error: Duplicate input. Action Ar can take input \\(attr: a "
               "index: 1\\) from either Action A2 or Action A1");

  delete sched;
  delete a1;
  delete a2;
  delete ar;
}

TEST(DDGSchedulerTestsDeathTest, NoMatchingRead) {
  auto *sched = new DDGScheduler(true);

  auto *a1 = new TestAction({{"a", 1, OUT}}, "A1");

  sched->add_action(a1);

  EXPECT_DEATH(
      sched->schedule(),
      "Error: Cannot find matching input to Action A1 of attr: a index: 1");

  delete sched;
  delete a1;
}

TEST(DDGSchedulerTestsDeathTest, NoMatchingWrite) {
  auto *sched = new DDGScheduler(true);

  auto *ar = new TestAction({{"a", 1, IN}}, "Ar");

  sched->add_action(ar);

  EXPECT_DEATH(
      sched->schedule(),
      "Error: Cannot find matching output for Action Ar of attr: a index: 1");

  delete sched;
  delete ar;
}
