#ifndef _IMPACT_ACTION_H_
#define _IMPACT_ACTION_H_

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

// An action interacts with a single scheduler.
class Scheduler;

/**
 * Intent of the data.
 */
enum ActionDataIntent { IN = 1, OUT = 2 };

/**
 * Action data identified by attribute, index, and intent.
 */
struct ActionData {
  /**
   * ActionData constructor
   * @param attr_ DataItem attribute name
   * @param idx_ an index to differentiate same attr
   * @param inout_ The intent of the data (IN or OUT)
   */
  ActionData(std::string attr_, int idx_, ActionDataIntent inout_)
      : attr(std::move(attr_)), idx(idx_), inout(inout_) {}

  std::string attr; ///< DataItem attribute for COM access.
  int idx; ///< An extra index to differentiate multiple DataItem usage.
  ActionDataIntent inout; ///< DataItem intent.
};

typedef std::vector<ActionData> ActionDataList;

/**
 * Represents an action taken during a simulation process.
 *
 * An Action is constructed from a list of data it will process. The data is
 * organized with the ActionData struct:
 * - the name (a DataItem window-attribute pair),
 * - an index (to distinguish multiple uses of a COM DataItem),
 * - and the intent (input or output).
 *
 * Action provides three core procedures:
 * - init()
 * - run()
 * - finalize()
 *
 * An Action interacts with the Scheduler through the declare() and name()
 * methods. The declare() method registers the read and write operations
 * performed on the attributes passed into the constructor. The name() method
 * provides a descriptive name for the Action for the print methods and
 * debugging.
 */
class Action {
public:
  /**
   * Construct an empty Action
   * @param name action name (Default: "")
   */
  explicit Action(const std::string &name = "");
  /**
   * Construct an action from an ActionDataList
   * @param name action name (Default: "")
   */
  explicit Action(ActionDataList, std::string name = "");

  Action(const Action &) = delete;
  Action &operator=(const Action &) = delete;
  Action(Action &&) = delete;
  Action &operator=(Action &&) = delete;

  virtual ~Action() = default;

public:
  /**
   * Initialize the Action.
   * @param t time
   */
  virtual void init(double t) = 0;
  /**
   * Run the Action.
   * @param t current time
   * @param dt time step
   * @param alpha interpolation sub step (between 0.0 and 1.0)
   */
  virtual void run(double t, double dt, double alpha) = 0;
  /**
   * Finalize the Action.
   */
  virtual void finalize() = 0;

public:
  /**
   * Print Action information to C-style stream.
   * @param f C stream
   */
  virtual void print(FILE *f) const;
  /**
   * Print topological sort information to C-style stream.
   * @param f C stream
   */
  virtual void print_toposort(FILE *f) const;

  /**
   * Used for sub-scheduling.
   */
  virtual void schedule() {}

public:
  /**
   * Scheduler interaction. Declares Action data to the Scheduler.
   */
  void declare(Scheduler &) const;

  /**
   * Get the Action name.
   * @return Action name
   */
  const std::string &name() const { return action_name; }
  /**
   * Set the Action name.
   * @param name Action name
   */
  void set_name(const std::string &name) { action_name = name; }

protected:
  /**
   * Get DataItem handle if OUT, or const handle if IN, from COM.
   * @param i index in the ActionData list
   * @param optional if true, do not check if DataItem exists
   * @return COM handle or const handle
   */
  int get_dataitem_handle(int i, bool optional = false) const;

protected:
  std::string action_name; ///< Action name
  ActionDataList action_data; ///< List of ActionData
};

#endif // _IMPACT_ACTION_H_
