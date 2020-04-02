#ifndef _IMPACT_SCHEDULER_H_
#define _IMPACT_SCHEDULER_H_

#include <cstdio>

#include "Action.h"

class Scheduler;
typedef void (Scheduler::*Scheduler_voidfn1_t)(double);

/**
 * A Scheduler manages actions through the lifetime of a simulation process.
 *
 * A Scheduler is a container of actions and is responsible for determining the
 * orders of initialization, execution, and finalization of its actions. An
 * Action is added to the Scheduler through the add_action() method, invoking
 * the Action::declare() method, which will callback the reads() and writes()
 * methods.
 *
 * Upon calling schedule(), the Scheduler will determine the order of
 * initialization, execution, and finalization of all added Action.
 *
 * The functions init_actions(), run_actions(), and finalize_actions()
 * correspond to the Action equivalents.
 */
class Scheduler {
public:
  /**
   * Construct a Scheduler named "Scheduler"
   * @param verbose print debugging information (Default: false)
   */
  explicit Scheduler(bool verbose = false);

  Scheduler(const Scheduler &) = delete;
  Scheduler &operator=(const Scheduler &) = delete;
  Scheduler(Scheduler &&) = delete;
  Scheduler &operator=(Scheduler &&) = delete;

  virtual ~Scheduler();

public:
  /**
   * Determines the order Actions will be run. Must be called after
   * add_actions() and before init_actions(), run_actions(), or
   * finalize_actions() can be used.
   */
  virtual void schedule() = 0;

public:
  /**
   * Add Action to Scheduler.
   */
  void add_action(Action *);

  /**
   * Action callback routine to declare inputs
   * @param attr DataItem attribute
   * @param idx an index to differentiate same attr
   */
  void reads(const Action *, const std::string &attr, int idx);
  /**
   * Action callback routine to declare outputs
   * @param attr DataItem attribute
   * @param idx an index to differentiate same attr
   */
  void writes(const Action *, const std::string &attr, int idx);

  /**
   * Initialize actions at time t
   * @param t time
   */
  void init_actions(double t);
  /**
   * Run actions at time t with time step dt.
   * @param t current time
   * @param dt time step
   * @param alpha interpolation sub step
   */
  void run_actions(double t, double dt, double alpha);
  /**
   * Finalize actions
   */
  void finalize_actions();

  /**
   * Print to file in GDL.
   * @param fname file name
   */
  void print(const char *fname) const;
  /**
   * Print Scheduler to C-style stream.
   * @param f C-style stream
   * @param container_name Name of the container
   * @return name of the scheduler in the container
   */
  char *print(FILE *f, const char *container_name) const;

  /**
   * Get the name of the scheduler. Default "Scheduler".
   * @return name
   */
  const std::string &name() const { return scheduler_name; }
  /**
   * Set the name of the scheduler.
   * @param name new name
   */
  void set_name(const std::string &name) { scheduler_name = name; }

  /**
   * Clear the inited state. Used for restarting.
   * @param t unused
   */
  void restarting(double t) { inited = false; }

  /**
   * Checks if the Scheduler contain actions.
   * @return
   */
  bool isEmpty() const { return actions.empty(); }

  /**
   * Print topological sort information to C-style stream.
   * @param f C stream
   */
  void print_toposort(FILE *f) const;

protected:
  struct ActionItem;
  typedef std::vector<ActionItem *> ActionList;

  /**
   * ActionItem stores an Action and its input/output links to other Action.
   */
  struct ActionItem {
    explicit ActionItem(Action *a) : myaction(a), print_flag(0) {}

    /**
     * Access contained action
     * @return pointer to action
     */
    inline const Action *action() const { return myaction; }
    /**
     * Access contained action
     * @return pointer to action
     */
    inline Action *action() { return myaction; }

    /**
     * Get number of input actions.
     * @return number of input actions
     */
    inline int n_write() const { return write_attr.size(); }
    /**
     * Get number of output actions.
     * @return number of output actions
     */
    inline int n_read() const { return read_attr.size(); }
    /**
     * Get name of action.
     * @return name of action.
     */
    inline const std::string &name() const { return myaction->name(); }
    /**
     * Are all input actions identified?
     * @return True if all input actions are identified.
     */
    bool fulfilledInput() const;
    /**
     * Are all output actions identified?
     * @return True if all output actions are identified.
     */
    bool fulfilledOutput() const;
    /**
     * Are all input and output actions identified?
     * @return True if all input and output actions are identified.
     */
    bool fulfilled() const;
    /**
     * Returns position id in read_attr, read_idx, and input vectors.
     * Returns -1 if not found.
     *
     * @param attr Action attribute
     * @param idx Action index
     * @return position in the read_attr, read_idx, and input vectors.
     */
    int hasInput(const std::string &attr, int idx) const;
    /**
     * Returns position id in write_attr, write_idx, and output vectors.
     * Returns -1 if not found.
     *
     * @param attr Action attribute
     * @param idx Action index
     * @return position in the write_attr, write_idx, and output vectors.
     */
    int hasOutput(const std::string &attr, int idx) const;
    /**
     * Print state to C I/O stream.
     * @param f pointer to C I/O stream (e.g., stdout)
     */
    void print(FILE *f) const;

    Action *myaction;

    std::vector<std::string> read_attr;
    std::vector<int> read_idx;
    ActionList input; // read_n

    std::vector<std::string> write_attr;
    std::vector<int> write_idx;
    ActionList output; // write_n

    mutable int print_flag; // for print
  };

protected:
  /**
   * For debugging, prints Actions to C-style stream
   * @param f C stream
   */
  void printActions(FILE *f) const;

protected:
  std::string scheduler_name; ///< name of the Scheduler

  ActionList actions; ///< all actions registered
  ActionList roots;   ///< forest
  ActionList sort;    ///< topological sort order

  bool scheduled; ///< flag: if has been scheduled
  bool inited;    ///< flag: true if init called

  bool verbose; ///< flag: verbosity (Default: false)

private:
  /**
   * Helper function to print ActionItem to C-style stream
   * @param f C stream
   * @param aitem ActionItem
   */
  static void print_helper(FILE *f, const ActionItem *aitem);
};

#endif // _IMPACT_SCHEDULER_H_
